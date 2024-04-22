/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 * @date 2024-04-21    MacRsh       Reconstructed
 */

#include "../mr-library/device/include/mr_serial.h"

#ifdef MR_USE_SERIAL

#define _SERIAL_STATE_SENDING           (0x02)              /**< Sending state */

MR_INLINE ssize_t _serial_read_poll(struct mr_serial *serial, uint8_t *buf,
                                    size_t count)
{
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    ssize_t rcount;

    /* Receive data */
    for (rcount = 0; rcount < count; rcount += sizeof(*buf))
    {
        int ret = ops->receive(driver, buf);
        if (ret < 0)
        {
            /* If no data received, return the error code */
            return (rcount == 0) ? ret : rcount;
        }
        buf++;
    }

    /* Return the number of bytes received */
    return rcount;
}

MR_INLINE ssize_t _serial_read_fifo(struct mr_serial *serial, uint8_t *buf,
                                    size_t count)
{
    /* Receive data from FIFO */
    return (ssize_t)mr_fifo_read(&serial->rfifo, buf, count);
}

MR_INLINE ssize_t _serial_write_poll(struct mr_serial *serial,
                                     const uint8_t *buf, size_t count)
{
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    ssize_t wcount;

    /* Send data */
    for (wcount = 0; wcount < count; wcount += sizeof(*buf))
    {
        int ret = ops->send(driver, *buf);
        if (ret < 0)
        {
            /* If no data sent, return the error code */
            return (wcount == 0) ? ret : wcount;
        }
        buf++;
    }

    /* Return the number of bytes sent */
    return wcount;
}

MR_INLINE ssize_t _serial_write_fifo(struct mr_serial *serial,
                                     const uint8_t *buf, size_t count)
{
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Driver does not support this function */
    if (ops->send_int_configure == NULL)
    {
        return MR_EIO;
    }

    /* Send data to FIFO */
    ssize_t wcount = (ssize_t)mr_fifo_write(&serial->wfifo, buf, count);
    if (wcount <= 0)
    {
        return wcount;
    }

    /* If the serial port is not sending, enable it */
    if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_SENDING) == false)
    {
        /* Enable serial TX interrupt */
        int ret = ops->send_int_configure(driver, true);
        if (ret >= 0)
        {
            /* Data has been written to the FIFO, if the boot send fails, wait
             * for the next retry startup */
            return wcount;
        }

        /* Set the sending state */
        MR_BIT_SET(serial->state, _SERIAL_STATE_SENDING);
    }

    /* Return the number of bytes sent */
    return wcount;
}

MR_INLINE int _serial_fifo_update(struct mr_fifo *fifo, size_t *size)
{
    /* Allocate new buffer for FIFO */
    int ret = mr_fifo_allocate(fifo, *size);
    if (ret < 0)
    {
        /* Old buffer has been released */
        *size = 0;
        return ret;
    }
    return MR_EOK;
}

static int serial_open(struct mr_device *device)
{
    struct mr_serial *serial = (struct mr_serial *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Enable serial */
    int ret = ops->configure(driver, true, &serial->config);
    if (ret < 0)
    {
        return ret;
    }

    /* Allocate fifo */
    mr_fifo_allocate(&serial->rfifo, serial->rfifo_size);
    mr_fifo_allocate(&serial->wfifo, serial->wfifo_size);
    return MR_EOK;
}

static int serial_close(struct mr_device *device)
{
    struct mr_serial *serial = (struct mr_serial *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable serial */
    int ret = ops->configure(driver, false, &serial->config);
    if (ret < 0)
    {
        return ret;
    }

    /* Release fifo */
    mr_fifo_free(&serial->rfifo);
    mr_fifo_free(&serial->wfifo);
    return MR_EOK;
}

static ssize_t serial_read(struct mr_device *device, int pos, void *buf,
                           size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)device;

    /* If fifo is set, read from fifo */
    if (mr_fifo_size_get(&serial->rfifo) != 0)
    {
        return _serial_read_fifo(serial, buf, count);
    }

    /* Polling read data */
    return _serial_read_poll(serial, buf, count);
}

static ssize_t serial_write(struct mr_device *device, int pos, const void *buf,
                            size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)device;

    /* If fifo is set, write from fifo */
    if (mr_fifo_size_get(&serial->wfifo) != 0)
    {
        return _serial_write_fifo(serial, buf, count);
    }

    /* Polling write data */
    return _serial_write_poll(serial, buf, count);
}

static int serial_ioctl(struct mr_device *device, int pos, int cmd, void *args)
{
    struct mr_serial *serial = (struct mr_serial *)device;
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    switch (cmd)
    {
        case MR_CTRL_SET(MR_CMD_SERIAL_CONFIG):
        {
            struct mr_serial_config *config = (struct mr_serial_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Set new configuration */
            int ret = ops->configure(driver, true, config);
            if (ret < 0)
            {
                return ret;
            }

            /* Update configuration */
            serial->config = *config;
            return sizeof(*config);
        }
        case MR_CTRL_SET(MR_CMD_SERIAL_RD_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Update FIFO size */
            serial->rfifo_size = *fifo_size;
            int ret = _serial_fifo_update(&serial->rfifo, &serial->rfifo_size);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*fifo_size);
        }
        case MR_CTRL_SET(MR_CMD_SERIAL_WR_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Update FIFO size */
            serial->wfifo_size = *fifo_size;
            int ret = _serial_fifo_update(&serial->wfifo, &serial->wfifo_size);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*fifo_size);
        }
        case MR_CTRL_CLR(MR_CMD_SERIAL_RD_FIFO_DATA):
        {
            /* Reset FIFO */
            mr_fifo_reset(&serial->rfifo);
            return MR_EOK;
        }
        case MR_CTRL_CLR(MR_CMD_SERIAL_WR_FIFO_DATA):
        {
            /* Reset FIFO */
            mr_fifo_reset(&serial->wfifo);
            return MR_EOK;
        }
        case MR_CTRL_GET(MR_CMD_SERIAL_CONFIG):
        {
            struct mr_serial_config *config = (struct mr_serial_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get configuration */
            *config = serial->config;
            return sizeof(*config);
        }
        case MR_CTRL_GET(MR_CMD_SERIAL_RD_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get FIFO size */
            *fifo_size = serial->rfifo_size;
            return sizeof(*fifo_size);
        }
        case MR_CTRL_GET(MR_CMD_SERIAL_WR_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get FIFO size */
            *fifo_size = serial->wfifo_size;
            return sizeof(*fifo_size);
        }
        case MR_CTRL_GET(MR_CMD_SERIAL_RD_FIFO_DATA):
        {
            size_t *data_size = (size_t *)args;

            if (data_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get data size */
            *data_size = mr_fifo_used_get(&serial->rfifo);
            return sizeof(*data_size);
        }
        case MR_CTRL_GET(MR_CMD_SERIAL_WR_FIFO_DATA):
        {
            size_t *data_size = (size_t *)args;

            if (data_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get data size */
            *data_size = mr_fifo_used_get(&serial->wfifo);
            return sizeof(*data_size);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

static int serial_isr(struct mr_device *device, uint32_t event, void *args)
{
    struct mr_serial *serial = (struct mr_serial *)device;
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    switch (event)
    {
        case MR_EVENT_SERIAL_RD_COMPLETE_INT:
        {
            int count = 1;

            /* If FIFO is empty, the read operation is abandoned */
            if (mr_fifo_size_get(&serial->rfifo) == 0)
            {
                return MR_EOK;
            }

            if (args != NULL)
            {
                /* Hardware FIFO is considered to be used */
                count = *((int *)args);
            }

            /* Read all data from hardware FIFO */
            for (size_t rcount = 0; rcount < count; rcount++)
            {
                uint8_t data;

                /* Read data from serial */
                int ret = ops->receive(driver, &data);
                if (ret < 0)
                {
                    return ret;
                }

                /* Force write data to FIFO */
                mr_fifo_write_force(&serial->rfifo, &data, sizeof(data));
            }

            /* Return number of bytes received */
            return count;
        }
        case MR_EVENT_SERIAL_WR_COMPLETE_INT:
        {
            uint8_t data;

            /* Driver does not support this function */
            if (ops->send_int_configure == NULL)
            {
                return MR_EIO;
            }

            /* If FIFO is empty, the write operation is abandoned */
            if (mr_fifo_size_get(&serial->wfifo) == 0)
            {
                return MR_EOK;
            }

            /* Write data from FIFO, if FIFO is empty, stop transmit */
            if (mr_fifo_peek(&serial->wfifo, &data, sizeof(data)) ==
                sizeof(data))
            {
                /* Write data to serial */
                int ret = ops->send(driver, data);
                if (ret < 0)
                {
                    return ret;
                }

                /* Discard data from FIFO */
                mr_fifo_discard(&serial->wfifo, sizeof(data));
                return MR_EBUSY;
            } else
            {
                /* Stop sending */
                int ret = ops->send_int_configure(driver, false);
                if (ret < 0)
                {
                    /* If the stop fails, nothing can do */
                    return ret;
                }

                /* Clear the sending state */
                MR_BIT_CLR(serial->state, _SERIAL_STATE_SENDING);
                return MR_EOK;
            }
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function register a serial.
 *
 * @param serial The serial.
 * @param path The path of the serial.
 * @param driver The driver of the serial.
 *
 * @return The error code.
 */
int mr_serial_register(struct mr_serial *serial, const char *path,
                       struct mr_driver *driver)
{
    MR_ASSERT(serial != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL));

    static struct mr_device_ops ops = {.open = serial_open,
                                       .close = serial_close,
                                       .read = serial_read,
                                       .write = serial_write,
                                       .ioctl = serial_ioctl,
                                       .isr = serial_isr};
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;

    /* Initialize the serial */
    serial->config = default_config;
    mr_fifo_init(&serial->rfifo, NULL, 0);
    mr_fifo_init(&serial->wfifo, NULL, 0);
#ifndef MR_CFG_SERIAL_RD_FIFO_SIZE
#define MR_CFG_SERIAL_RD_FIFO_SIZE      (64)
#endif /* MR_CFG_SERIAL_RD_FIFO_SIZE */
#ifndef MR_CFG_SERIAL_WR_FIFO_SIZE
#define MR_CFG_SERIAL_WR_FIFO_SIZE      (0)
#endif /* MR_CFG_SERIAL_WR_FIFO_SIZE */
    serial->rfifo_size = MR_CFG_SERIAL_RD_FIFO_SIZE;
    serial->wfifo_size = MR_CFG_SERIAL_WR_FIFO_SIZE;
    serial->state = 0;

    /* Register the serial device */
    return mr_device_register((struct mr_device *)serial, path,
                              MR_DEVICE_TYPE_SERIAL | MR_DEVICE_TYPE_FDX, &ops,
                              driver);
}

#endif /* MR_USE_SERIAL */
