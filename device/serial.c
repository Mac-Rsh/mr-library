/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "serial.h"

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

static mr_err_t err_io_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    return -MR_ERR_IO;
}

static void err_io_serial_write(mr_serial_t serial, mr_uint8_t data)
{

}

static mr_uint8_t err_io_serial_read(mr_serial_t serial)
{
    return 0;
}

static void err_io_serial_start_tx(mr_serial_t serial)
{

}

static void err_io_serial_stop_tx(mr_serial_t serial)
{

}

static mr_err_t mr_serial_open(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t)device;

    /* Reset fifo */
    mr_rb_reset(&serial->rx_fifo);
    mr_rb_reset(&serial->tx_fifo);

    return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_close(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t)device;
    struct mr_serial_config config = {0};

    return serial->ops->configure(serial, &config);
}

static mr_err_t mr_serial_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_serial_config_t config = (mr_serial_config_t)args;
                ret = serial->ops->configure(serial, config);
                if (ret == MR_ERR_OK)
                {
                    serial->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_serial_config_t config = (mr_serial_config_t)args;
                *config = serial->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_SET_RX_CB:
        {
            device->rx_cb = (mr_device_cb_t)args;
            return MR_ERR_OK;
        }

        case MR_DEVICE_CTRL_SET_TX_CB:
        {
            device->tx_cb = (mr_device_cb_t)args;
            return MR_ERR_OK;
        }

        case MR_DEVICE_CTRL_SET_RX_BUFSZ:
        {
            if (args)
            {
                mr_size_t bufsz = *((mr_size_t *)args);
                return mr_rb_allocate_buffer(&serial->rx_fifo, bufsz);
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_SET_TX_BUFSZ:
        {
            if (args)
            {
                mr_size_t bufsz = *((mr_size_t *)args);
                return mr_rb_allocate_buffer(&serial->tx_fifo, bufsz);
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
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_size_t read_size = 0;

    if (mr_rb_get_buffer_size(&serial->rx_fifo) == 0)
    {
        /* Blocking read */
        while ((read_size += sizeof(*read_buffer)) <= size)
        {
            *read_buffer = serial->ops->read(serial);
            read_buffer++;
        }
    } else
    {
        /* Non-blocking read */
        read_size = mr_rb_read(&serial->rx_fifo, read_buffer, size);
    }

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_serial_t serial = (mr_serial_t)device;
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_size_t write_size = 0;

    if (mr_rb_get_buffer_size(&serial->tx_fifo) == 0 || ((device->oflags & MR_DEVICE_OFLAG_NONBLOCKING) == MR_FALSE))
    {
        /* Blocking write */
        while ((write_size += sizeof(*write_buffer)) <= size)
        {
            serial->ops->write(serial, *write_buffer);
            write_buffer++;
        }
    } else
    {
        /* Non-blocking write */
        write_size = mr_rb_write(&serial->tx_fifo, write_buffer, size);

        /* Start interrupt send */
        serial->ops->start_tx(serial);
    }

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the serial device.
 *
 * @param serial The serial device to be added.
 * @param name The name of the device.
 * @param ops The operations of the device.
 * @param data The private data of the device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_serial_device_add(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
        {
            mr_serial_open,
            mr_serial_close,
            mr_serial_ioctl,
            mr_serial_read,
            mr_serial_write,
        };
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;
    mr_uint8_t support_flag = MR_DEVICE_OFLAG_RDWR;

    MR_ASSERT(serial != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    serial->config = default_config;
    mr_rb_init(&serial->rx_fifo, MR_NULL, 0);
    mr_rb_init(&serial->tx_fifo, MR_NULL, 0);

    /* Allocate fifo using configuration size */
    mr_rb_allocate_buffer(&serial->rx_fifo, MR_CFG_SERIAL_RX_BUFSZ);
    mr_rb_allocate_buffer(&serial->tx_fifo, MR_CFG_SERIAL_TX_BUFSZ);

    /* Non-blocking mode */
    if (ops->start_tx != MR_NULL && ops->stop_tx != MR_NULL)
    {
        support_flag |= MR_DEVICE_OFLAG_NONBLOCKING;
    }

    /* Protect every operation of the serial device */
    ops->configure = ops->configure ? ops->configure : err_io_serial_configure;
    ops->write = ops->write ? ops->write : err_io_serial_write;
    ops->read = ops->read ? ops->read : err_io_serial_read;
    ops->start_tx = ops->start_tx ? ops->start_tx : err_io_serial_start_tx;
    ops->stop_tx = ops->stop_tx ? ops->stop_tx : err_io_serial_stop_tx;
    serial->ops = ops;

    /* Add the device */
    return mr_device_add(&serial->device, name, Mr_Device_Type_Serial, support_flag, &device_ops, data);
}

/**
 * @brief This function service interrupt routine of the serial device.
 *
 * @param serial The serial device.
 * @param event The interrupt event.
 */
void mr_serial_device_isr(mr_serial_t serial, mr_uint32_t event)
{
    MR_ASSERT(serial != MR_NULL);

    switch (event & MR_SERIAL_EVENT_MASK)
    {
        case MR_SERIAL_EVENT_RX_INT:
        {
            /* Save data to the fifo */
            mr_uint8_t data = serial->ops->read(serial);
            mr_rb_push_force(&serial->rx_fifo, data);

            /* Call the receiving completion function */
            if (serial->device.rx_cb != MR_NULL)
            {
                mr_size_t size = mr_rb_get_data_size(&serial->rx_fifo);
                serial->device.rx_cb(&serial->device, &size);
            }
            break;
        }

        case MR_SERIAL_EVENT_TX_INT:
        {
            /* Write data from the fifo */
            mr_uint8_t data = 0;
            if (mr_rb_pop(&serial->tx_fifo, &data) == sizeof(data))
            {
                serial->ops->write(serial, data);
            } else
            {
                /* Stop interrupt send */
                serial->ops->stop_tx(serial);

                /* Call the sending completion function */
                if (serial->device.tx_cb != MR_NULL)
                {
                    mr_size_t size = 0;
                    serial->device.tx_cb(&serial->device, &size);
                }
            }
            break;
        }

        default:
            break;
    }
}

#endif