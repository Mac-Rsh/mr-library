/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_UART

struct drv_uart_data
{
    USART_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *rx_port;
    uint16_t rx_pin;
    GPIO_TypeDef *tx_port;
    uint16_t tx_pin;
    IRQn_Type irq;
    uint32_t remap;
};

#endif /* MR_USING_UART */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_UART_H_ */
