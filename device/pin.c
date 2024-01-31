/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "include/device/mr_pin.h"

#ifdef MR_USING_PIN

void _mr_fast_pin_init(struct mr_dev *dev);

MR_INLINE int pin_set_mode(struct mr_pin *pin, int number, int mode)
{
    struct mr_pin_ops *ops = (struct mr_pin_ops *)pin->dev.drv->ops;

    if (number < 0)
    {
        return MR_EINVAL;
    }

    return ops->configure(pin, number, mode);
}

static ssize_t mr_pin_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size;

#ifdef MR_USING_PIN_CHECK
    /* Check number is valid */
    if (dev->position < 0)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_PIN_CHECK */

    for (rd_size = 0; rd_size < count; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = (uint8_t)ops->read(pin, dev->position);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_pin_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *wr_buf = (uint8_t *)buf;
    ssize_t wr_size;

#ifdef MR_USING_PIN_CHECK
    /* Check number is valid */
    if (dev->position < 0)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_PIN_CHECK */

    for (wr_size = 0; wr_size < count; wr_size += sizeof(*wr_buf))
    {
        ops->write(pin, dev->position, *wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_pin_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)dev;

    switch (cmd)
    {
        case MR_IOC_PIN_SET_MODE:
        {
            if (args != MR_NULL)
            {
                struct mr_pin_config config = *((struct mr_pin_config *)args);

                int ret = pin_set_mode(pin, dev->position, config.mode);
                if (ret < 0)
                {
                    return ret;
                }
                return sizeof(config);
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
        case MR_ISR_PIN_EXTI_INT:
        {
            ssize_t number = *(int *)args;

            return number;
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
 * @param path The path of the pin.
 * @param drv The driver of the pin.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_pin_register(struct mr_pin *pin, const char *path, struct mr_drv *drv)
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

    MR_ASSERT(pin != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Register the pin */
    int ret = mr_dev_register(&pin->dev, path, MR_DEV_TYPE_PIN, MR_O_RDWR, &ops, drv);
    if (ret == MR_EOK)
    {
        /* Initialize the fast pin */
        _mr_fast_pin_init(&pin->dev);
    }
    return ret;
}

#endif /* MR_USING_PIN */
