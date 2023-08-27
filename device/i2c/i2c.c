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

#if (MR_CFG_I2C == MR_CFG_ENABLE)

#define MR_I2C_RD                       0
#define MR_I2C_WR                       1

static mr_err_t err_io_i2c_configure(mr_i2c_bus_t i2c_bus, mr_i2c_config_t config)
{
    return -MR_ERR_IO;
}

static void err_io_i2c_start(mr_i2c_bus_t i2c_bus)
{

}

static void err_io_i2c_stop(mr_i2c_bus_t i2c_bus)
{

}

static void err_io_i2c_write(mr_i2c_bus_t i2c_bus, mr_uint8_t data)
{

}

static mr_uint8_t err_io_i2c_read(mr_i2c_bus_t i2c_bus, mr_state_t ack)
{
    return 0;
}

static mr_err_t mr_i2c_device_attach_bus(mr_i2c_device_t i2c_device, const char *name)
{
    mr_device_t i2c_bus = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    /* Detach the i2c-bus */
    if (name == MR_NULL)
    {
        if (i2c_device->bus != MR_NULL)
        {
            /* Get the i2c-bus */
            i2c_bus = (mr_device_t)i2c_device->bus;

            /* Close the i2c-bus */
            ret = mr_device_close(i2c_bus);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }

            /* Clear the i2c-bus */
            i2c_device->bus = MR_NULL;
            i2c_device->device.support_flag = MR_OPEN_CLOSED;
        }

        return MR_ERR_OK;
    }

    /* Attach the i2c-bus */
    i2c_bus = mr_device_find(name);
    if (i2c_bus == MR_NULL || i2c_bus->type != Mr_Device_Type_I2CBUS)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Open the i2c-bus */
    ret = mr_device_open(i2c_bus, MR_OPEN_RDWR);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Set the i2c-bus */
    i2c_device->bus = (mr_i2c_bus_t)i2c_bus;
    i2c_device->device.support_flag = i2c_bus->support_flag;

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_take_bus(mr_i2c_device_t i2c_device)
{
    mr_i2c_bus_t i2c_bus = i2c_device->bus;
    mr_err_t ret = MR_ERR_OK;

    /* Take the mutex lock of the i2c-bus */
    ret = mr_mutex_take(&i2c_bus->lock, &i2c_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Check if the i2c-bus is different from the current one */
    if (i2c_bus->owner != i2c_device)
    {
        /* If the configuration is different, the i2c-bus is reconfigured */
        if (i2c_device->config.baud_rate != i2c_bus->config.baud_rate
            || i2c_device->config.host_slave != i2c_bus->config.host_slave)
        {
            ret = i2c_bus->ops->configure(i2c_bus, &i2c_device->config);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
        }

        /* Sets the i2c-bus owner to the current i2c-device */
        i2c_bus->config = i2c_device->config;
        i2c_bus->owner = i2c_device;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_release_bus(mr_i2c_device_t i2c_device)
{
    mr_i2c_bus_t i2c_bus = i2c_device->bus;

    /* Release the mutex lock of the i2c-bus */
    return mr_mutex_release(&i2c_bus->lock, &i2c_device->device.object);
}

static void mr_i2c_device_send_address(mr_i2c_device_t i2c_device, mr_state_t rw)
{
    mr_i2c_bus_t i2c_bus = i2c_device->bus;

    if (rw)
    {
        i2c_bus->ops->write(i2c_bus, (mr_uint8_t)(i2c_device->address << 1));
    } else
    {
        i2c_bus->ops->write(i2c_bus, (mr_uint8_t)(i2c_device->address << 1 | 0x01));
    }
}

static mr_err_t mr_i2c_device_open(mr_device_t device)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    /* Enable i2c using the default config */
    if (i2c_device->config.baud_rate == 0)
    {
        i2c_device->config = default_config;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_close(mr_device_t device)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

    /* Disable i2c */
    i2c_device->config.baud_rate = 0;

    return mr_i2c_device_attach_bus(i2c_device, MR_NULL);
}

static mr_err_t mr_i2c_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

    switch (cmd & MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_i2c_config_t config = (mr_i2c_config_t)args;
                i2c_device->config = *config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_i2c_config_t config = (mr_i2c_config_t)args;
                *config = i2c_device->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_ATTACH:
        {
            return mr_i2c_device_attach_bus(i2c_device, (const char *)args);
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_i2c_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_i2c_bus_t i2c_bus = MR_NULL;
    mr_size_t read_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the i2c-bus */
    ret = mr_i2c_device_take_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Get the i2c-bus */
    i2c_bus = i2c_device->bus;

    if (i2c_device->config.host_slave == MR_I2C_HOST)
    {
        /* Send position */
        if (pos != 0)
        {
            mr_size_t bits = 1 << i2c_device->config.pos_bits;

            i2c_bus->ops->start(i2c_bus);
            mr_i2c_device_send_address(i2c_device, MR_I2C_WR);

            while (bits--)
            {
                i2c_bus->ops->write(i2c_bus, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        /* Start and send the read command */
        i2c_bus->ops->start(i2c_bus);
        mr_i2c_device_send_address(i2c_device, MR_I2C_RD);

        /* Blocking read */
        for (read_size = 0; read_size < size; read_size += sizeof(*read_buffer))
        {
            *read_buffer = i2c_bus->ops->read(i2c_bus, (mr_state_t)(read_size < (size - sizeof(*read_buffer))));
            read_buffer++;
        }

        /* Stop read */
        i2c_bus->ops->stop(i2c_bus);
    }

    /* Release i2c-bus */
    mr_i2c_device_release_bus(i2c_device);

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_i2c_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_i2c_bus_t i2c_bus = MR_NULL;
    mr_size_t write_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the i2c-bus */
    ret = mr_i2c_device_take_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Get the i2c-bus */
    i2c_bus = i2c_device->bus;

    if (i2c_device->config.host_slave == MR_I2C_HOST)
    {
        /* Start and send the write command */
        i2c_bus->ops->start(i2c_bus);
        mr_i2c_device_send_address(i2c_device, MR_I2C_WR);

        /* Send position */
        if (pos != 0)
        {
            mr_size_t bits = 1 << i2c_device->config.pos_bits;

            while (bits--)
            {
                i2c_bus->ops->write(i2c_bus, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        for (write_size = 0; write_size < size; write_size += sizeof(*write_buffer))
        {
            i2c_bus->ops->write(i2c_bus, *write_buffer);
            write_buffer++;
        }

        /* Stop write */
        i2c_bus->ops->stop(i2c_bus);
    }

    /* Release i2c-bus */
    mr_i2c_device_release_bus(i2c_device);

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the i2c device.
 *
 * @param i2c_device The i2c device to be added.
 * @param name The name of the i2c device.
 * @param address The address of the i2c device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint8_t address)
{
    static struct mr_device_ops device_ops =
        {
            mr_i2c_device_open,
            mr_i2c_device_close,
            mr_i2c_device_ioctl,
            mr_i2c_device_read,
            mr_i2c_device_write,
        };

    MR_ASSERT(i2c_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Initialize the private fields */
    i2c_device->config.baud_rate = 0;
    i2c_device->bus = MR_NULL;
    i2c_device->address = address;

    /* Add the device */
    return mr_device_add(&i2c_device->device, name, Mr_Device_Type_I2C, MR_OPEN_CLOSED, &device_ops, MR_NULL);
}

static mr_err_t mr_i2c_bus_open(mr_device_t device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    /* Enable i2c-bus using the default config */
    if (i2c_bus->config.baud_rate == 0)
    {
        i2c_bus->config = default_config;
    }

    return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

static mr_err_t mr_i2c_bus_close(mr_device_t device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;

    /* Disable i2c */
    i2c_bus->config.baud_rate = 0;

    return i2c_bus->ops->configure(i2c_bus, &i2c_bus->config);
}

/**
 * @brief This function adds the i2c bus device.
 *
 * @param i2c_bus The i2c bus device to be added.
 * @param name The name of the i2c bus device.
 * @param ops The operations of the i2c bus device.
 * @param data The data of the i2c bus device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_i2c_bus_add(mr_i2c_bus_t i2c_bus, const char *name, struct mr_i2c_bus_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
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
    i2c_bus->config.baud_rate = 0;
    i2c_bus->owner = MR_NULL;
    mr_mutex_init(&i2c_bus->lock);

    /* Protect every operation of the i2c-bus device */
    ops->configure = ops->configure ? ops->configure : err_io_i2c_configure;
    ops->start = ops->start ? ops->start : err_io_i2c_start;
    ops->stop = ops->stop ? ops->stop : err_io_i2c_stop;
    ops->write = ops->write ? ops->write : err_io_i2c_write;
    ops->read = ops->read ? ops->read : err_io_i2c_read;
    i2c_bus->ops = ops;

    /* Add the device */
    return mr_device_add(&i2c_bus->device, name, Mr_Device_Type_I2CBUS, MR_OPEN_RDWR, &device_ops, data);
}

static mr_err_t err_io_soft_i2c_bus_configure(mr_soft_i2c_bus_t i2c_bus)
{
    return -MR_ERR_IO;
}

static void err_io_soft_i2c_scl_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{

}

static void err_io_soft_i2c_sda_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{

}

static mr_uint8_t err_io_soft_i2c_sda_read(mr_soft_i2c_bus_t i2c_bus)
{
    return 0;
}

static mr_uint8_t mr_soft_i2c_bus_wait_ack(mr_soft_i2c_bus_t soft_i2c_bus)
{
    mr_uint8_t ack = 0;

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    ack = soft_i2c_bus->ops->sda_read(soft_i2c_bus);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);

    return ack;
}

static void mr_soft_i2c_bus_send_ack(mr_soft_i2c_bus_t soft_i2c_bus, mr_uint8_t ack)
{
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);

    if (ack)
    {
        soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_LOW);
    } else
    {
        soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);
    }

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
}

static mr_err_t mr_soft_i2c_bus_configure(mr_i2c_bus_t i2c_bus, struct mr_i2c_config *config)
{
    mr_soft_i2c_bus_t soft_i2c_bus = (mr_soft_i2c_bus_t)i2c_bus;

    soft_i2c_bus->delay = 1000000u / config->baud_rate;
    return soft_i2c_bus->ops->configure(soft_i2c_bus);
}

static void mr_soft_i2c_bus_start(mr_i2c_bus_t i2c_bus)
{
    mr_soft_i2c_bus_t soft_i2c_bus = (mr_soft_i2c_bus_t)i2c_bus;

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);

    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
}

static void mr_soft_i2c_bus_stop(mr_i2c_bus_t i2c_bus)
{
    mr_soft_i2c_bus_t soft_i2c_bus = (mr_soft_i2c_bus_t)i2c_bus;

    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_LOW);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);

    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);
}

static void mr_soft_i2c_bus_write(mr_i2c_bus_t i2c_bus, mr_uint8_t data)
{
    mr_soft_i2c_bus_t soft_i2c_bus = (mr_soft_i2c_bus_t)i2c_bus;

    for (mr_size_t write_size = 0; write_size < 8; write_size++)
    {
        if (data & 0x80)
        {
            soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);
        } else
        {
            soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_LOW);
        }
        data = data << 1;

        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    }

    mr_soft_i2c_bus_wait_ack(soft_i2c_bus);
}

static mr_uint8_t mr_soft_i2c_bus_read(mr_i2c_bus_t i2c_bus, mr_state_t ack)
{
    mr_soft_i2c_bus_t soft_i2c_bus = (mr_soft_i2c_bus_t)i2c_bus;
    mr_uint8_t data = 0;

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);

    for (mr_size_t read_size = 0; read_size < 8; read_size++)
    {
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        data = data << 1;
        if (soft_i2c_bus->ops->sda_read(soft_i2c_bus) == MR_HIGH)
        {
            data |= 1;
        }
    }

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    mr_soft_i2c_bus_send_ack(soft_i2c_bus, ack);

    return data;
}

/**
 * @brief This function adds the soft i2c bus device.
 *
 * @param i2c_bus The i2c bus device to be added.
 * @param name The name of the i2c bus device.
 * @param ops The operations of the i2c bus device.
 * @param data The data of the i2c bus device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_i2c_bus_add(mr_soft_i2c_bus_t i2c_bus, const char *name, struct mr_soft_i2c_ops *ops, void *data)
{
    static struct mr_i2c_bus_ops i2c_bus_ops =
        {
            mr_soft_i2c_bus_configure,
            mr_soft_i2c_bus_start,
            mr_soft_i2c_bus_stop,
            mr_soft_i2c_bus_write,
            mr_soft_i2c_bus_read,
        };

    MR_ASSERT(i2c_bus != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    i2c_bus->delay = 0;

    /* Protect every operation of the soft-i2c-bus device */
    ops->configure = ops->configure ? ops->configure : err_io_soft_i2c_bus_configure;
    ops->scl_write = ops->scl_write ? ops->scl_write : err_io_soft_i2c_scl_write;
    ops->sda_write = ops->sda_write ? ops->sda_write : err_io_soft_i2c_sda_write;
    ops->sda_read = ops->sda_read ? ops->sda_read : err_io_soft_i2c_sda_read;
    i2c_bus->ops = ops;

    /* Add the i2c-bus device */
    return mr_i2c_bus_add(&i2c_bus->i2c_bus, name, &i2c_bus_ops, data);
}

#endif