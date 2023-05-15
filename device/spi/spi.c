/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/spi/spi.h"

#undef LOG_TAG
#define LOG_TAG "spi"

#if (MR_CONF_DEVICE_SPI == MR_CONF_ENABLE)

static mr_err_t mr_take_spi_bus(mr_spi_device_t spi_device)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(spi_device->bus != MR_NULL);

	/* Take the mutex lock of the spi-bus */
	ret = mr_mutex_take(&spi_device->bus->lock, &spi_device->device.object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Check if the current spi-device is the owner of the spi-bus */
	if (spi_device->bus->owner != spi_device)
	{
		/* Stop the chip-select of the last spi-bus owner */
		if (spi_device->bus->owner != MR_NULL)
			spi_device->bus->ops->cs_set(spi_device->bus, spi_device->bus->owner->cs_pin, MR_DISABLE);

		/* If the configuration is different, the spi-bus is reconfigured */
		if (spi_device->config.baud_rate != spi_device->bus->config.baud_rate
			|| spi_device->config.host_slave != spi_device->bus->config.host_slave
			|| spi_device->config.mode != spi_device->bus->config.mode
			|| spi_device->config.bit_order != spi_device->bus->config.bit_order)
		{
			ret = spi_device->bus->ops->configure(spi_device->bus, &spi_device->config);
			if (ret != MR_ERR_OK)
				return ret;
		}

		/* Sets the spi-bus owner to the current spi-device */
		spi_device->bus->config = spi_device->config;
		spi_device->bus->owner = spi_device;

		/* Start the chip-select of the spi-device */
		spi_device->bus->ops->cs_set(spi_device->bus, spi_device->bus->owner->cs_pin, MR_ENABLE);
	}

	return MR_ERR_OK;
}

static mr_err_t mr_release_spi_bus(mr_spi_device_t spi_device)
{
	MR_ASSERT(spi_device->bus != MR_NULL);

	return mr_mutex_release(&spi_device->bus->lock, &spi_device->device.object);
}

static mr_err_t mr_spi_bus_open(mr_device_t device)
{
	mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;
	struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

	/* Setting spi-bus to default-config */
	if (spi_bus->config.baud_rate == 0)
		spi_bus->config = default_config;

	return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_bus_close(mr_device_t device)
{
	mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;

	/* Setting spi-bus to close-config */
	spi_bus->config.baud_rate = 0;

	return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_device_open(mr_device_t device)
{
	mr_spi_device_t spi_device = (mr_spi_device_t)device;
	struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

	/* Setting spi-device to default-config */
	if (spi_device->config.baud_rate == 0)
		spi_device->config = default_config;

	return MR_ERR_OK;
}

static mr_err_t mr_spi_device_close(mr_device_t device)
{
	mr_spi_device_t spi_device = (mr_spi_device_t)device;

	/* Setting spi-device to close-config */
	spi_device->config.baud_rate = 0;

	/* Disconnect from the spi-bus */
	spi_device->bus = MR_NULL;

	return MR_ERR_OK;
}

static mr_err_t mr_spi_device_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_spi_device_t spi_device = (mr_spi_device_t)device;

	switch (cmd & _MR_CTRL_FLAG_MASK)
	{
		case MR_CTRL_CONFIG:
		{
			if (args)
			{
				spi_device->config = *(struct mr_spi_config *)args;
				return MR_ERR_OK;
			}
			return - MR_ERR_INVALID;
		}

		case MR_CTRL_ATTACH:
		{
			/* Find the spi-bus */
			mr_device_t spi_bus = mr_device_find((char *)args);
			if (spi_bus == MR_NULL)
				return - MR_ERR_NOT_FOUND;
			if (spi_bus->type != MR_DEVICE_TYPE_SPI_BUS)
				return - MR_ERR_INVALID;

			/* Open the spi-bus */
			mr_device_open(spi_bus, MR_OPEN_RDWR);
			spi_device->bus = (mr_spi_bus_t)spi_bus;
			return MR_ERR_OK;
		}

		default: return - MR_ERR_UNSUPPORTED;
	}
}

static mr_ssize_t mr_spi_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_spi_device_t spi_device = (mr_spi_device_t)device;
	mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
	mr_err_t ret = MR_ERR_OK;
	mr_size_t recv_size = 0;

	/* Take spi-bus */
	ret = mr_take_spi_bus(spi_device);
	if (ret != MR_ERR_OK)
	{
		MR_LOG_E(LOG_TAG, "Device %s: Failed to take spi-bus\r\n", device->object.name);
		return ret;
	}

	for (recv_size = 0; recv_size < size; recv_size ++)
	{
		spi_device->bus->ops->write(spi_device->bus, 0u);
		*recv_buffer = spi_device->bus->ops->read(spi_device->bus);
		recv_buffer ++;
	}

	/* Release spi-bus */
	mr_release_spi_bus(spi_device);

	return (mr_ssize_t)size;
}

static mr_ssize_t mr_spi_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_spi_device_t spi_device = (mr_spi_device_t)device;
	mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
	mr_err_t ret = MR_ERR_OK;
	mr_size_t send_size = 0;

	/* Take spi-bus */
	ret = mr_take_spi_bus(spi_device);
	if (ret != MR_ERR_OK)
	{
		MR_LOG_E(LOG_TAG, "Device %s: Failed to take spi-bus\r\n", device->object.name);
		return ret;
	}

	for (send_size = 0; send_size < size; send_size ++)
	{
		spi_device->bus->ops->write(spi_device->bus, *send_buffer);
		send_buffer ++;
	}

	/* Release spi-bus */
	mr_release_spi_bus(spi_device);

	return (mr_ssize_t)size;
}

static mr_err_t _err_io_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static void _err_io_spi_write(mr_spi_bus_t spi_bus, mr_uint8_t data)
{
	MR_ASSERT(0);
}

static mr_uint8_t _err_io_spi_read(mr_spi_bus_t spi_bus)
{
	MR_ASSERT(0);
	return 0;
}

static void _err_io_spi_cs_set(mr_spi_bus_t spi_bus, mr_uint16_t cs_pin, mr_state_t state)
{
	MR_ASSERT(0);
}

mr_err_t mr_hw_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, struct mr_spi_bus_ops *ops, void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			mr_spi_bus_open,
			mr_spi_bus_close,
			MR_NULL,
			MR_NULL,
			MR_NULL,
		};

	MR_ASSERT(spi_bus != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the spi-bus to the container */
	ret = mr_device_add(&spi_bus->device, name, MR_DEVICE_TYPE_SPI_BUS, MR_OPEN_RDWR, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the spi-bus fields */
	spi_bus->config.baud_rate = 0;
	spi_bus->owner = MR_NULL;
	mr_mutex_init(&spi_bus->lock);

	/* Set spi-bus operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_spi_configure;
	ops->write = ops->write ? ops->write : _err_io_spi_write;
	ops->read = ops->read ? ops->read : _err_io_spi_read;
	ops->cs_set = ops->cs_set ? ops->cs_set : _err_io_spi_cs_set;
	spi_bus->ops = ops;

	return MR_ERR_OK;
}

mr_err_t mr_hw_spi_device_add(mr_spi_device_t spi_device,
							  const char *name,
							  mr_uint16_t support_flag,
							  mr_uint16_t cs_pin)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			mr_spi_device_open,
			mr_spi_device_close,
			mr_spi_device_ioctl,
			mr_spi_device_read,
			mr_spi_device_write,
		};

	MR_ASSERT(spi_device != MR_NULL);
	MR_ASSERT(support_flag != MR_NULL);

	/* Add the spi-device to the container */
	ret = mr_device_add(&spi_device->device, name, MR_DEVICE_TYPE_SPI, support_flag, &device_ops, MR_NULL);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the spi-device fields */
	spi_device->config.baud_rate = 0;
	spi_device->bus = MR_NULL;
	spi_device->cs_pin = cs_pin;

	return MR_ERR_OK;
}

#endif