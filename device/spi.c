/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "spi.h"

#if (MR_CFG_SPI == MR_CFG_ENABLE)

#if (MR_CFG_PIN == MR_CFG_ENABLE)
#include "pin.h"
#endif

#define MR_SPI_RD                       0
#define MR_SPI_WR                       1
#define MR_SPI_RDWR                     2

static mr_err_t err_io_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
    return -MR_ERR_IO;
}

static void err_io_spi_write(mr_spi_bus_t spi_bus, mr_uint32_t data)
{

}

static mr_uint32_t err_io_spi_read(mr_spi_bus_t spi_bus)
{
    return 0;
}

static void err_io_spi_cs_write(mr_spi_bus_t spi_bus, mr_off_t cs_number, mr_level_t level)
{

}

static mr_level_t err_io_spi_cs_read(mr_spi_bus_t spi_bus, mr_off_t cs_number)
{
    return 0;
}

mr_err_t mr_spi_device_take_bus(mr_spi_device_t spi_device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)spi_device->bus;
    mr_err_t ret = MR_ERR_OK;

    /* Check if the spi-bus is valid */
    if (spi_bus == MR_NULL)
    {
        return -MR_ERR_UNSUPPORTED;
    }

    /* Take the mutex lock of the spi-bus */
    ret = mr_mutex_take(&spi_bus->lock, spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Check if the spi-bus owner is different from the current one */
    if (mr_mutex_get_owner(&spi_bus->lock) != spi_bus->owner)
    {
        /* If the configuration is different, the spi-bus is reconfigured */
        if (spi_device->config.baud_rate != spi_bus->config.baud_rate
            || spi_device->config.host_slave != spi_bus->config.host_slave
            || spi_device->config.mode != spi_bus->config.mode
            || spi_device->config.data_bits != spi_bus->config.data_bits
            || spi_device->config.bit_order != spi_bus->config.bit_order)
        {
            ret = spi_bus->ops->configure(spi_bus, &spi_device->config);
            if (ret != MR_ERR_OK)
            {
                /* Release the mutex lock of the spi-bus */
                mr_mutex_release(&spi_bus->lock, spi_device);
                return ret;
            }
        }

        /* Sets the spi-bus owner to the current spi-device */
        spi_bus->config = spi_device->config;
        spi_bus->owner = (mr_spi_device_t)mr_mutex_get_owner(&spi_bus->lock);
    }

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_release_bus(mr_spi_device_t spi_device)
{
    mr_spi_bus_t spi_bus = spi_device->bus;

    /* Release the mutex lock of the spi-bus */
    return mr_mutex_release(&spi_bus->lock, spi_device);
}

static mr_err_t mr_spi_device_connect_bus(mr_spi_device_t spi_device, const char *name)
{
    mr_device_t spi_bus = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    if (name != MR_NULL)
    {
        /* Connect the spi-bus */
        spi_bus = mr_device_find(name);
        if (spi_bus == MR_NULL || spi_bus->type != Mr_Device_Type_SPIBUS)
        {
            return -MR_ERR_NOT_FOUND;
        }
    }

    if (spi_bus != (mr_device_t)spi_device->bus)
    {
        /* Disconnect the old spi-bus */
        if (spi_device->bus != MR_NULL)
        {
            /* Release the mutex */
            if (spi_device->config.host_slave == MR_SPI_SLAVE)
            {
                mr_spi_device_release_bus(spi_device);
            }

            /* Close the spi-bus */
            ret = mr_device_close((mr_device_t)spi_device->bus);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
            spi_device->bus = MR_NULL;
        }

        /* Set the spi-bus */
        spi_device->bus = (mr_spi_bus_t)spi_bus;

        /* Slave mode monopolizes the bus */
        if (spi_device->config.host_slave == MR_SPI_SLAVE)
        {
            ret = mr_spi_device_take_bus(spi_device);
            if (ret != MR_ERR_OK)
            {
                spi_device->bus = MR_NULL;
                return ret;
            }
        }

        /* Open the spi-bus */
        return mr_device_open(spi_bus, MR_DEVICE_OFLAG_RDWR);
    }

    return MR_ERR_OK;
}

MR_INLINE void mr_spi_device_cs_set_state(mr_spi_device_t spi_device, mr_state_t state)
{
    mr_spi_bus_t spi_bus = spi_device->bus;

    if (spi_device->config.cs_active != MR_SPI_CS_ACTIVE_HARDWARE)
    {
        spi_bus->ops->cs_write(spi_bus, spi_device->cs_number, (mr_level_t)!(state ^ spi_device->config.cs_active));
    }
}

MR_INLINE mr_state_t mr_spi_device_cs_get_state(mr_spi_device_t spi_device)
{
    mr_spi_bus_t spi_bus = spi_device->bus;

    if (spi_device->config.cs_active != MR_SPI_CS_ACTIVE_HARDWARE)
    {
        return (mr_state_t)(spi_bus->ops->cs_read(spi_bus, spi_device->cs_number) == spi_device->config.cs_active);
    }

    return MR_ENABLE;
}

static mr_ssize_t mr_spi_device_transfer(mr_spi_device_t spi_device,
                                         void *write_data,
                                         void *read_data,
                                         mr_size_t size,
                                         mr_state_t rw)
{
    mr_spi_bus_t spi_bus = spi_device->bus;
    mr_size_t tf_size = 0;

    if (rw == MR_SPI_WR)
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                mr_uint8_t *w_data = (mr_uint8_t *)write_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_16:
            {
                mr_uint16_t *w_data = (mr_uint16_t *)write_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_32:
            {
                mr_uint32_t *w_data = (mr_uint32_t *)write_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            default:
                return -MR_ERR_INVALID;
        }
    } else if (rw == MR_SPI_RD)
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                mr_uint8_t *r_data = (mr_uint8_t *)read_data;
                while ((tf_size += (sizeof(*r_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, 0);
                    *r_data = spi_bus->ops->read(spi_bus);
                    r_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_16:
            {
                mr_uint16_t *r_data = (mr_uint16_t *)read_data;
                while ((tf_size += (sizeof(*r_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, 0);
                    *r_data = spi_bus->ops->read(spi_bus);
                    r_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_32:
            {
                mr_uint32_t *r_data = (mr_uint32_t *)read_data;
                while ((tf_size += (sizeof(*r_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, 0);
                    *r_data = spi_bus->ops->read(spi_bus);
                    r_data++;
                }
                break;
            }

            default:
                return -MR_ERR_INVALID;
        }
    } else
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                mr_uint8_t *w_data = (mr_uint8_t *)write_data;
                mr_uint8_t *r_data = (mr_uint8_t *)read_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    *r_data = spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_16:
            {
                mr_uint16_t *w_data = (mr_uint16_t *)write_data;
                mr_uint16_t *r_data = (mr_uint16_t *)read_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    *r_data = spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            case MR_SPI_DATA_BITS_32:
            {
                mr_uint32_t *w_data = (mr_uint32_t *)write_data;
                mr_uint32_t *r_data = (mr_uint32_t *)read_data;
                while ((tf_size += (sizeof(*w_data))) <= size)
                {
                    spi_bus->ops->write(spi_bus, *w_data);
                    *r_data = spi_bus->ops->read(spi_bus);
                    w_data++;
                }
                break;
            }

            default:
                return -MR_ERR_INVALID;
        }
    }

    return (mr_ssize_t)tf_size;
}

static mr_err_t mr_spi_device_configure_cs(mr_spi_device_t spi_device, mr_state_t state)
{
#if (MR_CFG_PIN == MR_CFG_ENABLE)
    struct mr_pin_config pin_config;
    mr_device_t pin = MR_NULL;

    pin_config.number = spi_device->cs_number;
    pin_config.mode = (state == MR_ENABLE) ? MR_PIN_MODE_OUTPUT : MR_PIN_MODE_NONE;

    /* Configure pin */
    if (spi_device->config.cs_active != MR_SPI_CS_ACTIVE_HARDWARE)
    {
        pin = mr_device_find("pin");
        if (pin != MR_NULL)
        {
            return mr_device_ioctl(pin, MR_DEVICE_CTRL_SET_CONFIG, &pin_config);
        }
    }
#endif

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_open(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    /* Reset fifo */
    mr_rb_reset(&spi_device->rx_fifo);
    mr_rb_reset(&spi_device->tx_fifo);

    return mr_spi_device_configure_cs(spi_device, MR_ENABLE);
}

static mr_err_t mr_spi_device_close(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    return mr_spi_device_configure_cs(spi_device, MR_DISABLE);
}

static mr_err_t mr_spi_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_spi_config_t config = (mr_spi_config_t)args;
                mr_err_t ret = MR_ERR_OK;

                if (config->host_slave != spi_device->config.host_slave)
                {
                    if (config->host_slave == MR_SPI_HOST)
                    {
                        if (spi_device->bus != MR_NULL)
                        {
                            mr_spi_device_release_bus(spi_device);
                        }
                    } else
                    {
                        if (spi_device->bus != MR_NULL)
                        {
                            /* Slave mode monopolizes the bus */
                            ret = mr_spi_device_take_bus(spi_device);
                            if (ret != MR_ERR_OK)
                            {
                                return ret;
                            }
                        }
                    }
                }
                spi_device->config = *config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_spi_config_t config = (mr_spi_config_t)args;
                *config = spi_device->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_SET_RX_CB:
        {
            device->rx_cb = (mr_device_cb_t)args;
            return MR_ERR_OK;
        }

        case MR_DEVICE_CTRL_CONNECT:
        {
            return mr_spi_device_connect_bus(spi_device, (const char *)args);
        }

        case MR_DEVICE_CTRL_SPI_TRANSFER:
        {
            if (args && (device->oflags & MR_DEVICE_OFLAG_RDWR) == MR_DEVICE_OFLAG_RDWR)
            {
                struct mr_spi_transfer *tf = (struct mr_spi_transfer *)args;
                mr_ssize_t tf_size = 0;
                mr_err_t ret = MR_ERR_OK;

                /* Take the spi-bus */
                ret = mr_spi_device_take_bus(spi_device);
                if (ret != MR_ERR_OK)
                {
                    return ret;
                }

                if (spi_device->config.host_slave == MR_SPI_HOST)
                {
                    /* Enable the chip-select of the current device */
                    mr_spi_device_cs_set_state(spi_device, MR_ENABLE);

                    tf_size =
                        mr_spi_device_transfer(spi_device, tf->write_buffer, tf->read_buffer, tf->size, MR_SPI_RDWR);

                    /* Disable the chip-select of the current device */
                    mr_spi_device_cs_set_state(spi_device, MR_DISABLE);
                } else
                {
                    tf_size =
                        mr_spi_device_transfer(spi_device, tf->write_buffer, tf->read_buffer, tf->size, MR_SPI_RDWR);
                }

                /* Release spi-bus */
                mr_spi_device_release_bus(spi_device);

                return (mr_err_t)tf_size;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_spi_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_ssize_t read_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    if (spi_device->config.host_slave == MR_SPI_HOST)
    {
        /* Enable the chip-select of the current device */
        mr_spi_device_cs_set_state(spi_device, MR_ENABLE);

        /* Send position */
        if (pos >= 0)
        {
            mr_spi_device_transfer(spi_device, &pos, MR_NULL, (spi_device->config.pos_bits >> 3), MR_SPI_WR);
        }

        /* Blocking read */
        read_size = mr_spi_device_transfer(spi_device, MR_NULL, read_buffer, size, MR_SPI_RD);

        /* Disable the chip-select of the current device */
        mr_spi_device_cs_set_state(spi_device, MR_DISABLE);
    } else
    {
        if (mr_rb_get_buffer_size(&spi_device->rx_fifo) == 0)
        {
            /* Blocking read */
            read_size = mr_spi_device_transfer(spi_device, MR_NULL, read_buffer, size, MR_SPI_RD);
        } else
        {
            /* Non-blocking read */
            read_size = (mr_ssize_t)mr_rb_read(&spi_device->rx_fifo, read_buffer, size);
        }
    }

    /* Release spi-bus */
    mr_spi_device_release_bus(spi_device);

    return read_size;
}

static mr_ssize_t mr_spi_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_ssize_t write_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    if (spi_device->config.host_slave == MR_SPI_HOST)
    {
        /* Enable the chip-select of the current device */
        mr_spi_device_cs_set_state(spi_device, MR_ENABLE);

        /* Send position */
        if (pos >= 0)
        {
            mr_spi_device_transfer(spi_device, &pos, MR_NULL, (spi_device->config.pos_bits >> 3), MR_SPI_WR);
        }

        /* Blocking write */
        write_size = mr_spi_device_transfer(spi_device, write_buffer, MR_NULL, size, MR_SPI_WR);

        /* Disable the chip-select of the current device */
        mr_spi_device_cs_set_state(spi_device, MR_DISABLE);
    } else
    {
        /* Blocking write */
        write_size = mr_spi_device_transfer(spi_device, write_buffer, MR_NULL, size, MR_SPI_WR);
    }

    /* Release spi-bus */
    mr_spi_device_release_bus(spi_device);

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the spi device.
 *
 * @param spi_device The spi device to be added.
 * @param name The name of the spi device.
 * @param cs_number The number of the chip-select.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_off_t cs_number)
{
    static struct mr_device_ops device_ops =
        {
            mr_spi_device_open,
            mr_spi_device_close,
            mr_spi_device_ioctl,
            mr_spi_device_read,
            mr_spi_device_write,
        };
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    MR_ASSERT(spi_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(cs_number >= 0);

    /* Initialize the private fields */
    spi_device->config = default_config;
    mr_rb_init(&spi_device->rx_fifo, MR_NULL, 0);
    mr_rb_init(&spi_device->tx_fifo, MR_NULL, 0);
    spi_device->cs_number = cs_number;
    spi_device->bus = MR_NULL;

    /* Allocate fifo using configuration size */
    mr_rb_allocate_buffer(&spi_device->rx_fifo, MR_CFG_SPI_RX_BUFSZ);
    mr_rb_allocate_buffer(&spi_device->rx_fifo, MR_CFG_SPI_TX_BUFSZ);

    /* Add the device */
    return mr_device_add(&spi_device->device, name, Mr_Device_Type_SPI, MR_DEVICE_OFLAG_RDWR, &device_ops, MR_NULL);
}

static mr_err_t mr_spi_bus_close(mr_device_t device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;

    /* Disable spi */
    spi_bus->config.baud_rate = 0;
    mr_mutex_init(&spi_bus->lock);

    return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

/**
 * @brief This function adds the spi bus device.
 *
 * @param spi_bus The spi bus device to be added.
 * @param name The name of the spi bus device.
 * @param ops The operations of the spi bus device.
 * @param data The data of the spi bus device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, struct mr_spi_bus_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
        {
            MR_NULL,
            mr_spi_bus_close,
            MR_NULL,
            MR_NULL,
            MR_NULL,
        };

    MR_ASSERT(spi_bus != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    spi_bus->config.baud_rate = 0;
    mr_mutex_init(&spi_bus->lock);
    spi_bus->owner = MR_NULL;

    /* Protect every operation of the spi-bus device */
    ops->configure = ops->configure ? ops->configure : err_io_spi_configure;
    ops->write = ops->write ? ops->write : err_io_spi_write;
    ops->read = ops->read ? ops->read : err_io_spi_read;
    ops->cs_write = ops->cs_write ? ops->cs_write : err_io_spi_cs_write;
    ops->cs_read = ops->cs_read ? ops->cs_read : err_io_spi_cs_read;
    spi_bus->ops = ops;

    /* Add the device */
    return mr_device_add(&spi_bus->device, name, Mr_Device_Type_SPIBUS, MR_DEVICE_OFLAG_RDWR, &device_ops, data);
}

/**
 * @brief This function service interrupt routine of the spi bus device.
 *
 * @param spi_bus The spi bus device.
 * @param event The interrupt event.
 */
void mr_spi_bus_isr(mr_spi_bus_t spi_bus, mr_uint32_t event)
{
    MR_ASSERT(spi_bus != MR_NULL);

    switch (event & MR_SPI_BUS_EVENT_MASK)
    {
        case MR_SPI_BUS_EVENT_RX_INT:
        {
            mr_spi_device_t spi_device = (mr_spi_device_t)mr_mutex_get_owner(&spi_bus->lock);

            /* Check if the spi device is valid */
            if (spi_device != MR_NULL)
            {
                /* Check if the chip-select is active */
                if (mr_spi_device_cs_get_state(spi_device) != MR_ENABLE)
                {
                    return;
                }

                /* Save data to the fifo */
                mr_uint32_t data = spi_bus->ops->read(spi_bus);
                mr_rb_write_force(&spi_device->rx_fifo, &data, (spi_device->config.data_bits >> 3));

                /* Call the receiving completion function */
                if (spi_device->device.rx_cb != MR_NULL)
                {
                    mr_size_t size = mr_rb_get_data_size(&spi_device->rx_fifo);
                    spi_device->device.rx_cb(&spi_device->device, &size);
                }
            }
            break;
        }

        default:
            break;
    }
}

#endif