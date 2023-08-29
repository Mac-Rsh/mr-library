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

#if (MR_CFG_SPI == MR_CFG_ENABLE)

#if (MR_CFG_PIN == MR_CFG_ENABLE)
#include "device/pin/pin.h"
#endif

static mr_err_t err_io_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
    return -MR_ERR_IO;
}

static void err_io_spi_write(mr_spi_bus_t spi_bus, mr_uint8_t data)
{

}

static mr_uint8_t err_io_spi_read(mr_spi_bus_t spi_bus)
{
    return 0;
}

static void err_io_spi_cs_write(mr_spi_bus_t spi_bus, mr_pos_t cs_number, mr_uint8_t value)
{

}

static mr_uint8_t err_io_spi_cs_read(mr_spi_bus_t spi_bus, mr_pos_t cs_number)
{
    return 0;
}

static mr_err_t mr_spi_device_attach_bus(mr_spi_device_t spi_device, const char *name)
{
    mr_device_t spi_bus = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    /* Detach the spi-bus */
    if (name == MR_NULL)
    {
        if (spi_device->bus != MR_NULL)
        {
            /* Get the spi-bus */
            spi_bus = (mr_device_t)spi_device->bus;

            /* Close the spi-bus */
            ret = mr_device_close(spi_bus);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }

            /* Clear the spi-bus */
            spi_device->bus = MR_NULL;
            spi_device->device.support_flag = MR_OPEN_CLOSED;
        }

        return MR_ERR_OK;
    }

    /* Attach the spi-bus */
    spi_bus = mr_device_find(name);
    if (spi_bus == MR_NULL || spi_bus->type != Mr_Device_Type_SPIBUS)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Open the spi-bus */
    ret = mr_device_open(spi_bus, MR_OPEN_RDWR);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Set the spi-bus */
    spi_device->bus = (mr_spi_bus_t)spi_bus;
    spi_device->device.support_flag = spi_bus->support_flag;

    return MR_ERR_OK;
}

mr_err_t mr_spi_device_take_bus(mr_spi_device_t spi_device)
{
    mr_spi_bus_t spi_bus = spi_device->bus;
    mr_err_t ret = MR_ERR_OK;

    /* Take the mutex lock of the spi-bus */
    ret = mr_mutex_take(&spi_bus->lock, &spi_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Check if the spi-bus is different from the current one */
    if (spi_bus->owner != spi_device)
    {
        /* If the configuration is different, the spi-bus is reconfigured */
        if (spi_device->config.baud_rate != spi_bus->config.baud_rate
            || spi_device->config.host_slave != spi_bus->config.host_slave
            || spi_device->config.mode != spi_bus->config.mode
            || spi_device->config.bit_order != spi_bus->config.bit_order)
        {
            ret = spi_bus->ops->configure(spi_bus, &spi_device->config);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
        }

        /* Sets the spi-bus owner to the current spi-device */
        spi_bus->config = spi_device->config;
        spi_bus->owner = spi_device;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_release_bus(mr_spi_device_t spi_device)
{
    mr_spi_bus_t spi_bus = spi_device->bus;

    /* Release the mutex lock of the spi-bus */
    return mr_mutex_release(&spi_bus->lock, &spi_device->device.object);
}

static mr_uint8_t mr_spi_device_transfer(mr_spi_device_t spi_device, mr_uint8_t data)
{
    mr_spi_bus_t spi_bus = spi_device->bus;

    spi_bus->ops->write(spi_bus, data);
    return spi_bus->ops->read(spi_bus);
}

static mr_err_t mr_spi_device_configure_cs(mr_spi_device_t spi_device, mr_state_t state)
{
#if (MR_CFG_PIN == MR_CFG_ENABLE)
    struct mr_pin_config pin_config;
    mr_device_t pin = MR_NULL;

    pin_config.number = spi_device->cs_number;
    pin_config.mode = (state == MR_ENABLE) ? MR_PIN_MODE_OUTPUT : MR_PIN_MODE_NONE;

    /* Configure pin */
    if (spi_device->config.cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        pin = mr_device_find("pin");
        if (pin != MR_NULL)
        {
            mr_device_ioctl(pin, MR_CTRL_SET_CONFIG, &pin_config);
        }
    }
#endif

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_open(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    /* Enable spi using the default config */
    if (spi_device->config.baud_rate == 0)
    {
        spi_device->config = default_config;
    }
    mr_spi_device_configure_cs(spi_device, MR_ENABLE);

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_close(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    /* Disable spi */
    spi_device->config.baud_rate = 0;
    mr_spi_device_configure_cs(spi_device, MR_DISABLE);

    return mr_spi_device_attach_bus(spi_device, MR_NULL);
}

static mr_err_t mr_spi_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    switch (cmd & MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_spi_config_t config = (mr_spi_config_t)args;
                spi_device->config = *config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_spi_config_t config = (mr_spi_config_t)args;
                *config = spi_device->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_RX_CB:
        {
            device->rx_cb = args;
            return MR_ERR_OK;
        }

        case MR_CTRL_ATTACH:
        {
            return mr_spi_device_attach_bus(spi_device, (const char *)args);
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_spi_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_spi_bus_t spi_bus = MR_NULL;
    mr_size_t read_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Get the spi-bus */
    spi_bus = spi_device->bus;

    if (spi_device->config.host_slave == MR_SPI_HOST)
    {
        /* Start the chip-select of the current device */
        spi_bus->ops->cs_write(spi_bus, spi_device->cs_number, spi_device->config.cs_active);

        /* Send position */
        if (pos != 0)
        {
            mr_size_t bits = 1 << spi_device->config.pos_bits;

            while (bits--)
            {
                mr_spi_device_transfer(spi_device, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        /* Blocking read */
        while ((read_size += sizeof(*read_buffer)) <= size)
        {
            *read_buffer = mr_spi_device_transfer(spi_device, 0u);
            read_buffer++;
        }

        /* Stop the chip-select of the current device */
        spi_bus->ops->cs_write(spi_bus, spi_device->cs_number, !spi_device->config.cs_active);
    } else
    {
        if (spi_bus->rx_fifo.size == 0)
        {
            /* Blocking read */
            while ((read_size += sizeof(*read_buffer)) <= size)
            {
                *read_buffer = mr_spi_device_transfer(spi_device, 0u);
                read_buffer++;
            }
        } else
        {
            /* Non-blocking read */
            while (read_size < size)
            {
                read_size += mr_rb_read(&spi_bus->rx_fifo, read_buffer + read_size, size - read_size);
            }
        }
    }

    /* Release spi-bus */
    mr_spi_device_release_bus(spi_device);

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_spi_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_spi_bus_t spi_bus = MR_NULL;
    mr_size_t write_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take the spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Get the spi-bus */
    spi_bus = spi_device->bus;

    if (spi_device->config.host_slave == MR_SPI_HOST)
    {
        /* Start the chip-select of the current device */
        spi_bus->ops->cs_write(spi_bus, spi_device->cs_number, spi_device->config.cs_active);

        /* Send position */
        if (pos != 0)
        {
            mr_size_t bits = 1 << spi_device->config.pos_bits;

            while (bits--)
            {
                mr_spi_device_transfer(spi_device, (mr_uint8_t)pos);
                pos >>= 8;
            }
        }

        /* Blocking write */
        while ((write_size += sizeof(*write_buffer)) <= size)
        {
            mr_spi_device_transfer(spi_device, *write_buffer);
            write_buffer++;
        }

        /* Stop the chip-select of the current device */
        spi_bus->ops->cs_write(spi_bus, spi_device->cs_number, !spi_device->config.cs_active);
    } else
    {
        /* Blocking write */
        while ((write_size += sizeof(*write_buffer)) <= size)
        {
            mr_spi_device_transfer(spi_device, *write_buffer);
            write_buffer++;
        }
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
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_pos_t cs_number)
{
    static struct mr_device_ops device_ops =
        {
            mr_spi_device_open,
            mr_spi_device_close,
            mr_spi_device_ioctl,
            mr_spi_device_read,
            mr_spi_device_write,
        };

    MR_ASSERT(spi_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Initialize the private fields */
    spi_device->config.baud_rate = 0;
    spi_device->bus = MR_NULL;
    spi_device->cs_number = cs_number;

    /* Add the device */
    return mr_device_add(&spi_device->device, name, Mr_Device_Type_SPI, MR_OPEN_CLOSED, &device_ops, MR_NULL);
}

static mr_err_t mr_spi_bus_set_fifo(mr_rb_t rb, mr_size_t fifo_size)
{
    mr_uint8_t *pool = MR_NULL;

    /* Free old buffer */
    if (rb->size != 0)
    {
        mr_free(rb->buffer);
    }

    /* Allocate new buffer */
    pool = mr_malloc(fifo_size);
    if (pool == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_rb_init(rb, pool, fifo_size);

    return MR_ERR_OK;
}

static mr_err_t mr_spi_bus_open(mr_device_t device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    /* Enable spi using the default config */
    if (spi_bus->config.baud_rate == 0)
    {
        spi_bus->config = default_config;
    }

    return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_bus_close(mr_device_t device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;

    /* Disable spi */
    spi_bus->config.baud_rate = 0;
    mr_spi_bus_set_fifo(&spi_bus->rx_fifo, 0);

    return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_bus_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;

    switch (cmd & MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_RX_BUFSZ:
        {
            if (args)
            {
                mr_size_t bufsz = *((mr_size_t *)args);
                return mr_spi_bus_set_fifo(&spi_bus->rx_fifo, bufsz);
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
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
            mr_spi_bus_open,
            mr_spi_bus_close,
            mr_spi_bus_ioctl,
            MR_NULL,
            MR_NULL,
        };

    MR_ASSERT(spi_bus != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    spi_bus->config.baud_rate = 0;
    spi_bus->owner = MR_NULL;
    mr_mutex_init(&spi_bus->lock);
    mr_rb_init(&spi_bus->rx_fifo, MR_NULL, 0);

    /* Protect every operation of the spi-bus device */
    ops->configure = ops->configure ? ops->configure : err_io_spi_configure;
    ops->write = ops->write ? ops->write : err_io_spi_write;
    ops->read = ops->read ? ops->read : err_io_spi_read;
    ops->cs_write = ops->cs_write ? ops->cs_write : err_io_spi_cs_write;
    ops->cs_read = ops->cs_read ? ops->cs_read : err_io_spi_cs_read;
    spi_bus->ops = ops;

    /* Add the device */
    return mr_device_add(&spi_bus->device, name, Mr_Device_Type_SPIBUS, MR_OPEN_RDWR, &device_ops, data);
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
            mr_spi_device_t spi_device = spi_bus->owner;

            if (spi_device->config.host_slave == MR_SPI_SLAVE)
            {
                /* Check if the chip-select is active */
                if (spi_device->config.cs_active != MR_SPI_CS_ACTIVE_NONE)
                {
                    if (spi_bus->ops->cs_read(spi_bus, spi_device->cs_number) != spi_device->config.cs_active)
                    {
                        break;
                    }
                }

                /* Read data into the fifo */
                mr_uint8_t data = spi_bus->ops->read(spi_bus);
                mr_rb_put_force(&spi_bus->rx_fifo, data);

                /* Call the receiving completion function */
                if (spi_device->device.rx_cb != MR_NULL)
                {
                    mr_size_t length = mr_rb_get_data_size(&spi_bus->rx_fifo);
                    spi_device->device.rx_cb(&spi_device->device, &length);
                }
            }
            break;
        }

        default:
            break;
    }
}

#endif