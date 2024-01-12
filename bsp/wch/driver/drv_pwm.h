/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-01    MacRsh       First version
 */

#ifndef _DRV_PWM_H_
#define _DRV_PWM_H_

#include "include/device/mr_pwm.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_PWM

struct drv_pwm_data
{
    TIM_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *ch1_port;
    uint32_t ch1_pin;
    GPIO_TypeDef *ch2_port;
    uint32_t ch2_pin;
    GPIO_TypeDef *ch3_port;
    uint32_t ch3_pin;
    GPIO_TypeDef *ch4_port;
    uint32_t ch4_pin;
    uint32_t remap;
};

#endif /* MR_USING_PWM */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_PWM_H_ */
