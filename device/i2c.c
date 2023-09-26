/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "i2c.h"

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

static mr_err_t mr_i2c_device_take_bus(mr_i2c_device_t i2c_device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)i2c_device->bus;
    mr_err_t ret = MR_ERR_OK;

    /* Check if the i2c-bus is valid */
    if (i2c_bus == MR_NULL)
    {
        return -MR_ERR_UNSUPPORTED;
    }

    /* Take the mutex lock of the i2c-bus */
    ret = mr_mutex_take(&i2c_bus->lock, i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Check if the i2c-bus owner is different from the current one */
    if (mr_mutex_get_owner(&i2c_bus->lock) != i2c_bus->owner)
    {
        /* If the configuration is different, the i2c-bus is reconfigured */
        if (i2c_device->config.baud_rate != i2c_bus->config.baud_rate
            || i2c_device->config.host_slave != i2c_bus->config.host_slave
            || i2c_device->config.addr_bits != i2c_bus->config.addr_bits)
        {
            ret = i2c_bus->ops->configure(i2c_bus, &i2c_device->config);
            if (ret != MR_ERR_OK)
            {
                /* Release the mutex lock of the i2c-bus */
                mr_mutex_release(&i2c_bus->lock, i2c_device);
                return ret;
            }
        }

        /* Sets the i2c-bus owner to the current i2c-device */
        i2c_bus->config = i2c_device->config;
        i2c_bus->owner = (mr_i2c_device_t)mr_mutex_get_owner(&i2c_bus->lock);
    }

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_release_bus(mr_i2c_device_t i2c_device)
{
    mr_i2c_bus_t i2c_bus = i2c_device->bus;

    /* Release the mutex lock of the i2c-bus */
    return mr_mutex_release(&i2c_bus->lock, i2c_device);
}

static mr_err_t mr_i2c_device_connect_bus(mr_i2c_device_t i2c_device, const char *name)
{
    mr_device_t i2c_bus = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    if (name != MR_NULL)
    {
        /* Connect the i2c-bus */
        i2c_bus = mr_device_find(name);
        if (i2c_bus == MR_NULL || i2c_bus->type != Mr_Device_Type_I2CBUS)
        {
            return -MR_ERR_NOT_FOUND;
        }
    }

    if (i2c_bus != (mr_device_t)i2c_device->bus)
    {
        /* Disconnect the old i2c-bus */
        if (i2c_device->bus != MR_NULL)
        {
            /* Release the mutex */
            if (i2c_device->config.host_slave == MR_I2C_SLAVE)
            {
                mr_i2c_device_release_bus(i2c_device);
            }

            /* Close the i2c-bus */
            ret = mr_device_close((mr_device_t)i2c_device->bus);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
            i2c_device->bus = MR_NULL;
        }

        /* Set the i2c-bus */
        i2c_device->bus = (mr_i2c_bus_t)i2c_bus;

        /* Slave mode monopolizes the bus */
        if (i2c_device->config.host_slave == MR_I2C_SLAVE)
        {
            ret = mr_i2c_device_take_bus(i2c_device);
            if (ret != MR_ERR_OK)
            {
                i2c_device->bus = MR_NULL;
                return ret;
            }
        }

        /* Open the i2c-bus */
        return mr_device_open(i2c_bus, MR_DEVICE_OFLAG_RDWR);
    }

    return MR_ERR_OK;
}

MR_INLINE void mr_i2c_device_send_address(mr_i2c_device_t i2c_device, mr_state_t rw)
{
    mr_i2c_bus_t i2c_bus = i2c_device->bus;

    i2c_bus->ops->start(i2c_bus);
    if (rw)
    {
        i2c_bus->ops->write(i2c_bus, (mr_uint8_t)(i2c_device->address << 1));
    } else
    {
        i2c_bus->ops->write(i2c_bus, (mr_uint8_t)(i2c_device->address << 1 | 0x01));
    }
}

static mr_err_t mr_i2c_device_close(mr_device_t device)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

    /* Reset fifo */
    mr_rb_reset(&i2c_device->rx_fifo);
    mr_rb_reset(&i2c_device->tx_fifo);

    return MR_ERR_OK;
}

static mr_err_t mr_i2c_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_i2c_config_t config = (mr_i2c_config_t)args;
                mr_err_t ret = MR_ERR_OK;

                if (config->host_slave != i2c_device->config.host_slave)
                {
                    if (config->host_slave == MR_I2C_HOST)
                    {
                        if (i2c_device->bus != MR_NULL)
                        {
                            mr_i2c_device_release_bus(i2c_device);
                        }
                    } else
                    {
                        if (i2c_device->bus != MR_NULL)
                        {
                            /* Slave mode monopolizes the bus */
                            ret = mr_i2c_device_take_bus(i2c_device);
                            if (ret != MR_ERR_OK)
                            {
                                return ret;
                            }
                        }
                    }
                }
                i2c_device->config = *config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_i2c_config_t config = (mr_i2c_config_t)args;
                *config = i2c_device->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_CONNECT:
        {
            return mr_i2c_device_connect_bus(i2c_device, (const char *)args);
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_i2c_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)i2c_device->bus;
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_size_t read_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the i2c-bus */
    ret = mr_i2c_device_take_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    if (i2c_device->config.host_slave == MR_I2C_HOST)
    {
        /* Send position */
        if (pos >= 0)
        {
            mr_size_t bits = 0;

            /* Start and send the write command */
            mr_i2c_device_send_address(i2c_device, MR_I2C_WR);

            while ((bits += 8) <= i2c_device->config.pos_bits)
            {
                i2c_bus->ops->write(i2c_bus, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        /* Start and send the read command */
        mr_i2c_device_send_address(i2c_device, MR_I2C_RD);

        while ((read_size += sizeof(*read_buffer)) <= size)
        {
            *read_buffer = i2c_bus->ops->read(i2c_bus, (mr_state_t)(read_size != size));
            read_buffer++;
        }

        /* Stop read */
        i2c_bus->ops->stop(i2c_bus);
    } else
    {
        if (mr_rb_get_buffer_size(&i2c_device->rx_fifo) == 0)
        {
            /* Blocking read */
            while ((read_size += sizeof(*read_buffer)) <= size)
            {
                *read_buffer = i2c_bus->ops->read(i2c_bus, (mr_state_t)(read_size != size));
                read_buffer++;
            }
        } else
        {
            /* Non-blocking read */
            read_size = (mr_ssize_t)mr_rb_read(&i2c_device->rx_fifo, read_buffer, size);
        }
    }

    /* Release i2c-bus */
    mr_i2c_device_release_bus(i2c_device);

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_i2c_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_i2c_device_t i2c_device = (mr_i2c_device_t)device;
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)i2c_device->bus;
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_size_t write_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the i2c-bus */
    ret = mr_i2c_device_take_bus(i2c_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    if (i2c_device->config.host_slave == MR_I2C_HOST)
    {
        /* Start and send the write command */
        mr_i2c_device_send_address(i2c_device, MR_I2C_WR);

        /* Send position */
        if (pos >= 0)
        {
            mr_size_t bits = 0;

            while ((bits += 8) <= i2c_device->config.pos_bits)
            {
                i2c_bus->ops->write(i2c_bus, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        while ((write_size += sizeof(*write_buffer)) <= size)
        {
            i2c_bus->ops->write(i2c_bus, *write_buffer);
            write_buffer++;
        }

        /* Stop write */
        i2c_bus->ops->stop(i2c_bus);
    } else
    {
        /* Blocking write */
        while ((write_size += sizeof(*write_buffer)) <= size)
        {
            i2c_bus->ops->write(i2c_bus, *write_buffer);
            write_buffer++;
        }
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
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint32_t address)
{
    static struct mr_device_ops device_ops =
        {
            MR_NULL,
            mr_i2c_device_close,
            mr_i2c_device_ioctl,
            mr_i2c_device_read,
            mr_i2c_device_write,
        };
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    MR_ASSERT(i2c_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Initialize the private fields */
    i2c_device->config = default_config;
    mr_rb_init(&i2c_device->rx_fifo, MR_NULL, 0);
    mr_rb_init(&i2c_device->tx_fifo, MR_NULL, 0);
    i2c_device->address = address;
    i2c_device->bus = MR_NULL;

    /* Allocate fifo using configuration size */
    mr_rb_allocate_buffer(&i2c_device->rx_fifo, MR_CFG_I2C_RX_BUFSZ);
    mr_rb_allocate_buffer(&i2c_device->rx_fifo, MR_CFG_I2C_TX_BUFSZ);

    /* Add the device */
    return mr_device_add(&i2c_device->device, name, Mr_Device_Type_I2C, MR_DEVICE_OFLAG_RDWR, &device_ops, MR_NULL);
}

static mr_err_t mr_i2c_bus_close(mr_device_t device)
{
    mr_i2c_bus_t i2c_bus = (mr_i2c_bus_t)device;

    /* Disable i2c */
    i2c_bus->config.baud_rate = 0;
    mr_mutex_init(&i2c_bus->lock);

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
            MR_NULL,
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
    mr_mutex_init(&i2c_bus->lock);
    i2c_bus->owner = MR_NULL;

    /* Protect every operation of the i2c-bus device */
    ops->configure = ops->configure ? ops->configure : err_io_i2c_configure;
    ops->start = ops->start ? ops->start : err_io_i2c_start;
    ops->stop = ops->stop ? ops->stop : err_io_i2c_stop;
    ops->write = ops->write ? ops->write : err_io_i2c_write;
    ops->read = ops->read ? ops->read : err_io_i2c_read;
    i2c_bus->ops = ops;

    /* Add the device */
    return mr_device_add(&i2c_bus->device, name, Mr_Device_Type_I2CBUS, MR_DEVICE_OFLAG_RDWR, &device_ops, data);
}

static mr_err_t err_io_soft_i2c_bus_configure(mr_soft_i2c_bus_t i2c_bus)
{
    return -MR_ERR_IO;
}

static void err_io_soft_i2c_scl_write(mr_soft_i2c_bus_t i2c_bus, mr_level_t level)
{

}

static void err_io_soft_i2c_sda_write(mr_soft_i2c_bus_t i2c_bus, mr_level_t level)
{

}

static mr_level_t err_io_soft_i2c_sda_read(mr_soft_i2c_bus_t i2c_bus)
{
    return 0;
}

static mr_state_t mr_soft_i2c_bus_wait_ack(mr_soft_i2c_bus_t soft_i2c_bus)
{
    mr_level_t ack = 0;

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    ack = soft_i2c_bus->ops->sda_read(soft_i2c_bus);
    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);

    return (mr_state_t)ack;
}

static void mr_soft_i2c_bus_send_ack(mr_soft_i2c_bus_t soft_i2c_bus, mr_state_t ack)
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
    mr_size_t bits = 0;

    for (bits = 0; bits < 8; bits++)
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
    mr_size_t bits = 0;

    soft_i2c_bus->ops->scl_write(soft_i2c_bus, MR_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus->ops->sda_write(soft_i2c_bus, MR_HIGH);

    for (bits = 0; bits < 8; bits++)
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