/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-13    MacRsh       First version
 */

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#include "device/i2c.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_I2C

struct drv_i2c_bus_data
{
    I2C_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *scl_port;
    uint32_t sda_pin;
    GPIO_TypeDef *sda_port;
    uint32_t scl_pin;
    IRQn_Type irq;
    uint32_t remap;
};

#endif /* MR_USING_I2C */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_I2C_H_ */
