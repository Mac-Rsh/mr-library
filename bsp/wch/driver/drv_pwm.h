/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-12     MacRsh       first version
 */

#ifndef _DRV_PWM_H
#define _DRV_PWM_H

#include "device/pwm/pwm.h"

#if (MR_CFG_PWM == MR_CFG_ENABLE)

struct ch32_pwm_data
{
    const char *name;

    TIM_TypeDef *Instance;
    mr_uint32_t tim_periph_clock;
    mr_uint32_t gpio_periph_clock;
    GPIO_TypeDef *gpio_port;
    mr_uint16_t channel1_gpio_pin;
    mr_uint16_t channel2_gpio_pin;
    mr_uint16_t channel3_gpio_pin;
    mr_uint16_t channel4_gpio_pin;
};

#endif

#endif /* _DRV_PWM_H */
