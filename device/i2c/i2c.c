/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/i2c/i2c.h"

#if (MR_CONF_I2C == MR_CONF_ENABLE)

#define _MR_I2C_WRITE   0
#define _MR_I2C_READ    1

static mr_err_t mr_take_i2c_bus(mr_i2c_device_t i2c_device)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(i2c_device->bus != MR_NULL);

    /* Take the mutex lock of the i2c-bus */
    ret = mr_mutex_take(&i2c_device->bus->lock, &i2c_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

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
            {
                return ret;
            }
        }

        /* Sets the i2c-bus owner to the current i2c-device */
        i2c_device->bus->config = i2c_device->config;
        i2c_device->bus->owner = i2c_device;
    }

    /* Send the i2c-bus start signal and read cmd */
    i2c_device->bus->ops->start(i2c_device->bus);

    return MR_ERR_OK;
}

static mr_err_t mr_release_i2c_bus(mr_i2c_device_t i2c_device)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(i2c_device->bus != MR_NULL);

    /* Release the mutex lock of the i2c-bus */
    ret = mr_mutex_release(&i2c_device->bus->lock, &i2c_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Send the i2c-bus stop signal */
    i2c_device->bus->ops->stop(i2c_device->bus);

    return MR_ERR_OK;
}

static void mr_i2c_bus_send_address(mr_i2c_device_t i2c_device, mr_uint8_t read_or_write)
{
    MR_ASSERT(i2c_device->bus != MR_NULL);

    if (read_or_write)
    {
        /* Send the read address signal */
        if (i2c_device->config.address_mode == MR_I2C_ADDRESS_MODE_10)
        {
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(0xf1 | ((i2c_device->address) >> 8) << 1));
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 2));
        } else
        {
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 1 | 0x01));
        }
    } else
    {
        /* Send the write address signal */
        if (i2c_device->config.address_mode == MR_I2C_ADDRESS_MODE_10)
        {
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(0xf0 | ((i2c_device->address) >> 8) << 1));
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 2));
        } else
        {
            i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)(i2c_device->address << 1));
        }
    }
}

static mr_err_t mr_i2c_bus_open(mr_device_t device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    /* Setting i2c-bus to default config, if the baud-rate not set */
    if (i2c_bus->config.baud_rate == 0)
    {
        i2c_bus->config = default_config;
    }

    return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

static mr_err_t mr_i2c_bus_close(mr_device_t device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;

    /* Setting i2c-bus to close config */
    i2c_bus->config.baud_rate = 0;

    return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

static mr_err_t mr_i2c_device_open(mr_device_t device)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    /* Setting i2c-device to default config, if the baud-rate not set */
    if (i2c_device->config.baud_rate == 0)
    {
        i2c_device->config = default_config;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_close(mr_device_t device)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

    /* Setting i2c-device to close config */
    i2c_device->config.baud_rate = 0;

    /* Disconnect from the i2c-bus */
    i2c_device->bus = MR_NULL;

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_CONFIG:
        {
            if (args)
            {
                i2c_device->config = *(struct mr_i2c_config *)args;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_ATTACH:
        {
            /* Detach the i2c-bus */
            if (args == MR_NULL)
            {
                i2c_device->bus = MR_NULL;
                return MR_ERR_OK;
            }

            /* Find the i2c-bus */
            mr_device_t i2c_bus = mr_device_find((char *)args);
            if (i2c_bus == MR_NULL)
            {
                return -MR_ERR_NOT_FOUND;
            }
            if (i2c_bus->type != MR_DEVICE_TYPE_I2C_BUS)
            {
                return -MR_ERR_INVALID;
            }

            /* Open the i2c-bus */
            ret = mr_device_open(i2c_bus, MR_OPEN_RDWR);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
            i2c_device->bus = (mr_i2c_bus_t)i2c_bus;
            return MR_ERR_OK;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_i2c_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
    mr_err_t ret = MR_ERR_OK;
    mr_size_t recv_size = 0;

    /* Take i2c-bus */
    ret = mr_take_i2c_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Send offset */
    if (pos > 0)
    {
        mr_i2c_bus_send_address(i2c_device, _MR_I2C_WRITE);
        i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)pos);
        i2c_device->bus->ops->start(i2c_device->bus);
    }

    /* Send the i2c-bus read command */
    mr_i2c_bus_send_address(i2c_device, _MR_I2C_READ);

    for (recv_size = 0; recv_size < size; recv_size += sizeof(*recv_buffer))
    {
        *recv_buffer = i2c_device->bus->ops->read(i2c_device->bus, (recv_size == 0));
        recv_buffer++;
    }

    /* Release i2c-bus */
    mr_release_i2c_bus(i2c_device);

    return (mr_ssize_t)recv_size;
}

static mr_ssize_t mr_i2c_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
    mr_err_t ret = MR_ERR_OK;
    mr_size_t send_size = 0;

    /* Take i2c-bus */
    ret = mr_take_i2c_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Send the i2c-bus write command */
    mr_i2c_bus_send_address(i2c_device, _MR_I2C_WRITE);

    /* Send offset */
    if (pos > 0)
    {
        i2c_device->bus->ops->write(i2c_device->bus, (mr_uint8_t)pos);
    }

    for (send_size = 0; send_size < size; send_size += sizeof(*send_buffer))
    {
        i2c_device->bus->ops->write(i2c_device->bus, *send_buffer);
        send_buffer++;
    }

    /* Release i2c-bus */
    mr_release_i2c_bus(i2c_device);

    return (mr_ssize_t)send_size;
}

static mr_err_t _err_io_i2c_configure(mr_i2c_bus_t i2c_bus, struct mr_i2c_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
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

static mr_uint8_t _err_io_i2c_read(mr_i2c_bus_t i2c_bus, mr_uint8_t ack_state)
{
    MR_ASSERT(0);
    return 0;
}

mr_err_t mr_i2c_bus_add(mr_i2c_bus_t i2c_bus, const char *name, void *data, struct mr_i2c_bus_ops *ops)
{
    const static struct mr_device_ops device_ops =
            {
                    mr_i2c_bus_open,
                    mr_i2c_bus_close,
                    MR_NULL,
                    MR_NULL,
                    MR_NULL,
            };

    MR_ASSERT(i2c_bus != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    i2c_bus->device.type = MR_DEVICE_TYPE_I2C_BUS;
    i2c_bus->device.data = data;
    i2c_bus->device.ops = &device_ops;

    i2c_bus->config.baud_rate = 0;
    i2c_bus->owner = MR_NULL;
    mr_mutex_init(&i2c_bus->lock);

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_i2c_configure;
    ops->start = ops->start ? ops->start : _err_io_i2c_start;
    ops->stop = ops->stop ? ops->stop : _err_io_i2c_stop;
    ops->write = ops->write ? ops->write : _err_io_i2c_write;
    ops->read = ops->read ? ops->read : _err_io_i2c_read;
    i2c_bus->ops = ops;

    /* Add to the container */
    return mr_device_add(&i2c_bus->device, name, MR_OPEN_RDWR);
}

mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint8_t address)
{
    const static struct mr_device_ops device_ops =
            {
                    .open = mr_i2c_device_open,
                    .close = mr_i2c_device_close,
                    .ioctl = mr_i2c_device_ioctl,
                    .read = mr_i2c_device_read,
                    .write = mr_i2c_device_write,
            };

    MR_ASSERT(i2c_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Initialize the private fields */
    i2c_device->device.type = MR_DEVICE_TYPE_I2C;
    i2c_device->device.data = MR_NULL;
    i2c_device->device.ops = &device_ops;

    i2c_device->config.baud_rate = 0;
    i2c_device->bus = MR_NULL;
    i2c_device->address = address;

    /* Add to the container */
    return mr_device_add(&i2c_device->device, name, MR_OPEN_RDWR);
}

#endif /* MR_CONF_I2C */