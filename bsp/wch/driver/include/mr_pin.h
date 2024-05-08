/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _MR_PIN_DRIVER_H_
#define _MR_PIN_DRIVER_H_

#include "../mr-library/include/device/mr_pin.h"
#include "../mr-library/driver/include/mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_PIN

struct mr_pin_driver_port
{
    GPIO_TypeDef *port;
};

struct mr_pin_driver_pin
{
    uint32_t pin;
};

#endif /* MR_USE_PIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PIN_DRIVER_H_ */
