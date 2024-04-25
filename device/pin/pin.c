/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "../mr-library/device/include/mr_pin.h"

#ifdef MR_USE_PIN

#define _PIN_IS_VALID(_pin, _number)                                           \
    (((_number) >= 0) && ((_number) < (sizeof(_pin->pins) * 4)))
#define _PIN_IS_RDONLY(_data, _number)                                         \
    (((_data)->pins[(_number) / 16] & (0x1 << (_number))) != 0)
#define _PIN_IS_WRONLY(_data, _number)                                         \
    (((_data)->pins[(_number) / 16] & (0x2 << (_number))) != 0)
#define _PIN_IS_EXISTED(_data, _number)                                        \
    (((_data)->pins[(_number) / 16] & (0x3 << (_number))) != 0)
#define _PIN_MODE_SET(_pin, _number, _mode)                                    \
    do                                                                         \
    {                                                                          \
        MR_BIT_CLR((_pin)->pins[(_number) / 16],                               \
                   (0x3 << (((_number) % 16) * 2)));                           \
        MR_BIT_SET((_pin)->pins[(_number) / 16],                               \
                   ((_mode) << (((_number) % 16) * 2)));                       \
    } while (0)
#define _PIN_MODE_GET(_pin, _number)                                           \
    ((uint32_t)((_pin)->pins[(_number) / 16] & (0x3 << (((_number) % 16) * 2))))
#define _PIN_IS_ENABLED(_pin, _number)                                         \
    (_PIN_MODE_GET(_pin, _number) != MR_PIN_MODE_NONE)

MR_INLINE int _pin_configure_set(struct mr_pin *pin, int number,
                                 struct mr_pin_config *config)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET((struct mr_device *)pin);
    struct mr_pin_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    struct mr_pin_driver_data *data = _MR_DRIVER_DATA_GET(driver);

    /* Check if the pin is valid */
    if (_PIN_IS_VALID(pin, number) == false)
    {
        return MR_EINVAL;
    }

    /* Check if the pin is existed */
    if ((data != NULL) && (_PIN_IS_EXISTED(data, number) == false))
    {
        return MR_EINVAL;
    }

    /* Check if the pin is already configured */
    if (config->mode == _PIN_MODE_GET(pin, number))
    {
        return MR_EOK;
    }

    /* Configure the pin */
    int ret = ops->configure(driver, (uint32_t)number, config->mode);
    if (ret < 0)
    {
        return ret;
    }

    /* Set the pin mode */
    _PIN_MODE_SET(pin, number, config->mode);
    return MR_EOK;
}

MR_INLINE int _pin_configure_get(struct mr_pin *pin, int number,
                                 struct mr_pin_config *config)
{
    /* Check if the pin is valid */
    if (_PIN_IS_VALID(pin, number) == false)
    {
        return MR_EINVAL;
    }

    /* Get the pin mode */
    config->mode = _PIN_MODE_GET(pin, number);
    return MR_EOK;
}

static int pin_open(struct mr_device *device)
{
    struct mr_pin *pin = (struct mr_pin *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pin_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Recover all enabled pins */
    for (uint32_t i = 0; i < (sizeof(pin->pins) * 2); i++)
    {
        uint32_t mode = _PIN_MODE_GET(pin, i);

        if (mode != MR_PIN_MODE_NONE)
        {
            ops->configure(driver, i, mode);
        }
    }
    return MR_EOK;
}

static int pin_close(struct mr_device *device)
{
    struct mr_pin *pin = (struct mr_pin *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pin_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable all pins */
    for (uint32_t i = 0; i < (sizeof(pin->pins) * 2); i++)
    {
        uint32_t mode = _PIN_MODE_GET(pin, i);

        if (mode != MR_PIN_MODE_NONE)
        {
            /* Just close the pin, without clearing the mode mask, and
             * the pin mode will be restored when opened */
            ops->configure(driver, i, MR_PIN_MODE_NONE);
        }
    }
    return MR_EOK;
}

static ssize_t pin_read(struct mr_device *device, int pos, void *buf,
                        size_t count)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pin_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint8_t *rbuf = (uint8_t *)buf;
    ssize_t rcount;

#ifdef MR_USE_PIN_CHECK
    struct mr_pin *pin = (struct mr_pin *)device;

    /* Check if the pin is enabled */
    if ((_PIN_IS_VALID(pin, pos) == false) ||
        (_PIN_IS_ENABLED(pin, pos) == false))
    {
        return MR_EINVAL;
    }
#endif /* MR_USE_PIN_CHECK */

    /* Read data */
    for (rcount = 0; rcount < count; rcount += sizeof(*rbuf))
    {
        int ret = ops->read(driver, (uint32_t)pos, rbuf);
        if (ret < 0)
        {
            /* If no data is read, return the error code */
            return (rcount == 0) ? ret : rcount;
        }
        rbuf++;
    }

    /* Return the number of bytes read */
    return rcount;
}

static ssize_t pin_write(struct mr_device *device, int pos, const void *buf,
                         size_t count)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pin_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    const uint8_t *wbuf = (const uint8_t *)buf;
    ssize_t wcount;

#ifdef MR_USE_PIN_CHECK
    struct mr_pin *pin = (struct mr_pin *)device;

    /* Check if the pin is enabled */
    if ((_PIN_IS_VALID(pin, pos) == false) ||
        (_PIN_IS_ENABLED(pin, pos) == false))
    {
        return MR_EINVAL;
    }
#endif /* MR_USE_PIN_CHECK */

    /* Write data */
    for (wcount = 0; wcount < count; wcount += sizeof(*wbuf))
    {
        int ret = ops->write(driver, pos, *wbuf);
        if (ret < 0)
        {
            /* If no data is written, return the error code */
            return (wcount == 0) ? ret : wcount;
        }
        wbuf++;
    }

    /* Return the number of bytes written */
    return wcount;
}

static int pin_ioctl(struct mr_device *device, int pos, int cmd, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)device;

    switch (cmd)
    {
        case MR_CTRL_SET(MR_CMD_PIN_MODE):
        {
            struct mr_pin_config *config = (struct mr_pin_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Set the pin configure */
            int ret = _pin_configure_set(pin, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        case MR_CTRL_GET(MR_CMD_PIN_MODE):
        {
            struct mr_pin_config *config = (struct mr_pin_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get the pin configure */
            int ret = _pin_configure_get(pin, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

static int pin_isr(struct mr_device *device, uint32_t event, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)device;

    switch (event)
    {
        case MR_EVENT_PIN_EXTI_INT:
        {
            int *number = (int *)args;

            /* Check if the pin is enabled */
            if ((_PIN_IS_VALID(pin, *number) == false) ||
                _PIN_IS_ENABLED(pin, *number) == false)
            {
                /* This EXTI will be ignored */
                return MR_EINVAL;
            }
            return MR_EOK;
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function registers a pin.
 *
 * @param pin The pin.
 * @param path The path of the pin.
 * @param driver The driver of the pin.
 *
 * @return The error code.
 */
int mr_pin_register(struct mr_pin *pin, const char *path,
                    struct mr_driver *driver)
{
    static struct mr_device_ops ops = {.open = pin_open,
                                       .close = pin_close,
                                       .read = pin_read,
                                       .write = pin_write,
                                       .ioctl = pin_ioctl,
                                       .isr = pin_isr};

    MR_ASSERT(pin != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL));

    /* Register the pin device */
    return mr_device_register((struct mr_device *)pin, path,
                              MR_DEVICE_TYPE_PIN | MR_DEVICE_TYPE_FULL_DUPLEX,
                              &ops, driver);
}

#endif /* MR_USE_PIN */
