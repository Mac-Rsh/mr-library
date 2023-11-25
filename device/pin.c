/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "include/device/pin.h"

#ifdef MR_USING_PIN

static ssize_t mr_pin_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size = 0;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = (uint8_t)ops->read(pin, off);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_pin_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *wr_buf = (uint8_t *)buf;
    ssize_t wr_size = 0;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        ops->write(pin, off, (int)*wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_pin_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTRL_PIN_SET_MODE:
        {
            if (args != MR_NULL)
            {
                int mode = *((int *)args);

                /* Check offset is valid */
                if (off < 0)
                {
                    return MR_EINVAL;
                }

                return ops->configure(pin, off, mode);
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_pin_isr(struct mr_dev *dev, int event, void *args)
{
    switch (event)
    {
        case MR_ISR_PIN_RD_INT:
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
 * @brief This function registers a pin.
 *
 * @param pin The pin.
 * @param name The name of the pin.
 * @param drv The driver of the pin.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_pin_register(struct mr_pin *pin, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            MR_NULL,
            MR_NULL,
            mr_pin_read,
            mr_pin_write,
            mr_pin_ioctl,
            mr_pin_isr
        };

    mr_assert(pin != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Register the pin */
    return mr_dev_register(&pin->dev, name, Mr_Dev_Type_Pin, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_PIN */
