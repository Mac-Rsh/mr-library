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

#if (MR_CONF_PIN == MR_CONF_ENABLE)
#include "device/pin/pin.h"
#endif  /* MR_CONF_PIN */

#if (MR_CONF_SPI == MR_CONF_ENABLE)

static mr_err_t mr_spi_bus_rx_fifo_init(mr_spi_bus_t spi_bus, mr_size_t rx_bufsz)
{
    mr_uint8_t *buffer = MR_NULL;

    if (spi_bus->rx_fifo.fifo.buffer)
    {
        mr_free(spi_bus->rx_fifo.fifo.buffer);
        mr_fifo_init(&spi_bus->rx_fifo.fifo, MR_NULL, 0);
    }

    spi_bus->rx_fifo.bufsz = rx_bufsz;

    if (spi_bus->rx_fifo.bufsz != 0)
    {
        buffer = mr_malloc(spi_bus->rx_fifo.bufsz);
        if (buffer == MR_NULL)
        {
            return -MR_ERR_NO_MEMORY;
        }
        mr_fifo_init(&spi_bus->rx_fifo.fifo, buffer, spi_bus->rx_fifo.bufsz);
    }

    return MR_ERR_OK;
}

static mr_err_t mr_spi_bus_tx_fifo_init(mr_spi_bus_t spi_bus, mr_size_t tx_bufsz)
{
    mr_uint8_t *buffer = MR_NULL;

    if (spi_bus->tx_fifo.fifo.buffer)
    {
        mr_free(spi_bus->tx_fifo.fifo.buffer);
        mr_fifo_init(&spi_bus->tx_fifo.fifo, MR_NULL, 0);
    }

    spi_bus->tx_fifo.bufsz = tx_bufsz;

    if (spi_bus->tx_fifo.bufsz != 0)
    {
        buffer = mr_malloc(spi_bus->tx_fifo.bufsz);
        if (buffer == MR_NULL)
        {
            return -MR_ERR_NO_MEMORY;
        }
        mr_fifo_init(&spi_bus->tx_fifo.fifo, buffer, spi_bus->tx_fifo.bufsz);
    }

    return MR_ERR_OK;
}

mr_inline mr_uint8_t mr_spi_bus_transfer(mr_spi_bus_t spi_bus, mr_uint8_t data)
{
    spi_bus->ops->write(spi_bus, data);
    return spi_bus->ops->read(spi_bus);
}

mr_inline mr_err_t mr_spi_device_take_bus(mr_spi_device_t spi_device)
{
    mr_err_t ret = MR_ERR_OK;

    /* Without attach the bus */
    if (spi_device->bus == MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Take the mutex lock of the spi-bus */
    ret = mr_mutex_take(&spi_device->bus->lock, &spi_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Check if the current spi-device is the owner of the spi-bus */
    if (spi_device->bus->owner != spi_device)
    {
        /* If the configuration is different, the spi-bus is reconfigured */
        if (spi_device->config.baud_rate != spi_device->bus->config.baud_rate
            || spi_device->config.host_slave != spi_device->bus->config.host_slave
            || spi_device->config.mode != spi_device->bus->config.mode
            || spi_device->config.bit_order != spi_device->bus->config.bit_order)
        {
            ret = spi_device->bus->ops->configure(spi_device->bus, &spi_device->config);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
        }

        /* Sets the spi-bus owner to the current spi-device */
        spi_device->bus->config = spi_device->config;
        spi_device->bus->owner = spi_device;
    }

    return MR_ERR_OK;
}

mr_inline mr_err_t mr_spi_device_release_bus(mr_spi_device_t spi_device)
{
    mr_err_t ret = MR_ERR_OK;

    /* Without attach the bus */
    if (spi_device->bus == MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Release the mutex lock of the spi-bus */
    ret = mr_mutex_release(&spi_device->bus->lock, &spi_device->device.object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_spi_bus_open(mr_device_t device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;
    mr_err_t ret = MR_ERR_OK;

    /* Allocate the fifo buffer */
    ret = mr_spi_bus_rx_fifo_init(spi_bus, spi_bus->rx_fifo.bufsz);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    ret = mr_spi_bus_tx_fifo_init(spi_bus, spi_bus->tx_fifo.bufsz);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Setting spi-bus to default config, if the baud-rate not set */
    if (spi_bus->config.baud_rate == 0)
    {
        spi_bus->config = default_config;
    }

    return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_bus_close(mr_device_t device)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;
    mr_err_t ret = MR_ERR_OK;

    /* Allocate the fifo buffer */
    ret = mr_spi_bus_rx_fifo_init(spi_bus, 0);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    ret = mr_spi_bus_tx_fifo_init(spi_bus, 0);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    spi_bus->rx_fifo.bufsz = MR_CONF_SPI_RX_BUFSZ;
    spi_bus->tx_fifo.bufsz = MR_CONF_SPI_TX_BUFSZ;

    /* Setting spi-bus to close config */
    spi_bus->config.baud_rate = 0;

    return spi_bus->ops->configure(spi_bus, &spi_bus->config);
}

static mr_err_t mr_spi_bus_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_spi_bus_t spi_bus = (mr_spi_bus_t)device;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_RX_BUFSZ:
        {
            if (args)
            {
                return mr_spi_bus_rx_fifo_init(spi_bus, *((mr_size_t *)args));
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_TX_BUFSZ:
        {
            if (args)
            {
                return mr_spi_bus_tx_fifo_init(spi_bus, *((mr_size_t *)args));
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_err_t mr_spi_device_open(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    /* Setting spi-device to default config, if the baud-rate not set */
    if (spi_device->config.baud_rate == 0)
    {
        spi_device->config = default_config;
    }

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_close(mr_device_t device)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;

    /* Setting spi-device to close config */
    spi_device->config.baud_rate = 0;

    /* Disconnect from the spi-bus */
    spi_device->bus = MR_NULL;

    return MR_ERR_OK;
}

static mr_err_t mr_spi_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *buffer = MR_NULL;
    mr_size_t transfer_size = 0;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_CONFIG:
        {
            if (args)
            {
                spi_device->config = *(struct mr_spi_config *)args;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_RX_CB:
        {
            device->rx_cb = args;
            return MR_ERR_OK;
        }

        case MR_CTRL_SET_TX_CB:
        {
            device->tx_cb = args;
            return MR_ERR_OK;
        }

        case MR_CTRL_ATTACH:
        {
            /* Detach the spi-bus */
            if (args == MR_NULL)
            {
                /* Close the spi-bus */
                if (spi_device->bus != MR_NULL)
                {
                    ret = mr_device_close((mr_device_t)spi_device->bus);
                    if (ret != MR_ERR_OK)
                    {
                        return ret;
                    }
                }
                spi_device->bus = MR_NULL;
                return MR_ERR_OK;
            }

            /* Find the spi-bus */
            mr_device_t spi_bus = mr_device_find((char *)args);
            if (spi_bus == MR_NULL || spi_bus->type != MR_DEVICE_TYPE_SPI_BUS)
            {
                return -MR_ERR_NOT_FOUND;
            }

            /* Open the spi-bus */
            ret = mr_device_open(spi_bus, MR_OPEN_RDWR);
            if (ret != MR_ERR_OK)
            {
                return ret;
            }
            spi_device->bus = (mr_spi_bus_t)spi_bus;
            return MR_ERR_OK;
        }

        case MR_CTRL_TRANSFER:
        {
            if (args)
            {
                /* Take spi-bus */
                ret = mr_spi_device_take_bus(spi_device);
                if (ret != MR_ERR_OK)
                {
                    return ret;
                }

                do
                {
                    buffer = (mr_uint8_t *)((mr_transfer_t)args)->data;

                    for (transfer_size = 0;
                         transfer_size < ((mr_transfer_t)args)->size; transfer_size += sizeof(*buffer))
                    {
                        *buffer = mr_spi_bus_transfer(spi_device->bus, *buffer);
                        buffer++;
                    }

                    /* Get the next transfer */
                    args = ((mr_transfer_t)args)->next;
                } while (args != MR_NULL);

                /* Release spi-bus */
                mr_spi_device_release_bus(spi_device);
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_spi_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
    mr_spi_bus_t spi_bus = MR_NULL;
    mr_size_t recv_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    spi_bus = spi_device->bus;

    if (spi_device->config.host_slave == MR_SPI_HOST)
    {
        /* Start the chip-select of the current device */
        spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_ENABLE);

        /* Send position */
        if (pos != 0)
        {
            mr_spi_bus_transfer(spi_bus, (mr_uint8_t)pos);
        }

        /* Blocking read */
        for (recv_size = 0; recv_size < size; recv_size += sizeof(*recv_buffer))
        {
            *recv_buffer = mr_spi_bus_transfer(spi_bus, 0u);
            recv_buffer++;
        }

        /* Stop the chip-select of the current device */
        spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_DISABLE);
    } else
    {
        if (spi_bus->rx_fifo.bufsz == 0)
        {
            /* Blocking read */
            for (recv_size = 0; recv_size < size; recv_size += sizeof(*recv_buffer))
            {
                *recv_buffer = mr_spi_bus_transfer(spi_bus, 0u);
                recv_buffer++;
            }
        } else
        {
            /* Non-blocking read */
            for (recv_size = 0; recv_size < size;)
            {
                recv_size += mr_fifo_read(&spi_bus->rx_fifo.fifo, recv_buffer + recv_size, size - recv_size);
            }
        }
    }

    /* Release spi-bus */
    mr_spi_device_release_bus(spi_device);

    return (mr_ssize_t)recv_size;
}

static mr_ssize_t mr_spi_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_spi_device_t spi_device = (mr_spi_device_t)device;
    mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
    mr_spi_bus_t spi_bus = MR_NULL;
    mr_size_t send_size = 0;
    mr_uint8_t send_position = 0;
    mr_size_t dma_size = 0;
    mr_err_t ret = MR_ERR_OK;

    /* Take spi-bus */
    ret = mr_spi_device_take_bus(spi_device);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    spi_bus = spi_device->bus;

    if (spi_bus->tx_fifo.bufsz == 0 || (spi_device->device.open_flag & MR_OPEN_NONBLOCKING) == 0)
    {
        if (spi_device->config.host_slave == MR_SPI_HOST)
        {
            /* Start the chip-select of the current device */
            spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_ENABLE);

            /* Send position */
            if (pos != 0)
            {
                mr_spi_bus_transfer(spi_bus, (mr_uint8_t)pos);
            }
        }

        /* Blocking write */
        for (send_size = 0; send_size < size; send_size += sizeof(*send_buffer))
        {
            mr_spi_bus_transfer(spi_bus, *send_buffer);
            send_buffer++;
        }

        /* Stop the chip-select of the current device */
        if (spi_device->config.host_slave == MR_SPI_HOST)
        {
            spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_DISABLE);
        }
    } else
    {
        if (spi_device->config.host_slave == MR_SPI_HOST)
        {
            /* Send position */
            if (pos != 0)
            {
                send_position = (mr_uint8_t)pos;
                for (send_size = 0; send_size < sizeof(send_position);)
                {
                    send_size += mr_fifo_write(&spi_bus->tx_fifo.fifo, &send_position, sizeof(send_position));
                }
            }
        }

        /* Count the number of send */
        spi_device->tx_count += size;
        if (mr_list_is_empty(&spi_device->tx_list))
        {
            mr_list_insert_before(&spi_bus->tx_list, &spi_device->tx_list);
        }

        /* Non-blocking write */
        for (send_size = 0; send_size < size;)
        {
            /* If this is the first write, start sending */
            if (mr_fifo_get_data_size(&spi_bus->tx_fifo.fifo) != 0)
            {
                send_size += mr_fifo_write(&spi_bus->tx_fifo.fifo, send_buffer + send_size, size - send_size);
            } else
            {
                send_size += mr_fifo_write(&spi_bus->tx_fifo.fifo, send_buffer + send_size, size - send_size);

                /* Start the chip-select of the current device */
                if (spi_device->config.host_slave == MR_SPI_HOST)
                {
                    spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_ENABLE);
                }

                if ((device->open_flag & MR_OPEN_DMA) == 0)
                {
                    /* Interrupt write */
                    spi_bus->ops->start_tx(spi_bus);
                } else
                {
                    /* DMA write */
                    dma_size = mr_fifo_read(&spi_bus->tx_fifo.fifo,
                                            spi_bus->tx_dma,
                                            sizeof(spi_bus->tx_dma));
                    spi_bus->ops->start_dma_tx(spi_bus, spi_bus->tx_dma, dma_size);
                }
            }
        }
    }

    /* Release spi-bus */
    mr_spi_device_release_bus(spi_device);

    return (mr_ssize_t)send_size;
}

static mr_err_t _err_io_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
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

static void _err_io_spi_cs_ctrl(mr_spi_bus_t spi_bus, mr_pos_t cs_pin, mr_state_t state)
{
    MR_ASSERT(0);
}

static mr_uint8_t _err_io_spi_cs_read(mr_spi_bus_t spi_bus, mr_pos_t cs_pin)
{
    MR_ASSERT(0);
    return 0;
}

static void _err_io_spi_start_tx(mr_spi_bus_t spi_bus)
{
    MR_ASSERT(0);
}

static void _err_io_spi_stop_tx(mr_spi_bus_t spi_bus)
{
    MR_ASSERT(0);
}

mr_err_t mr_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, void *data, struct mr_spi_bus_ops *ops)
{
    const static struct mr_device_ops device_ops =
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
    spi_bus->device.type = MR_DEVICE_TYPE_SPI_BUS;
    spi_bus->device.data = data;
    spi_bus->device.ops = &device_ops;

    spi_bus->config.baud_rate = 0;
    spi_bus->owner = MR_NULL;
    mr_mutex_init(&spi_bus->lock);
    spi_bus->rx_fifo.bufsz = MR_CONF_SPI_RX_BUFSZ;
    mr_fifo_init(&spi_bus->rx_fifo.fifo, MR_NULL, 0);
    spi_bus->tx_fifo.bufsz = MR_CONF_SPI_TX_BUFSZ;
    mr_fifo_init(&spi_bus->tx_fifo.fifo, MR_NULL, 0);
    mr_list_init(&spi_bus->tx_list);

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_spi_configure;
    ops->write = ops->write ? ops->write : _err_io_spi_write;
    ops->read = ops->read ? ops->read : _err_io_spi_read;
    ops->cs_ctrl = ops->cs_ctrl ? ops->cs_ctrl : _err_io_spi_cs_ctrl;
    ops->cs_read = ops->cs_read ? ops->cs_read : _err_io_spi_cs_read;
    ops->start_tx = ops->start_tx ? ops->start_tx : _err_io_spi_start_tx;
    ops->stop_tx = ops->stop_tx ? ops->stop_tx : _err_io_spi_stop_tx;
    spi_bus->ops = ops;

    /* Add to the container */
    return mr_device_add(&spi_bus->device, name, MR_OPEN_RDWR);
}

mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_uint16_t cs_pin)
{
    const static struct mr_device_ops device_ops =
            {
                    mr_spi_device_open,
                    mr_spi_device_close,
                    mr_spi_device_ioctl,
                    mr_spi_device_read,
                    mr_spi_device_write,
            };
#if (MR_CONF_PIN == MR_CONF_ENABLE)
    struct mr_pin_config pin_config = {cs_pin, MR_PIN_MODE_OUTPUT};
    mr_device_t pin = MR_NULL;
#endif  /* MR_CONF_PIN */

    MR_ASSERT(spi_device != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Initialize the private fields */
    spi_device->device.type = MR_DEVICE_TYPE_SPI;
    spi_device->device.data = MR_NULL;
    spi_device->device.ops = &device_ops;

    spi_device->config.baud_rate = 0;
    spi_device->bus = MR_NULL;
    spi_device->cs_pin = cs_pin;
    mr_list_init(&spi_device->tx_list);
    spi_device->tx_count = 0;

#if (MR_CONF_PIN == MR_CONF_ENABLE)
    /* Configure pin */
    pin = mr_device_find("pin");
    if (pin == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }
    mr_device_open(pin, MR_OPEN_RDWR);
    mr_device_ioctl(pin, MR_CTRL_CONFIG, &pin_config);
#endif  /* MR_CONF_PIN */

    /* Add to the container */
    return mr_device_add(&spi_device->device, name, MR_OPEN_RDWR | MR_OPEN_NONBLOCKING);
}

void mr_spi_bus_isr(mr_spi_bus_t spi_bus, mr_uint32_t event)
{
    mr_spi_device_t spi_device = MR_NULL;
    mr_uint32_t length = 0;
    mr_uint8_t data = 0;

    MR_ASSERT(spi_bus != MR_NULL);

    switch (event & _MR_SPI_BUS_EVENT_MASK)
    {
        case MR_SPI_BUS_EVENT_RX_INT:
        {
            spi_device = spi_bus->owner;

            /* The data not belong to this device */
            if (spi_device->config.host_slave == MR_SPI_HOST ||
                spi_bus->ops->cs_read(spi_bus, spi_device->cs_pin) != spi_device->config.cs_active)
            {
                break;
            }

            /* Read data into the fifo */
            data = spi_bus->ops->read(spi_bus);
            mr_fifo_write_force(&spi_bus->rx_fifo.fifo, &data, sizeof(data));

            /* Invoke the rx-cb function */
            if (spi_device->device.rx_cb != MR_NULL)
            {
                length = mr_fifo_get_data_size(&spi_bus->rx_fifo.fifo);
                spi_device->device.rx_cb(&spi_device->device, &length);
            }
            break;
        }

        case MR_SPI_BUS_EVENT_TX_INT:
        {
            /* Check if the list is not empty */
            if (!mr_list_is_empty(&spi_bus->tx_list))
            {
                spi_device = mr_container_of(spi_bus->tx_list.next, struct mr_spi_device, tx_list);
                if (spi_device->tx_count == 0)
                {
                    /* Remove from the list */
                    mr_list_remove(&spi_device->tx_list);

                    /* Stop the chip-select of the current device */
                    if (spi_device->config.host_slave == MR_SPI_HOST)
                    {
                        spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_DISABLE);
                    }

                    /* Invoke the tx-cb function */
                    if (spi_device->device.tx_cb != MR_NULL)
                    {
                        spi_device->device.tx_cb(&spi_device->device, &spi_device->tx_count);
                    }

                    /* Select next device if list not empty */
                    if (!mr_list_is_empty(&spi_bus->tx_list))
                    {
                        spi_device = mr_container_of(spi_bus->tx_list.next, struct mr_spi_device, tx_list);

                        /* Start the chip-select of the current device */
                        if (spi_device->config.host_slave == MR_SPI_HOST)
                        {
                            spi_bus->ops->cs_ctrl(spi_bus, spi_device->cs_pin, MR_ENABLE);
                        }
                    }
                } else
                {
                    /* Decrement tx count */
                    spi_device->tx_count--;

                    /* Write data from the fifo */
                    mr_fifo_read(&spi_bus->tx_fifo.fifo, &data, sizeof(data));
                    mr_spi_bus_transfer(spi_bus, data);
                }
            } else
            {
                /* Stop transmission */
                spi_bus->ops->stop_tx(spi_bus);
            }
            break;
        }

        default:
            break;
    }
}

#endif  /* MR_CONF_SPI */