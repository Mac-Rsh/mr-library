/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 * @date 2024-05-08    MacRsh       Added support for DMA
 */

#include "../mr-library/include/device/mr_serial.h"

#ifdef MR_USE_SERIAL

#define _SERIAL_STATE_RECEIVE_INT_ASYNC (0x01 << 0)         /**< Receive interrupt async */
#define _SERIAL_STATE_SEND_INT          (0x01 << 8)         /**< Send interrupt */
#define _SERIAL_STATE_SEND_INT_ASYNC    (0x02 << 8)         /**< Send interrupt async */
#define _SERIAL_STATE_RECEIVE_DMA       (0x01 << 16)        /**< Receive DMA */
#define _SERIAL_STATE_RECEIVE_DMA_TOP   (0x02 << 16)        /**< Receive DMA top */
#define _SERIAL_STATE_RECEIVE_DMA_BOT   (0x04 << 16)        /**< Receive DMA bot */
#define _SERIAL_STATE_RECEIVE_DMA_ASYNC (0x08 << 16)        /**< Receive DMA async */

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

#ifdef MR_USE_SERIAL_ASYNC
MR_INLINE ssize_t _serial_async_read_int(struct mr_serial *serial, uint8_t *buf,
                                         size_t count)
{
    /* Receive data from FIFO */
    size_t rcount = mr_fifo_read(&serial->rfifo, buf, count);

    /* If there is enough data to read from the FIFO, async operations are not
     * required */
    if (rcount == count)
    {
        return rcount;
    }

    /* Set the async receive buffer and count */
    serial->rabuf = buf + rcount;
    serial->racount = count - rcount;
    MR_BIT_SET(serial->state, _SERIAL_STATE_RECEIVE_INT_ASYNC);
    return MR_EOK;
}

#ifdef MR_USE_SERIAL_DMA
MR_INLINE ssize_t _serial_async_read_dma(struct mr_serial *serial, uint8_t *buf,
                                         size_t count)
{
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)serial);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Receive data from FIFO */
    size_t rcount = mr_fifo_read(&serial->rfifo, buf, count);

    /* If there is enough data to read from the FIFO, async operations are not
     * required */
    if (rcount == count)
    {
        return rcount;
    }

    /* Stop the current transmission */
    if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA) == true)
    {
        ops->receive_dma(driver, false, NULL, 0);
        MR_BIT_CLR(serial->state, _SERIAL_STATE_RECEIVE_DMA |
                                      _SERIAL_STATE_RECEIVE_DMA_TOP |
                                      _SERIAL_STATE_RECEIVE_DMA_BOT);
    }

    /* Set the async receive buffer and count */
    serial->rabuf = buf + rcount;
    serial->racount = count - rcount;

    /* Start the receive DMA */
    MR_BIT_SET(serial->state,
               _SERIAL_STATE_RECEIVE_DMA | _SERIAL_STATE_RECEIVE_DMA_ASYNC);
    int ret = ops->receive_dma(driver, true, serial->rabuf, serial->racount);
    if (ret < 0)
    {
        MR_BIT_CLR(serial->state,
                   _SERIAL_STATE_RECEIVE_DMA | _SERIAL_STATE_RECEIVE_DMA_ASYNC);
        return ret;
    }
    return MR_EOK;
}
#endif /* MR_USE_SERIAL_DMA */
#endif /* MR_USE_SERIAL_ASYNC */

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
    if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_SEND_INT) == false)
    {
        /* Enable serial TX interrupt */
        int ret = ops->send_int_configure(driver, true);
        if (ret >= 0)
        {
            /* Data has been written to the FIFO, if the boot sent fails, wait
             * for the next retry startup */
            return wcount;
        }

        /* Set the sending state */
        MR_BIT_SET(serial->state, _SERIAL_STATE_SEND_INT);
    }

    /* Return the number of bytes sent */
    return wcount;
}

MR_INLINE int _serial_fifo_allocate(struct mr_fifo *fifo, size_t *size)
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

    /* Allocate FIFO */
    mr_fifo_allocate(&serial->rfifo, serial->rfifo_size);
    mr_fifo_allocate(&serial->wfifo, serial->wfifo_size);

#ifdef MR_USE_SERIAL_DMA
    /* If the driver supports DMA, start DMA to receive data */
    if (ops->receive_dma != NULL)
    {
        MR_BIT_SET(serial->state,
                   _SERIAL_STATE_RECEIVE_DMA | _SERIAL_STATE_RECEIVE_DMA_TOP);
        ops->receive_dma(driver, true, serial->rdma, sizeof(serial->rdma) / 2);
    }
#endif /* MR_USE_SERIAL_DMA */
    return MR_EOK;
}

static int serial_close(struct mr_device *device)
{
    struct mr_serial *serial = (struct mr_serial *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable serial TX interrupt */
    if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_SEND_INT) == true)
    {
        ops->send_int_configure(driver, false);
        MR_BIT_CLR(serial->state, _SERIAL_STATE_SEND_INT);
    }

#ifdef MR_USE_SERIAL_DMA
    /* Stop DMA */
    if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA) == true)
    {
        ops->receive_dma(driver, false, NULL, 0);
        MR_BIT_CLR(serial->state, _SERIAL_STATE_RECEIVE_DMA |
                                      _SERIAL_STATE_RECEIVE_DMA_TOP |
                                      _SERIAL_STATE_RECEIVE_DMA_BOT);
    }
#endif /* MR_USE_SERIAL_DMA */

    /* Disable serial */
    int ret = ops->configure(driver, false, NULL);
    if (ret < 0)
    {
        return ret;
    }

    /* Release FIFO */
    mr_fifo_free(&serial->rfifo);
    mr_fifo_free(&serial->wfifo);
    return MR_EOK;
}

static ssize_t serial_read(struct mr_device *device, int pos, void *buf,
                           size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)device;

    /* If FIFO is set, read from FIFO */
    if (mr_fifo_size_get(&serial->rfifo) != 0)
    {
        return _serial_read_fifo(serial, buf, count);
    }

    /* Polling read data */
    return _serial_read_poll(serial, buf, count);
}

#ifdef MR_USE_SERIAL_ASYNC
static ssize_t serial_read_async(struct mr_device *device, int pos, void *buf,
                                 size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)device;

    /* Check if the serial port is busy */
    if (serial->state &
        (_SERIAL_STATE_RECEIVE_INT_ASYNC | _SERIAL_STATE_RECEIVE_DMA_ASYNC))
    {
        return MR_EBUSY;
    }

#ifdef MR_USE_SERIAL_DMA
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_serial_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* If the driver supports DMA, start DMA to async read data */
    if (ops->receive_dma != NULL)
    {
        return _serial_async_read_dma(serial, buf, count);
    }
#endif /* MR_USE_SERIAL_DMA */

    /* Interrupt async read data */
    return _serial_async_read_int(serial, buf, count);
}
#endif /* MR_USE_SERIAL_ASYNC */

static ssize_t serial_write(struct mr_device *device, int pos, const void *buf,
                            size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)device;

    /* If FIFO is set, write from FIFO */
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
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
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
            int ret =
                _serial_fifo_allocate(&serial->rfifo, &serial->rfifo_size);
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
            int ret =
                _serial_fifo_allocate(&serial->wfifo, &serial->wfifo_size);
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
            size_t count = 1;

            if (args != NULL)
            {
                /* Hardware FIFO is considered to be used */
                count = *((size_t *)args);
            }

#ifdef MR_USE_SERIAL_ASYNC
            if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_RECEIVE_INT_ASYNC) ==
                true)
            {
                size_t rcount;

                /* Read all data from hardware FIFO */
                for (rcount = 0; rcount < count; rcount++)
                {
                    uint8_t data;

                    /* Read data from serial */
                    int ret = ops->receive(driver, &data);
                    if (ret < 0)
                    {
                        return ret;
                    }
                    *serial->rabuf = data;
                    serial->rabuf++;
                    serial->racount--;

                    /* If there is no more data to read, exit the loop */
                    if (serial->racount == 0)
                    {
                        /* Stop async read operation */
                        MR_BIT_CLR(serial->state,
                                   _SERIAL_STATE_RECEIVE_INT_ASYNC);
                        break;
                    }
                }

                /* Async read operation is incomplete */
                if (serial->racount != 0)
                {
                    return MR_EBUSY;
                }

                /* If there is any data left, it is stored in the buffer */
                count -= rcount;
                if (count == 0)
                {
                    return MR_EOK;
                }
            }
#endif /* MR_USE_SERIAL_ASYNC */

            /* If FIFO is empty, the read operation is abandoned */
            if (mr_fifo_size_get(&serial->rfifo) == 0)
            {
                return MR_EOK;
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
            return MR_EOK;
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

            /* If FIFO is empty, stop sending */
            if (mr_fifo_peek(&serial->wfifo, &data, sizeof(data)) !=
                sizeof(data))
            {
                /* Stop sending */
                int ret = ops->send_int_configure(driver, false);
                if (ret < 0)
                {
                    /* If the stop is failed, nothing can do */
                    return ret;
                }

                /* Clear the sending state */
                MR_BIT_CLR(serial->state, _SERIAL_STATE_SEND_INT);
                return MR_EOK;
            }

            /* Write data to serial */
            int ret = ops->send(driver, data);
            if (ret < 0)
            {
                return ret;
            }

            /* Discard data from FIFO */
            mr_fifo_discard(&serial->wfifo, sizeof(data));
            return MR_EBUSY;
        }
#ifdef MR_USE_SERIAL_DMA
        case MR_EVENT_SERIAL_RD_COMPLETE_DMA:
        {
            size_t count = sizeof(serial->rdma) / 2;
            uint8_t *rdma, *ndma;

            /* Driver does not support this function */
            if (ops->receive_dma == NULL)
            {
                return MR_EIO;
            }

            if (args != NULL)
            {
                /* Only partially done */
                count = *((size_t *)args);

                /* At most half of DMA buffer is used */
                if (count > sizeof(serial->rdma) / 2)
                {
                    count = sizeof(serial->rdma) / 2;
                }
            }

            /* Ping-pong operation */
            if (MR_BIT_IS_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA_TOP) ==
                true)
            {
                /* Top half of DMA buffer is used */
                rdma = serial->rdma;
                ndma = &serial->rdma[sizeof(serial->rdma) / 2];
                MR_BIT_CLR(serial->state, _SERIAL_STATE_RECEIVE_DMA_TOP);
                MR_BIT_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA_BOT);
            } else if (MR_BIT_IS_SET(serial->state,
                                     _SERIAL_STATE_RECEIVE_DMA_BOT) == true)
            {
                /* Bottom half of DMA buffer is used */
                rdma = &serial->rdma[sizeof(serial->rdma) / 2];
                ndma = serial->rdma;
                MR_BIT_CLR(serial->state, _SERIAL_STATE_RECEIVE_DMA_BOT);
                MR_BIT_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA_TOP);
            }
#ifdef MR_USE_SERIAL_ASYNC
            else if (MR_BIT_IS_SET(serial->state,
                                   _SERIAL_STATE_RECEIVE_DMA_ASYNC) == true)
            {
                count = serial->racount;

                if (args != NULL)
                {
                    /* Only partially done */
                    count = *((size_t *)args);

                    /* Not more than async read count */
                    if (count > serial->racount)
                    {
                        count = serial->racount;
                    }
                }

                serial->rabuf += count;
                serial->racount -= count;

                /* Async read operation is complete */
                if (serial->racount == 0)
                {
                    /* Stop async read operation */
                    MR_BIT_CLR(serial->state, _SERIAL_STATE_RECEIVE_DMA_ASYNC);
                    MR_BIT_SET(serial->state, _SERIAL_STATE_RECEIVE_DMA_TOP);
                    ops->receive_dma(driver, true, serial->rdma,
                                     sizeof(serial->rdma) / 2);
                    return MR_EOK;
                }

                /* Continue reading */
                int ret = ops->receive_dma(driver, true, serial->rabuf,
                                           serial->racount);
                if (ret < 0)
                {
                    return ret;
                }
                return MR_EBUSY;
            }
#endif /* MR_USE_SERIAL_ASYNC */
            else
            {
                return MR_EINVAL;
            }

            /* Force write data to FIFO */
            mr_fifo_write_force(&serial->rfifo, rdma, count);

            /* Start receiving data from DMA */
            return ops->receive_dma(driver, true, ndma,
                                    sizeof(serial->rdma) / 2);
        }
#endif /* MR_USE_SERIAL_DMA */
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
                       const struct mr_driver *driver)
{
    MR_ASSERT(serial != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL));

    static struct mr_device_ops ops = {.open = serial_open,
                                       .close = serial_close,
                                       .read = serial_read,
#ifdef MR_USE_SERIAL_ASYNC
                                       .read_async = serial_read_async,
#endif /* MR_USE_SERIAL_ASYNC */
                                       .write = serial_write,
                                       .ioctl = serial_ioctl,
                                       .isr = serial_isr};
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;

    /* Initialize the serial */
    serial->config = default_config;
    mr_fifo_init(&serial->rfifo, NULL, 0);
    mr_fifo_init(&serial->wfifo, NULL, 0);
#ifndef MR_CFG_SERIAL_RD_FIFO_SIZE
#define MR_CFG_SERIAL_RD_FIFO_SIZE      (128)
#endif /* MR_CFG_SERIAL_RD_FIFO_SIZE */
#ifndef MR_CFG_SERIAL_WR_FIFO_SIZE
#define MR_CFG_SERIAL_WR_FIFO_SIZE      (0)
#endif /* MR_CFG_SERIAL_WR_FIFO_SIZE */
    serial->rfifo_size = MR_CFG_SERIAL_RD_FIFO_SIZE;
    serial->wfifo_size = MR_CFG_SERIAL_WR_FIFO_SIZE;
    serial->state = 0;

    /* Register the serial */
    return mr_device_register(
        (struct mr_device *)serial, path,
        MR_DEVICE_TYPE_SERIAL | MR_DEVICE_TYPE_FULL_DUPLEX, &ops, driver);
}

#endif /* MR_USE_SERIAL */
