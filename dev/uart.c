/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "uart.h"

#ifdef MR_USING_UART

static int mr_uart_open(struct mr_dev *dev)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;

    /* Allocate FIFO buffers */
    int ret = mr_ringbuf_allocate(&uart->rd_fifo, uart->rd_bufsz);
    if (ret != MR_EOK)
    {
        return ret;
    }
    ret = mr_ringbuf_allocate(&uart->wr_fifo, uart->wr_bufsz);
    if (ret != MR_EOK)
    {
        return ret;
    }

    return ops->configure(uart, &uart->config);
}

static int mr_uart_close(struct mr_dev *dev)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;
    struct mr_uart_config close_config = {0};

    /* Free FIFO buffers */
    mr_ringbuf_free(&uart->rd_fifo);
    mr_ringbuf_free(&uart->wr_fifo);

    return ops->configure(uart, &close_config);
}

static ssize_t mr_uart_read(struct mr_dev *dev, int off, void *buf, size_t size, int sync_or_async)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;

    if (mr_ringbuf_get_bufsz(&uart->rd_fifo) == 0)
    {
        return (ssize_t)ops->read(uart, buf, size);
    } else
    {
        return (ssize_t)mr_ringbuf_read(&uart->rd_fifo, buf, size);
    }
}

static ssize_t mr_uart_write(struct mr_dev *dev, int off, const void *buf, size_t size, int sync_or_async)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;

    if (sync_or_async == MR_SYNC)
    {
        return (ssize_t)ops->write(uart, buf, size);
    } else
    {
        if (mr_ringbuf_get_bufsz(&uart->wr_fifo) == 0)
        {
            return (ssize_t)ops->write(uart, buf, size);
        } else
        {
            ssize_t ret = (ssize_t)mr_ringbuf_write(&uart->wr_fifo, buf, size);

            /* Start interrupt sending */
            ops->start_tx(uart);
            return ret;
        }
    }
}

static int mr_uart_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_uart_config config = *(struct mr_uart_config *)args;

                int ret = ops->configure(uart, &config);
                if (ret == MR_EOK)
                {
                    uart->config = config;
                }
                return ret;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_SET_RD_BUFSZ:
        {
            if (args != MR_NULL)
            {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&uart->rd_fifo, bufsz);
                uart->rd_bufsz = 0;
                if (ret == MR_EOK)
                {
                    uart->rd_bufsz = bufsz;
                }
                return ret;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_SET_WR_BUFSZ:
        {
            if (args != MR_NULL)
            {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&uart->wr_fifo, bufsz);
                uart->wr_bufsz = 0;
                if (ret == MR_EOK)
                {
                    uart->wr_bufsz = bufsz;
                }
                return ret;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_uart_config *config = (struct mr_uart_config *)args;

                *config = uart->config;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_GET_RD_BUFSZ:
        {
            if (args != MR_NULL)
            {
                *(size_t *)args = uart->rd_bufsz;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_GET_WR_BUFSZ:
        {
            if (args != MR_NULL)
            {
                *(size_t *)args = uart->wr_bufsz;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_uart_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_uart *uart = (struct mr_uart *)dev;
    struct mr_uart_ops *ops = (struct mr_uart_ops *)dev->drv->ops;

    switch (event)
    {
        case MR_ISR_EVENT_RD_INTER:
        {
            uint8_t data = 0;

            ops->read(uart, &data, sizeof(data));
            mr_ringbuf_push_force(&uart->rd_fifo, data);
            return (ssize_t)mr_ringbuf_get_data_size(&uart->rd_fifo);
        }
        case MR_ISR_EVENT_WR_INTER:
        {
            uint8_t data = 0;

            if (mr_ringbuf_pop(&uart->wr_fifo, &data) == sizeof(data))
            {
                ops->write(uart, &data, sizeof(data));
            } else
            {
                ops->stop_tx(uart);
            }
            return (ssize_t)mr_ringbuf_get_data_size(&uart->wr_fifo);
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function register a uart.
 *
 * @param uart The uart.
 * @param name The name of the uart.
 * @param drv The driver of the uart.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_uart_register(struct mr_uart *uart, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            mr_uart_open,
            mr_uart_close,
            mr_uart_read,
            mr_uart_write,
            mr_uart_ioctl,
            mr_uart_isr
        };
    struct mr_uart_config default_config = MR_UART_CONFIG_DEFAULT;

    mr_assert(uart != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Initialize the fields */
    uart->config = default_config;
    mr_ringbuf_init(&uart->rd_fifo, MR_NULL, 0);
    mr_ringbuf_init(&uart->wr_fifo, MR_NULL, 0);
#ifndef MR_CFG_UART_RD_BUFSZ_INIT
#define MR_CFG_UART_RD_BUFSZ_INIT       (0)
#endif /* MR_CFG_UART_RD_BUFSZ_INIT */
#ifndef MR_CFG_UART_WR_BUFSZ_INIT
#define MR_CFG_UART_WR_BUFSZ_INIT       (0)
#endif /* MR_CFG_UART_WR_BUFSZ_INIT */
    uart->rd_bufsz = MR_CFG_UART_RD_BUFSZ_INIT;
    uart->wr_bufsz = MR_CFG_UART_WR_BUFSZ_INIT;

    /* Register the uart */
    return mr_dev_register(&uart->dev, name, Mr_Dev_Type_Uart, MR_SFLAG_RDWR | MR_SFLAG_NONBLOCK, &ops, drv);
}

#endif /* MR_USING_UART */
