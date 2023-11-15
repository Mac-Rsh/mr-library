/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "gpio.h"

#ifdef MR_USING_GPIO

static ssize_t mr_gpio_read(struct mr_dev *dev, int off, void *buf, size_t size, int sync_or_async)
{
    struct mr_gpio *gpio = (struct mr_gpio *)dev;
    struct mr_gpio_ops *ops = (struct mr_gpio_ops *)dev->drv->ops;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size = 0;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = (uint8_t)ops->read(gpio, off);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_gpio_write(struct mr_dev *dev, int off, const void *buf, size_t size, int sync_or_async)
{
    struct mr_gpio *gpio = (struct mr_gpio *)dev;
    struct mr_gpio_ops *ops = (struct mr_gpio_ops *)dev->drv->ops;
    uint8_t *wr_buf = (uint8_t *)buf;
    ssize_t wr_size = 0;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        ops->write(gpio, off, (int)*wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_gpio_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_gpio *gpio = (struct mr_gpio *)dev;
    struct mr_gpio_ops *ops = (struct mr_gpio_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_gpio_config *config = (struct mr_gpio_config *)args;

                /* Check offset is valid */
                if (off < 0)
                {
                    return MR_EINVAL;
                }

                return ops->configure(gpio, off, config->mode);
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_EINVAL;
        }
    }
}

static ssize_t mr_gpio_isr(struct mr_dev *dev, int event, void *args)
{
    switch (event)
    {
        case MR_ISR_EVENT_RD_INTER:
        {
            return (ssize_t)*(int *)args;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a gpio.
 *
 * @param gpio The gpio.
 * @param name The name of the gpio.
 * @param drv The driver of the gpio.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_gpio_register(struct mr_gpio *gpio, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            MR_NULL,
            MR_NULL,
            mr_gpio_read,
            mr_gpio_write,
            mr_gpio_ioctl,
            mr_gpio_isr
        };

    mr_assert(gpio != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Register the gpio */
    return mr_dev_register(&gpio->dev, name, Mr_Dev_Type_Gpio, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_GPIO */
