/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-21     MacRsh       first version
 */

#include <device/i2c/i2c.h>

#if (MR_DEVICE_I2C == MR_CONF_ENABLE)

static mr_err_t mr_take_i2c_bus(mr_i2c_device_t i2c_device)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(i2c_device->bus != MR_NULL);

	/* Take the mutex lock of the i2c-bus */
	ret = mr_mutex_take(&i2c_device->bus->lock, &i2c_device->device.object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Check if the current i2c-device is the owner of the i2c-bus */
	if (i2c_device->bus->owner != i2c_device)
	{
		/* If the configuration is different, the i2c-bus is reconfigured */
		if (i2c_device->config.baud_rate != i2c_device->bus->config.baud_rate
			|| i2c_device->config.host_slave != i2c_device->bus->config.host_slave
			|| i2c_device->config.address_mode != i2c_device->bus->config.address_mode)
		{
			ret = i2c_device->bus->ops->configure(i2c_device->bus, &i2c_device->config);
			if (ret != MR_ERR_OK)
				return ret;
		}

		/* Sets the i2c-bus owner to the current i2c-device */
		i2c_device->bus->config = i2c_device->config;
		i2c_device->bus->owner = i2c_device;
	}

	return MR_ERR_OK;
}

static mr_err_t mr_release_i2c_bus(mr_i2c_device_t i2c_device)
{
	MR_ASSERT(i2c_device->bus != MR_NULL);

	return mr_mutex_release(&i2c_device->bus->lock, &i2c_device->device.object);
}

static mr_err_t mr_i2c_bus_open(mr_device_t device)
{
	mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;
	struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

	/* Setting i2c-bus to default-config */
	if (i2c_bus->config.baud_rate == 0)
		i2c_bus->config = default_config;

	return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

static mr_err_t mr_i2c_bus_close(mr_device_t device)
{
	mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;

	/* Setting i2c-bus to close-config */
	i2c_bus->config.baud_rate = 0;

	return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

static mr_err_t mr_i2c_device_open(mr_device_t device)
{
	mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
	struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

	/* Setting i2c-device to default-config */
	if (i2c_device->config.baud_rate == 0)
		i2c_device->config = default_config;

	return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_close(mr_device_t device)
{
	mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

	/* Setting i2c-device to close-config */
	i2c_device->config.baud_rate = 0;

	/* Disconnect from the i2c-bus */
	i2c_device->bus = MR_NULL;

	return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
	mr_err_t ret = MR_ERR_OK;

	switch (cmd & _MR_CMD_MASK)
	{
		case MR_CMD_CONFIG:
		{
			if (args)
				i2c_device->config = *(struct mr_i2c_config *)args;
			break;
		}

		case MR_CMD_ATTACH:
		{
			if (args)
			{
				/* Find in the kernel i2c-bus */
				mr_device_t i2c_bus = mr_device_find((char *)args);
				if (i2c_bus == MR_NULL || i2c_bus->type != MR_DEVICE_TYPE_I2C_BUS)
				{
					ret = - MR_ERR_NOT_FOUND;
					break;
				}

				/* Open i2c-bus */
				mr_device_open(i2c_bus, MR_OPEN_RDWR);
				i2c_device->bus = (mr_i2c_bus_t)i2c_bus;
			}
			break;
		}

		default: ret = - MR_ERR_UNSUPPORTED;
	}

	return ret;
}

static mr_size_t mr_i2c_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
	mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
	mr_err_t ret = MR_ERR_OK;
	mr_size_t recv_size = size;

	/* Take i2c-bus */
	ret = mr_take_i2c_bus(i2c_device);
	if (ret != MR_ERR_OK)
		return 0;

	/* Send the i2c start signal and read cmd */
	i2c_device->bus->ops->start(i2c_device->bus);
	if (i2c_device->config.address_mode == MR_I2C_ADDRESS_MODE_10)
	{
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(0xF1 | ((i2c_device->address) >> 8) << 1));
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 2));
	} else
	{
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 1 | 0x01));
	}

	while (recv_size --)
	{
		*recv_buffer = i2c_device->bus->ops->read(i2c_device->bus, (mr_state_t)(recv_size == 0));
		recv_buffer ++;
	}

	/* Release i2c-bus */
	mr_release_i2c_bus(i2c_device);

	return size;
}

static mr_size_t mr_i2c_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
	mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;;
	mr_err_t ret = MR_ERR_OK;
	mr_size_t send_size = size;

	/* Take i2c-bus */
	ret = mr_take_i2c_bus(i2c_device);
	if (ret != MR_ERR_OK)
		return 0;

	/* Send the i2c start signal and write cmd */
	i2c_device->bus->ops->start(i2c_device->bus);
	if (i2c_device->config.address_mode == MR_I2C_ADDRESS_MODE_10)
	{
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(0xF0 | ((i2c_device->address) >> 8) << 1));
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 2));
	} else
	{
		i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 1));
	}

	while (send_size --)
	{
		i2c_device->bus->ops->write(i2c_device->bus, *send_buffer);
		send_buffer ++;
	}

	/* Release i2c-bus */
	mr_release_i2c_bus(i2c_device);

	return size;
}

static mr_err_t _err_io_i2c_configure(mr_i2c_bus_t i2c_bus, struct mr_i2c_config *config)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static void _err_io_i2c_start(mr_i2c_bus_t i2c_bus)
{
	MR_ASSERT(0);
}

static void _err_io_i2c_stop(mr_i2c_bus_t i2c_bus)
{
	MR_ASSERT(0);
}

static void _err_io_i2c_write(mr_i2c_bus_t i2c_bus, mr_uint8_t data)
{
	MR_ASSERT(0);
}

static mr_uint8_t _err_io_i2c_read(mr_i2c_bus_t i2c_bus, mr_state_t ack_state)
{
	MR_ASSERT(0);
	return 0;
}

mr_err_t mr_hw_i2c_bus_add_to_container(mr_i2c_bus_t i2c_bus, const char *name, struct mr_i2c_bus_ops *ops, void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			.open = mr_i2c_bus_open,
			.close = mr_i2c_bus_close,
			.ioctl = MR_NULL,
			.read = MR_NULL,
			.write = MR_NULL,
		};

	MR_ASSERT(i2c_bus != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the i2c-bus to the container */
	ret = mr_device_add_to_container(&i2c_bus->device, name, MR_DEVICE_TYPE_I2C_BUS, MR_OPEN_RDWR, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the i2c-bus fields */
	i2c_bus->config.baud_rate = 0;
	i2c_bus->owner = MR_NULL;
	mr_mutex_init(&i2c_bus->lock);

	/* Set i2c-bus operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_i2c_configure;
	ops->start = ops->start ? ops->start : _err_io_i2c_start;
	ops->stop = ops->stop ? ops->stop : _err_io_i2c_stop;
	ops->write = ops->write ? ops->write : _err_io_i2c_write;
	ops->read = ops->read ? ops->read : _err_io_i2c_read;
	i2c_bus->ops = ops;

	return MR_ERR_OK;
}

mr_err_t mr_hw_i2c_device_add_to_container(mr_i2c_device_t i2c_device,
										   const char *name,
										   mr_uint16_t support_flag,
										   mr_uint8_t address)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			.open = mr_i2c_device_open,
			.close = mr_i2c_device_close,
			.ioctl = mr_i2c_device_ioctl,
			.read = mr_i2c_device_read,
			.write = mr_i2c_device_write,
		};

	MR_ASSERT(i2c_device != MR_NULL);
	MR_ASSERT(support_flag != MR_NULL);

	/* Add the i2c-device to the container */
	ret = mr_device_add_to_container(&i2c_device->device, name, MR_DEVICE_TYPE_I2C, support_flag, &device_ops, MR_NULL);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the i2c-device fields */
	i2c_device->address = address;
	i2c_device->config.baud_rate = 0;
	i2c_device->bus = MR_NULL;

	return MR_ERR_OK;
}

#endif