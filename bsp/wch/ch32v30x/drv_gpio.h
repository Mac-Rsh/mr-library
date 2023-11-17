/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include "include/device/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_GPIO

struct drv_gpio_data
{
    GPIO_TypeDef *port;
};

struct drv_gpio_pin_data
{
    uint32_t pin;
};

#endif /* MR_USING_GPIO */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_GPIO_H_ */
