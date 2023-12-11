/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _DRV_SERIAL_H_
#define _DRV_SERIAL_H_

#include "include/device/serial.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SERIAL

struct drv_serial_data
{
    USART_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *rx_port;
    uint32_t rx_pin;
    GPIO_TypeDef *tx_port;
    uint32_t tx_pin;
    IRQn_Type irq;
    uint32_t remap;
};

#endif /* MR_USING_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_SERIAL_H_ */
