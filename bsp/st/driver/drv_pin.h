/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _DRV_PIN_H_
#define _DRV_PIN_H_

#include "include/device/pin.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_PIN

struct drv_pin_port_data
{
    GPIO_TypeDef *port;
};

struct drv_pin_data
{
    uint32_t pin;
};

#endif /* MR_USING_PIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_PIN_H_ */
