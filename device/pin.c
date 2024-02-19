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

#define PIN_MODE_SET(_pin, _number, _mode)                                              \
    do                                                                                  \
    {                                                                                   \
        MR_BIT_CLR((_pin)->pins[(_number) / 8], (0xf << (((_number) % 8) * 4)));        \
        MR_BIT_SET((_pin)->pins[(_number) / 8], ((_mode) << (((_number) % 8) * 4)));    \
    } while (0)
#define PIN_MODE_GET(_pin, _number)                                                     \
        ((int)(((_pin)->pins[(_number) / 8] >> (((_number) % 8) * 4)) & 0xf))           \

MR_INLINE int pin_set_mode(struct mr_pin *pin, int number, struct mr_pin_config config)
{
    struct mr_pin_ops *ops = (struct mr_pin_ops *)pin->dev.drv->ops;

    if ((number < 0) || (number >= (sizeof(pin->pins) * 2))) {
        return MR_EINVAL;
    }

    int ret = ops->configure(pin, number, config.mode);
    if (ret < 0) {
        return ret;
    }

    PIN_MODE_SET(pin, number, config.mode);
    return MR_EOK;
}

MR_INLINE int pin_get_mode(struct mr_pin *pin, int number, struct mr_pin_config *config)
{
    if ((number < 0) || (number >= (sizeof(pin->pins) * 2))) {
        return MR_EINVAL;
    }

    config->mode = PIN_MODE_GET(pin, number);
    return MR_EOK;
}

static int mr_pin_close(struct mr_dev *dev)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;

#ifdef MR_USING_PIN_AUTO_DISABLE
    /* Disable all pins */
    for (size_t i = 0; i < (sizeof(pin->pins) * 2); i++)
    {
        if (PIN_MODE_GET(pin, i) != MR_PIN_MODE_NONE)
        {
            ops->configure(pin, (int)i, MR_PIN_MODE_NONE);
            PIN_MODE_SET(pin, i, MR_PIN_MODE_NONE);
        }
    }
#endif /* MR_USING_PIN_AUTO_DISABLE */
    return MR_EOK;
}

static ssize_t mr_pin_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size;

#ifdef MR_USING_PIN_CHECK
    /* Check number is valid */
    if ((dev->position < 0) ||
        (dev->position >= (sizeof(pin->pins) * 2)) ||
        (PIN_MODE_GET(pin, dev->position) == MR_PIN_MODE_NONE)) {
        return MR_EINVAL;
    }
#endif /* MR_USING_PIN_CHECK */

    for (rd_size = 0; rd_size < count; rd_size += sizeof(*rd_buf)) {
        int ret = (uint8_t)ops->read(pin, dev->position, rd_buf);
        if (ret < 0) {
            return (rd_size == 0) ? ret : rd_size;
        }
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
    if ((dev->position < 0) ||
        (dev->position >= (sizeof(pin->pins) * 2)) ||
        (PIN_MODE_GET(pin, dev->position) == MR_PIN_MODE_NONE)) {
        return MR_EINVAL;
    }
#endif /* MR_USING_PIN_CHECK */

    for (wr_size = 0; wr_size < count; wr_size += sizeof(*wr_buf)) {
        int ret = ops->write(pin, dev->position, *wr_buf);
        if (ret < 0) {
            return (wr_size == 0) ? ret : wr_size;
        }
        wr_buf++;
    }
    return wr_size;
}

static int mr_pin_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)dev;

    switch (cmd) {
        case MR_IOC_PIN_SET_MODE: {
            if (args != MR_NULL) {
                struct mr_pin_config config = *((struct mr_pin_config *)args);

                int ret = pin_set_mode(pin, dev->position, config);
                if (ret < 0) {
                    return ret;
                }
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_PIN_GET_MODE: {
            if (args != MR_NULL) {
                struct mr_pin_config *config = (struct mr_pin_config *)args;

                int ret = pin_get_mode(pin, dev->position, config);
                if (ret < 0) {
                    return ret;
                }
                return sizeof(*config);
            }
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_pin_isr(struct mr_dev *dev, int event, void *args)
{
    switch (event) {
        case MR_ISR_PIN_EXTI_INT: {
            ssize_t number = *(int *)args;
            return number;
        }
        default: {
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
    static struct mr_dev_ops ops = {MR_NULL,
                                    mr_pin_close,
                                    mr_pin_read,
                                    mr_pin_write,
                                    mr_pin_ioctl,
                                    mr_pin_isr};

    MR_ASSERT(pin != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Register the pin */
    int ret = mr_dev_register(&pin->dev, path, MR_DEV_TYPE_PIN, MR_O_RDWR, &ops, drv);
    if (ret == MR_EOK) {
        /* Initialize the fast pin */
        _mr_fast_pin_init(&pin->dev);
    }
    return ret;
}

#endif /* MR_USING_PIN */
