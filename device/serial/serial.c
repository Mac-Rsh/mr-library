/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/serial/serial.h"

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)

static mr_err_t mr_serial_rx_fifo_init(mr_serial_t serial, mr_size_t rx_bufsz)
{
    mr_uint8_t *buffer = MR_NULL;

    if (serial->rx_fifo.buffer)
    {
        mr_free(serial->rx_fifo.buffer);
        mr_fifo_init(&serial->rx_fifo, MR_NULL, 0);
    }

    serial->rx_bufsz = rx_bufsz;

    if (serial->rx_bufsz != 0)
    {
        buffer = mr_malloc(serial->rx_bufsz);
        if (buffer == MR_NULL)
        {
            return -MR_ERR_NO_MEMORY;
        }
        mr_fifo_init(&serial->rx_fifo, buffer, serial->rx_bufsz);
    }

    return MR_ERR_OK;
}

static mr_err_t mr_serial_tx_fifo_init(mr_serial_t serial, mr_size_t tx_bufsz)
{
    mr_uint8_t *buffer = MR_NULL;

    if (serial->tx_fifo.buffer)
    {
        mr_free(serial->tx_fifo.buffer);
        mr_fifo_init(&serial->tx_fifo, MR_NULL, 0);
    }

    serial->tx_bufsz = tx_bufsz;

    if (serial->tx_bufsz != 0)
    {
        buffer = mr_malloc(serial->tx_bufsz);
        if (buffer == MR_NULL)
        {
            return -MR_ERR_NO_MEMORY;
        }
        mr_fifo_init(&serial->tx_fifo, buffer, serial->tx_bufsz);
    }

    return MR_ERR_OK;
}

static mr_err_t mr_serial_open(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_err_t ret = MR_ERR_OK;
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;

    /* Allocate the fifo buffer */
    ret = mr_serial_rx_fifo_init(serial, serial->rx_bufsz);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    ret = mr_serial_tx_fifo_init(serial, serial->tx_bufsz);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Setting serial to default config, if baud_rate not set */
    if (serial->config.baud_rate == 0)
    {
        serial->config = default_config;
    }

    return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_close(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_err_t ret = MR_ERR_OK;

    /* Free the fifo buffer */
    ret = mr_serial_rx_fifo_init(serial, 0);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    ret = mr_serial_tx_fifo_init(serial, 0);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }
    serial->rx_bufsz = MR_CONF_SERIAL_RX_BUFSZ;
    serial->tx_bufsz = MR_CONF_SERIAL_TX_BUFSZ;

    /* Setting serial to close config */
    serial->config.baud_rate = 0;

    return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_CONFIG:
        {
            if (args)
            {
                ret = serial->ops->configure(serial, (struct mr_serial_config *)args);
                if (ret == MR_ERR_OK)
                {
                    serial->config = *(struct mr_serial_config *)args;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_RX_CB:
        {
            device->rx_cb = args;
            return MR_ERR_OK;
        }

        case MR_CTRL_SET_RX_BUFSZ:
        {
            if (args)
            {
                return mr_serial_rx_fifo_init(serial, *((mr_size_t *)args));
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_TX_BUFSZ:
        {
            if (args)
            {
                return mr_serial_tx_fifo_init(serial, *((mr_size_t *)args));
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_serial_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
    mr_size_t recv_size = 0;

    if (serial->rx_bufsz == 0)
    {
        /* Blocking read */
        for (recv_size = 0; recv_size < size; recv_size += sizeof(*recv_buffer))
        {
            *recv_buffer = serial->ops->read(serial);
            recv_buffer++;
        }
    } else
    {
        /* Interrupt read */
        for (recv_size = 0; recv_size < size;)
        {
            recv_size += mr_fifo_read(&serial->rx_fifo, recv_buffer + recv_size, size - recv_size);
        }
    }

    return (mr_ssize_t)recv_size;
}

static mr_ssize_t mr_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
    mr_size_t send_size = 0;
    mr_size_t dma_size = 0;

    if (serial->tx_bufsz == 0 || (device->open_flag & MR_OPEN_NONBLOCKING) == 0)
    {
        /* Blocking write */
        for (send_size = 0; send_size < size; send_size += sizeof(*send_buffer))
        {
            serial->ops->write(serial, *send_buffer);
            send_buffer++;
        }
    } else
    {
        /* Non-blocking write */
        for (send_size = 0; send_size < size;)
        {
            send_size += mr_fifo_write(&serial->tx_fifo, send_buffer + send_size, size - send_size);

            if (mr_fifo_get_data_size(&serial->tx_fifo) == 0)
            {
                if((device->open_flag & MR_OPEN_DMA) == 0)
                {
                    /* Interrupt write */
                    serial->ops->start_tx(serial);
                }
                else
                {
                    /* DMA write */
                    dma_size = mr_fifo_read(&serial->tx_fifo, serial->tx_dma, sizeof(serial->tx_dma));
                    serial->ops->start_dma_tx(serial, serial->tx_dma, dma_size);
                }
            }
        }
    }

    return (mr_ssize_t)send_size;
}

static mr_err_t _err_io_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static void _err_io_serial_write(mr_serial_t serial, mr_uint8_t data)
{
    MR_ASSERT(0);
}

static mr_uint8_t _err_io_serial_read(mr_serial_t serial)
{
    MR_ASSERT(0);
    return 0;
}

static void _err_io_serial_start_tx(mr_serial_t serial)
{
    MR_ASSERT(0);
}

static void _err_io_serial_stop_tx(mr_serial_t serial)
{
    MR_ASSERT(0);
}

static void _err_io_serial_start_dma_tx(mr_serial_t serial, mr_uint8_t *buffer, mr_size_t size)
{
    MR_ASSERT(0);
}

static void _err_io_serial_stop_dma_tx(mr_serial_t serial)
{
    MR_ASSERT(0);
}

mr_err_t mr_serial_device_add(mr_serial_t serial, const char *name, void *data, struct mr_serial_ops *ops)
{
    const static struct mr_device_ops device_ops =
            {
                    mr_serial_open,
                    mr_serial_close,
                    mr_serial_ioctl,
                    mr_serial_read,
                    mr_serial_write,
            };

    MR_ASSERT(serial != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    serial->device.type = MR_DEVICE_TYPE_SERIAL;
    serial->device.data = data;
    serial->device.ops = &device_ops;

    serial->config.baud_rate = 0;
    serial->rx_bufsz = MR_CONF_SERIAL_RX_BUFSZ;
    serial->tx_bufsz = MR_CONF_SERIAL_TX_BUFSZ;
    mr_fifo_init(&serial->rx_fifo, MR_NULL, 0);
    mr_fifo_init(&serial->tx_fifo, MR_NULL, 0);

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_serial_configure;
    ops->write = ops->write ? ops->write : _err_io_serial_write;
    ops->read = ops->read ? ops->read : _err_io_serial_read;
    ops->start_tx = ops->start_tx ? ops->start_tx : _err_io_serial_start_tx;
    ops->stop_tx = ops->stop_tx ? ops->stop_tx : _err_io_serial_stop_tx;
    ops->start_dma_tx = ops->start_dma_tx ? ops->start_dma_tx : _err_io_serial_start_dma_tx;
    ops->stop_dma_tx = ops->stop_dma_tx ? ops->stop_dma_tx : _err_io_serial_stop_dma_tx;
    serial->ops = ops;

    /* Add to the container */
    return mr_device_add(&serial->device, name, MR_OPEN_RDWR);
}

void mr_serial_device_isr(mr_serial_t serial, mr_uint32_t event)
{
    mr_uint32_t length = 0;
    mr_uint8_t data = 0;
    mr_size_t dma_size = 0;

    MR_ASSERT(serial != MR_NULL);

    switch (event & _MR_SERIAL_EVENT_MASK)
    {
        case MR_SERIAL_EVENT_RX_INT:
        {
            /* Read data into the fifo */
            data = serial->ops->read(serial);
            mr_fifo_write_force(&serial->rx_fifo, &data, 1);

            /* Invoke the rx-cb function */
            if (serial->device.rx_cb != MR_NULL)
            {
                length = mr_fifo_get_data_size(&serial->rx_fifo);
                serial->device.rx_cb(&serial->device, &length);
            }
            break;
        }

        case MR_SERIAL_EVENT_TX_INT:
        {
            length = mr_fifo_get_data_size(&serial->tx_fifo);
            if (length == 0)
            {
                serial->ops->stop_tx(serial);

                /* Invoke the tx-cb function */
                if (serial->device.tx_cb != MR_NULL)
                {
                    serial->device.tx_cb(&serial->device, &length);
                }
                break;
            }

            /* Write data from the fifo */
            mr_fifo_read(&serial->tx_fifo, &data, 1);
            serial->ops->write(serial, data);
            break;
        }

        case MR_SERIAL_EVENT_RX_DMA:
        {
            dma_size = (event >> 16) & 0xffff;
            mr_fifo_write_force(&serial->rx_fifo, serial->rx_dma, dma_size);

            /* Invoke the rx-cb function */
            if (serial->device.rx_cb != MR_NULL)
            {
                length = mr_fifo_get_data_size(&serial->rx_fifo);
                serial->device.rx_cb(&serial->device, &length);
            }
            break;
        }

        case MR_SERIAL_EVENT_TX_DMA:
        {
            length = mr_fifo_get_data_size(&serial->tx_fifo);
            if (length == 0)
            {
                serial->ops->stop_dma_tx(serial);

                /* Invoke the tx-cb function */
                if (serial->device.tx_cb != MR_NULL)
                {
                    serial->device.tx_cb(&serial->device, &length);
                }
                break;
            }

            /* Write data from the fifo to the dma */
            dma_size = mr_fifo_read(&serial->tx_fifo, serial->tx_dma, sizeof(serial->tx_dma));
            serial->ops->start_dma_tx(serial, serial->tx_dma, dma_size);
            break;
        }

        default:
            break;
    }
}

#endif /* MR_CONF_SERIAL */