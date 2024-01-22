/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-21    MacRsh       First version
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
    TIM_HandleTypeDef handle;
    TIM_TypeDef *instance;
};

#endif /* MR_USING_PWM */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_PWM_H_ */
