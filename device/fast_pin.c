/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-11-08    MacRsh       First version
 */

#include "include/device/mr_pin.h"

#ifdef MR_USING_PIN

/**
 * @addtogroup PIN
 * @{
 */

/**
 * @brief This function get the fast pin device pointer.
 *
 * @param magic The magic number.
 *
 * @return The fast pin device pointer.
 *
 * @note Please do not appear the code in this file in the application layer.
 */
MR_INLINE struct mr_dev **_fast_pin_dev_get(uint32_t magic)
{
    static struct mr_dev *dev = MR_NULL;

    /* If that doesn't stop you, feel free to use it */
    if (magic == MR_MAGIC_NUMBER) {
        return &dev;
    }
    return MR_NULL;
}

/**
 * @brief This function initialize the fast pin device.
 *
 * @param dev The pin device.
 *
 * @note Please do not appear the code in this file in the application layer.
 */
void _mr_fast_pin_init(struct mr_dev *dev)
{
    if (_fast_pin_dev_get(dev->magic) != MR_NULL) {
        *_fast_pin_dev_get(dev->magic) = dev;
    }
}

/**
 * @brief This function set the pin mode.
 *
 * @param number The pin number.
 * @param mode The pin mode.
 *
 * @return 0 on success, otherwise an error code.
 *
 * @note Please do not appear the code in this file in the application layer.
 */
int _mr_fast_pin_mode(int number, int mode)
{
    struct mr_dev *dev = *_fast_pin_dev_get(MR_MAGIC_NUMBER);

    if (dev == MR_NULL) {
        return MR_ENOTFOUND;
    }
    return ((struct mr_pin_ops *)dev->drv->ops)->configure((struct mr_pin *)dev, number, mode);
}

/**
 * @brief This function read the pin value.
 *
 * @param number The pin number.
 *
 * @return The pin value.
 *
 * @note Please do not appear the code in this file in the application layer.
 */
uint8_t _mr_fast_pin_read(int number)
{
    struct mr_dev *dev = *_fast_pin_dev_get(MR_MAGIC_NUMBER);
    uint8_t value = 0;

    ((struct mr_pin_ops *)dev->drv->ops)->read((struct mr_pin *)dev, number, &value);
    return value;
}

/**
 * @brief This function write the pin value.
 *
 * @param number The pin number.
 * @param value The pin value.
 *
 * @note Please do not appear the code in this file in the application layer.
 */
void _mr_fast_pin_write(int number, int value)
{
    struct mr_dev *dev = *_fast_pin_dev_get(MR_MAGIC_NUMBER);

    ((struct mr_pin_ops *)dev->drv->ops)->write((struct mr_pin *)dev, number, value);
}
/** @} */

#endif /* MR_USING_PIN */

