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

static mr_err_t mr_serial_open(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t)device;
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;
    struct mr_serial_fifo *fifo = MR_NULL;

    /* Allocation the fifo memory */
    if (serial->fifo_bufsz != 0)
    {
        fifo = (struct mr_serial_fifo *)mr_malloc(sizeof(struct mr_serial_fifo) + serial->fifo_bufsz);
        if (fifo == MR_NULL)
        {
            return -MR_ERR_NO_MEMORY;
        }
        mr_fifo_init(&fifo->fifo, fifo->pool, serial->fifo_bufsz);
        serial->rx_fifo = fifo;
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
    struct mr_serial_fifo *fifo = MR_NULL;

    /* Release the fifo memory */
    if (serial->fifo_bufsz != 0)
    {
        fifo = (struct mr_serial_fifo *)serial->rx_fifo;
        mr_free(fifo);
        serial->rx_fifo = MR_NULL;
    }

    /* Setting serial to close config */
    serial->config.baud_rate = 0;

    return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_err_t ret = MR_ERR_OK;
    mr_serial_t serial = (mr_serial_t)device;

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

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_serial_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_serial_t serial = (mr_serial_t)device;
    struct mr_serial_fifo *fifo = (struct mr_serial_fifo *)serial->rx_fifo;
    mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
    mr_size_t recv_size = 0;

    /* Read data from fifo */
    do
    {
        recv_size = mr_fifo_read(&fifo->fifo, recv_buffer, size);
    } while (recv_size == 0);

    return (mr_ssize_t)recv_size;
}

static mr_ssize_t mr_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
    mr_size_t send_size = 0;

    for (send_size = 0; send_size < size; send_size += sizeof(send_buffer))
    {
        serial->ops->write(serial, *send_buffer);
        send_buffer++;
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
    serial->fifo_bufsz = MR_CONF_SERIAL_BUFSZ;
    serial->rx_fifo = MR_NULL;

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_serial_configure;
    ops->write = ops->write ? ops->write : _err_io_serial_write;
    ops->read = ops->read ? ops->read : _err_io_serial_read;
    serial->ops = ops;

    /* Add to the container */
    return mr_device_add(&serial->device, name, MR_OPEN_RDWR);
}

void mr_serial_device_isr(mr_serial_t serial, mr_uint32_t event)
{
    struct mr_serial_fifo *rx_fifo = MR_NULL;
    mr_size_t length = 0;
    mr_uint8_t data = 0;

    MR_ASSERT(serial != MR_NULL);

    switch (event & _MR_SERIAL_EVENT_MASK)
    {
        case MR_SERIAL_EVENT_RX_INT:
        {
            rx_fifo = (struct mr_serial_fifo *)serial->rx_fifo;

            /* Read data into the fifo */
            data = serial->ops->read(serial);
            mr_fifo_write_force(&rx_fifo->fifo, &data, 1);

            /* Invoke the rx-cb function */
            if (serial->device.rx_cb != MR_NULL)
            {
                length = mr_fifo_get_data_size(&rx_fifo->fifo);
                serial->device.rx_cb(&serial->device, &length);
            }
            break;
        }

        default:
            break;
    }
}

#endif /* MR_CONF_SERIAL */