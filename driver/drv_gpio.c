/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#include "drv_gpio.h"

#if (MR_CFG_PIN == MR_CFG_ENABLE)

static struct drv_gpio_data drv_gpio_data[] =
    {
        {"pin", /* ... */},
    };

static struct mr_pin pin_device;

static mr_err_t drv_pin_configure(mr_pin_t pin, mr_pin_config_t config)
{
    struct drv_gpio_data *pin_data = (struct drv_gpio_data *)pin->device.data;

    /* ... */

    return MR_ERR_OK;
}

static mr_level_t drv_pin_read(mr_pin_t pin, mr_off_t number)
{
    struct drv_gpio_data *pin_data = (struct drv_gpio_data *)pin->device.data;
    mr_level_t level = MR_LOW;

    /* ... */

    return level;
}

static void drv_pin_write(mr_pin_t pin, mr_off_t number, mr_level_t level)
{
    struct drv_gpio_data *pin_data = (struct drv_gpio_data *)pin->device.data;

    /* ... */
}

mr_err_t drv_gpio_init(void)
{
    static struct mr_pin_ops drv_ops =
        {
            drv_pin_configure,
            drv_pin_read,
            drv_pin_write,
        };
    mr_err_t ret = MR_ERR_OK;

    ret = mr_pin_device_add(&pin_device, drv_gpio_data[0].name, &drv_ops, &drv_gpio_data[0]);
    MR_ASSERT(ret == MR_ERR_OK);

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_gpio_init);

#endif