/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-19     MacRsh       first version
 */

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#include "device/i2c.h"
#include "mrboard.h"

#if (MR_CFG_I2C == MR_CFG_ENABLE)

struct ch32_soft_i2c_bus_data
{
    const char *name;

    mr_uint32_t gpio_periph_clock;
    GPIO_TypeDef *gpio_port;
    mr_uint16_t scl_gpio_pin;
    mr_uint16_t sda_gpio_pin;
};

#endif

#endif /* _DRV_I2C_H_ */