/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-09     MacRsh       first version
 */

#include "serial.h"

mr_err_t mr_serial_init(mr_serial_t serial, struct mr_serial_config *config)
{
    mr_err_t error_code = MR_ERR_OK;
    struct mr_serial_fifo *fifo = MR_NULL;

    /* Check that the fifo has been allocated */
    if (serial->rx_fifo != MR_NULL || serial->tx_fifo != MR_NULL)
        return -MR_ERR_GENERIC;

    if (config->fifo_size != 0)
    {
        if (config->fifo_size < MR_SERIAL_FIFO_MIN_SIZE)
            config->fifo_size = MR_SERIAL_FIFO_MIN_SIZE;

        /* Allocation the rx-fifo memory */
        fifo = (struct mr_serial_fifo *) mr_malloc(sizeof(struct mr_serial_fifo *) + config->fifo_size);
        if (fifo == MR_NULL)
            return -MR_ERR_NO_MEMORY;

        mr_ringbuffer_init(&fifo->ringbuffer, fifo->pool, config->fifo_size);
        serial->rx_fifo = fifo;
        fifo = MR_NULL;

        /* Allocation the tx-fifo memory */
        fifo = (struct mr_serial_fifo *) mr_malloc(sizeof(struct mr_serial_fifo *) + config->fifo_size);
        if (fifo == MR_NULL)
            return -MR_ERR_NO_MEMORY;

        mr_ringbuffer_init(&fifo->ringbuffer, fifo->pool, config->fifo_size);
        serial->tx_fifo = fifo;
        fifo = MR_NULL;
    }

    /* Initialize the underlying hardware */
    if (serial->ops->configure != MR_NULL)
    {
        error_code = serial->ops->configure(serial, config);
        if (error_code != MR_ERR_OK)
            return error_code;

        serial->config = *config;
        return MR_ERR_OK;
    } else
        return -MR_ERR_IO;
}

mr_err_t mr_serial_uninit(mr_serial_t serial)
{
    struct mr_serial_fifo *fifo = MR_NULL;

    if (serial->config.fifo_size != 0)
    {
        /* Check that the fifo has been allocated */
        if (serial->rx_fifo == MR_NULL || serial->tx_fifo == MR_NULL)
            return -MR_ERR_GENERIC;

        /* Release the rx-fifo memory */
        fifo = (struct mr_serial_fifo *) serial->rx_fifo;
        mr_free(fifo);
        serial->rx_fifo = MR_NULL;

        /* Release the tx-fifo memory */
        fifo = (struct mr_serial_fifo *) serial->tx_fifo;
        mr_free(fifo);
        serial->tx_fifo = MR_NULL;
    }

    /* Uninitialize the underlying hardware */
    if (serial->ops->configure != MR_NULL)
    {
        serial->config.baud_rate = 0;
        return serial->ops->configure(serial, &serial->config);
    } else
        return -MR_ERR_IO;
}

mr_size_t mr_serial_write(mr_serial_t serial, const mr_uint8_t *buffer, mr_size_t count)
{
    mr_size_t send_count = count;

    if (serial->ops->write_byte != MR_NULL)
    {
        while (send_count--)
        {
            serial->ops->write_byte(serial, *buffer);
            buffer++;
        }

        return count;
    } else
    {
//        MR_LOG_E();
        return 0;
    }
}

mr_size_t mr_serial_write_byte(mr_serial_t serial, mr_uint8_t data)
{
    if (serial->ops->write_byte != MR_NULL)
    {
        serial->ops->write_byte(serial, data);
        return 1;
    } else
    {
//        MR_LOG_E();
        return 0;
    }
}

mr_size_t mr_serial_read(mr_serial_t serial, mr_uint8_t *buffer, mr_size_t count)
{
    struct mr_serial_fifo *fifo = MR_NULL;
    mr_size_t recv_count;

    if (serial->config.fifo_size != 0)
    {
        fifo = (struct mr_serial_fifo *) serial->rx_fifo;
        do
        {
            recv_count = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
        } while (recv_count < count);

        return mr_ringbuffer_read(&fifo->ringbuffer, buffer, count);
    } else
        return 0;
}

mr_size_t mr_serial_read_byte(mr_serial_t serial, mr_uint8_t *data)
{
    struct mr_serial_fifo *fifo = MR_NULL;
    mr_size_t recv_count;

    if (serial->config.fifo_size != 0)
    {
        fifo = (struct mr_serial_fifo *) serial->rx_fifo;
        do
        {
            recv_count = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
        } while (recv_count < 1);

        return mr_ringbuffer_read_byte(&fifo->ringbuffer, data);
    } else
        return 0;
}

static mr_err_t mr_device_serial_open(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t) device;

    return mr_serial_init(serial, &serial->config);
}

static mr_err_t mr_device_serial_close(mr_device_t device)
{
    mr_serial_t serial = (mr_serial_t) device;

    return mr_serial_uninit(serial);
}

static mr_err_t mr_device_serial_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_serial_t serial = (mr_serial_t) device;
    struct mr_serial_config config = serial->config;

    switch (cmd & _MR_CMD_MASK)
    {
        case MR_CMD_CONFIG:
        {
            config = *(struct mr_serial_config *) args;
            /* The fifo size cannot be changed once the device is opened */
            if ((config.fifo_size != serial->config.fifo_size) && (device->ref_count > 0))
                return -MR_ERR_BUSY;
            else
                return mr_serial_init(serial, &config);
        }

        case MR_CMD_SET_RX_CALLBACK:
        {
            device->rx_callback = (mr_err_t (*)(mr_device_t device, void *args)) args;
            return MR_ERR_OK;
        }

        default:return -MR_ERR_UNSUPPORTED;
    }
}

static mr_size_t mr_device_serial_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t count)
{
    mr_serial_t serial = (mr_serial_t) device;

    if (count == 0) return 0;
    return mr_serial_read(serial, buffer, count);
}

static mr_size_t mr_device_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t count)
{
    mr_serial_t serial = (mr_serial_t) device;

    if (count == 0) return 0;
    return mr_serial_write(serial, buffer, count);
}

mr_err_t mr_serial_add_to_container(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data)
{
    mr_err_t error_code = MR_ERR_OK;
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;
    const static struct mr_serial_ops null_ops = {MR_NULL};
    const static struct mr_device_ops device_ops =
        {
            .open = mr_device_serial_open,
            .close = mr_device_serial_close,
            .ioctl = mr_device_serial_ioctl,
            .read = mr_device_serial_read,
            .write = mr_device_serial_write,
        };

    /* Initialize the device and add the device to the container */
    mr_device_init(&serial->device, name);
    error_code = mr_device_add_to_container(&serial->device, MR_DEVICE_TYPE_SERIAL, MR_OPEN_RDWR, &device_ops, data);
    if (error_code != MR_ERR_OK)
        return error_code;

    /* Initialize the serial fields */
    serial->config = default_config;
    serial->tx_fifo = MR_NULL;
    serial->rx_fifo = MR_NULL;

    /* Set serial's operations as null_ops if ops is null */
    serial->ops = (ops == MR_NULL) ? &null_ops : ops;

    return MR_ERR_OK;
}

void mr_hw_serial_isr(mr_serial_t serial, mr_uint16_t event)
{
    switch (event & _MR_SERIAL_EVENT_MASK)
    {
        case MR_SERIAL_EVENT_RX_INT:
        {
            struct mr_serial_fifo *fifo = (struct mr_serial_fifo *) serial->rx_fifo;
            mr_uint8_t data = 0;

            /* Read data into the ring buffer */
            data = serial->ops->read_byte(serial);
            mr_ringbuffer_write_byte_force(&fifo->ringbuffer, data);

            /* Invoke the rx-callback function */
            if (serial->device.rx_callback != MR_NULL)
            {
                mr_size_t rx_length = 0;

                rx_length = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
                serial->device.rx_callback(&serial->device, &rx_length);
            }
            break;
        }

        default:break;
    }
}
