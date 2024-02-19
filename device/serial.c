/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "include/device/mr_serial.h"

#ifdef MR_USING_SERIAL

MR_INLINE ssize_t serial_poll_read(struct mr_serial *serial, uint8_t *buf, size_t count)
{
    struct mr_serial_ops *ops = (struct mr_serial_ops *)serial->dev.drv->ops;
    ssize_t size;

    for (size = 0; size < count; size += sizeof(*buf)) {
        int ret = ops->read(serial, buf);
        if (ret < 0) {
            return (size == 0) ? ret : size;
        }
        buf++;
    }
    return size;
}

MR_INLINE ssize_t serial_poll_write(struct mr_serial *serial, uint8_t *buf, size_t count)
{
    struct mr_serial_ops *ops = (struct mr_serial_ops *)serial->dev.drv->ops;
    ssize_t size;

    for (size = 0; size < count; size += sizeof(*buf)) {
        int ret = ops->write(serial, *buf);
        if (ret < 0) {
            return (size == 0) ? ret : size;
        }
        buf++;
    }
    return size;
}

#ifdef MR_USING_SERIAL_DMA
MR_INLINE ssize_t serial_dma_write(struct mr_serial *serial, uint8_t *buf, size_t count)
{
    struct mr_serial_ops *ops = (struct mr_serial_ops *)serial->dev.drv->ops;

    if (serial->dma_wr_bufsz == 0) {
        if (serial->nonblock_state == MR_DISABLE) {
            ops->start_dma_tx(serial, buf, count);
            return (ssize_t)count;
        } else {
            return MR_EBUSY;
        }
    } else {
        if (serial->nonblock_state == MR_DISABLE) {
            if (count > serial->dma_wr_bufsz) {
                memcpy(serial->dma_wr_buf, buf, serial->dma_wr_bufsz);
                ops->start_dma_tx(serial, serial->dma_wr_buf, serial->dma_wr_bufsz);
                return (ssize_t)(serial->dma_wr_bufsz +
                                 mr_ringbuf_write(&serial->wr_fifo,
                                                  buf + serial->dma_wr_bufsz,
                                                  count - serial->dma_wr_bufsz));
            } else {
                memcpy(serial->dma_wr_buf, buf, count);
                ops->start_dma_tx(serial, serial->dma_wr_buf, count);
                return (ssize_t)count;
            }
        } else {
            return (ssize_t)mr_ringbuf_write(&serial->wr_fifo, buf, count);
        }
    }
}
#endif /* MR_USING_SERIAL_DMA */

MR_INLINE ssize_t serial_nonblocking_write(struct mr_serial *serial, uint8_t *buf, size_t count)
{
    struct mr_serial_ops *ops = (struct mr_serial_ops *)serial->dev.drv->ops;
    ssize_t size;

#ifdef MR_USING_SERIAL_DMA
    /* DMA sending */
    if ((ops->start_dma_tx != MR_NULL) && (ops->stop_dma_tx != MR_NULL)) {
        return serial_dma_write(serial, buf, count);
    }
#endif /* MR_USING_SERIAL_DMA */

    /* Interrupt sending */
    size = (ssize_t)mr_ringbuf_write(&serial->wr_fifo, buf, count);
    if ((size > 0) && (serial->nonblock_state == MR_DISABLE)) {
        ops->start_tx(serial);
    }
    return size;
}

static int mr_serial_open(struct mr_dev *dev)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    struct mr_serial_ops *ops = (struct mr_serial_ops *)dev->drv->ops;

    int ret = mr_ringbuf_allocate(&serial->rd_fifo, serial->rd_bufsz);
    if (ret < 0) {
        return ret;
    }
    ret = mr_ringbuf_allocate(&serial->wr_fifo, serial->wr_bufsz);
    if (ret < 0) {
        return ret;
    }

#ifdef MR_USING_SERIAL_DMA
    serial->dma_rd_buf = (uint8_t *)mr_malloc(serial->dma_rd_bufsz);
    if ((serial->dma_rd_buf == MR_NULL) && (serial->dma_rd_bufsz != 0)) {
        return MR_ENOMEM;
    }
    serial->dma_wr_buf = (uint8_t *)mr_malloc(serial->dma_wr_bufsz);
    if ((serial->dma_wr_buf == MR_NULL) && (serial->dma_wr_bufsz != 0)) {
        return MR_ENOMEM;
    }

    /* Configure DMA */
    if ((ops->start_dma_rx != MR_NULL) && (serial->dma_rd_bufsz != 0)) {
        ops->start_dma_rx(serial, serial->dma_rd_buf, serial->dma_rd_bufsz);
    }
#endif /* MR_USING_SERIAL_DMA */

    return ops->configure(serial, &serial->config);
}

static int mr_serial_close(struct mr_dev *dev)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    struct mr_serial_ops *ops = (struct mr_serial_ops *)dev->drv->ops;
    struct mr_serial_config close_config = {0};

    mr_ringbuf_free(&serial->rd_fifo);
    mr_ringbuf_free(&serial->wr_fifo);

#ifdef MR_USING_SERIAL_DMA
    mr_free(serial->dma_rd_buf);
    mr_free(serial->dma_wr_buf);
#endif /* MR_USING_SERIAL_DMA */

    return ops->configure(serial, &close_config);
}

static ssize_t mr_serial_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size;

    if (mr_ringbuf_get_bufsz(&serial->rd_fifo) == 0) {
        rd_size = serial_poll_read(serial, rd_buf, count);
    } else {
        rd_size = (ssize_t)mr_ringbuf_read(&serial->rd_fifo, buf, count);
    }
    return rd_size;
}

static ssize_t mr_serial_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    uint8_t *wr_buf = (uint8_t *)buf;
    ssize_t wr_size;

    if (dev->sync == MR_SYNC) {
        wr_size = serial_poll_write(serial, wr_buf, count);
    } else {
        wr_size = serial_nonblocking_write(serial, wr_buf, count);
    }
    return wr_size;
}

static int mr_serial_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    struct mr_serial_ops *ops = (struct mr_serial_ops *)dev->drv->ops;

    switch (cmd) {
        case MR_IOC_SERIAL_SET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_serial_config config = *(struct mr_serial_config *)args;

                int ret = ops->configure(serial, &config);
                if (ret < 0) {
                    return ret;
                }
                serial->config = config;
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_SET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&serial->rd_fifo, bufsz);
                serial->rd_bufsz = 0;
                if (ret < 0) {
                    return ret;
                }
                serial->rd_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_SET_WR_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&serial->wr_fifo, bufsz);
                serial->wr_bufsz = 0;
                if (ret < 0) {
                    return ret;
                }
                serial->wr_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_CLR_RD_BUF: {
            mr_ringbuf_reset(&serial->rd_fifo);
            return MR_EOK;
        }
        case MR_IOC_SERIAL_CLR_WR_BUF: {
            mr_ringbuf_reset(&serial->wr_fifo);
            return MR_EOK;
        }
        case MR_IOC_SERIAL_GET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_serial_config *config = (struct mr_serial_config *)args;

                *config = serial->config;
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = serial->rd_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_WR_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = serial->wr_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_RD_DATASZ: {
            if (args != MR_NULL) {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_data_size(&serial->rd_fifo);
                return sizeof(*datasz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_WR_DATASZ: {
            if (args != MR_NULL) {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_data_size(&serial->wr_fifo);
                return sizeof(*datasz);
            }
            return MR_EINVAL;
        }
#ifdef MR_USING_SERIAL_DMA
        case MR_IOC_SERIAL_SET_RD_DMA_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                if (ops->stop_dma_rx) {
                    return MR_EIO;
                }
                ops->stop_dma_rx(serial);

                uint8_t *pool = mr_realloc(serial->dma_rd_buf, bufsz);
                if ((pool == MR_NULL) && (bufsz != 0)) {
                    return MR_ENOMEM;
                }
                serial->dma_rd_buf = pool;
                serial->dma_rd_bufsz = bufsz;

                if ((ops->start_dma_rx != MR_NULL) && (serial->dma_rd_bufsz != 0)) {
                    ops->start_dma_rx(serial, serial->dma_rd_buf, serial->dma_rd_bufsz);
                }
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_SET_WR_DMA_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                uint8_t *pool = mr_realloc(serial->dma_wr_buf, bufsz);
                if ((pool == MR_NULL) && (bufsz != 0)) {
                    return MR_ENOMEM;
                }
                serial->dma_wr_buf = pool;
                serial->dma_wr_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_RD_DMA_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = serial->dma_rd_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SERIAL_GET_WR_DMA_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = serial->dma_wr_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
#endif /* MR_USING_SERIAL_DMA */
        default: {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_serial_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_serial *serial = (struct mr_serial *)dev;
    struct mr_serial_ops *ops = (struct mr_serial_ops *)dev->drv->ops;

    switch (event) {
        case MR_ISR_SERIAL_RD_INT: {
            uint8_t data;

            /* Read data to FIFO */
            int ret = ops->read(serial, &data);
            if (ret < 0) {
                return ret;
            }
            mr_ringbuf_push_force(&serial->rd_fifo, data);
            return MR_EOK;
        }
        case MR_ISR_SERIAL_WR_INT: {
            uint8_t data;

            /* Write data from FIFO, if FIFO is empty, stop transmit */
            if (mr_ringbuf_pop(&serial->wr_fifo, &data) == sizeof(data)) {
                ops->write(serial, data);
                return MR_EBUSY;
            } else {
                serial->nonblock_state = MR_DISABLE;
                ops->stop_tx(serial);
                return MR_EOK;
            }
        }
#ifdef MR_USING_SERIAL_DMA
        case MR_ISR_SERIAL_RD_DMA: {
            if (args != MR_NULL) {
                size_t dma_rx_datasz = *(size_t *)args;

                mr_ringbuf_write_force(&serial->rd_fifo,
                                       serial->dma_rd_buf,
                                       MR_BOUND(dma_rx_datasz, 0, serial->dma_rd_bufsz));
                if (ops->start_dma_rx != MR_NULL) {
                    ops->start_dma_rx(serial, serial->dma_rd_buf, serial->dma_rd_bufsz);
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_ISR_SERIAL_WR_DMA: {
            if (serial->dma_wr_bufsz == 0) {
                serial->nonblock_state = MR_DISABLE;
                ops->stop_dma_tx(serial);
                return MR_EOK;
            } else {
                size_t size = mr_ringbuf_read(&serial->wr_fifo,
                                              serial->dma_wr_buf,
                                              serial->dma_wr_bufsz);
                if (size != 0) {
                    ops->start_dma_tx(serial, serial->dma_wr_buf, size);
                    return MR_EBUSY;
                } else {
                    serial->nonblock_state = MR_DISABLE;
                    ops->stop_dma_tx(serial);
                    return MR_EOK;
                }
            }
        }
#endif /* MR_USING_SERIAL_DMA */
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function register a serial.
 *
 * @param serial The serial.
 * @param path The path of the serial.
 * @param drv The driver of the serial.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_serial_register(struct mr_serial *serial, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops = {mr_serial_open,
                                    mr_serial_close,
                                    mr_serial_read,
                                    mr_serial_write,
                                    mr_serial_ioctl,
                                    mr_serial_isr};
    struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;

    MR_ASSERT(serial != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    serial->config = default_config;
    mr_ringbuf_init(&serial->rd_fifo, MR_NULL, 0);
    mr_ringbuf_init(&serial->wr_fifo, MR_NULL, 0);
#ifndef MR_CFG_SERIAL_RD_BUFSZ
#define MR_CFG_SERIAL_RD_BUFSZ          (0)
#endif /* MR_CFG_SERIAL_RD_BUFSZ */
#ifndef MR_CFG_SERIAL_WR_BUFSZ
#define MR_CFG_SERIAL_WR_BUFSZ          (0)
#endif /* MR_CFG_SERIAL_WR_BUFSZ */
    serial->rd_bufsz = MR_CFG_SERIAL_RD_BUFSZ;
    serial->wr_bufsz = MR_CFG_SERIAL_WR_BUFSZ;
#ifdef MR_USING_SERIAL_DMA
    serial->dma_rd_buf = MR_NULL;
    serial->dma_wr_buf = MR_NULL;
#ifndef MR_CFG_SERIAL_RD_DMA_BUFSZ
#define MR_CFG_SERIAL_RD_DMA_BUFSZ      (0)
#endif /* MR_CFG_SERIAL_RD_DMA_BUFSZ */
#ifndef MR_CFG_SERIAL_WR_DMA_BUFSZ
#define MR_CFG_SERIAL_WR_DMA_BUFSZ      (0)
#endif /* MR_CFG_SERIAL_WR_DMA_BUFSZ */
    serial->dma_rd_bufsz = MR_CFG_SERIAL_RD_DMA_BUFSZ;
    serial->dma_wr_bufsz = MR_CFG_SERIAL_WR_DMA_BUFSZ;
#endif /* MR_USING_SERIAL_DMA */
    serial->nonblock_state = MR_DISABLE;

    /* Register the serial */
    return mr_dev_register(&serial->dev,
                           path,
                           MR_DEV_TYPE_SERIAL,
                           MR_O_RDWR | MR_O_NONBLOCK,
                           &ops,
                           drv);
}

#endif /* MR_USING_SERIAL */
