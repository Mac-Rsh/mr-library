/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "device/serial.h"
#include "mrboard.h"

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

/**
 * @struct Driver uart data
 */
struct drv_uart_data
{
    const char *name;

    USART_TypeDef *instance;
    mr_uint32_t uart_periph_clock;
    mr_uint32_t gpio_periph_clock;
    GPIO_TypeDef *tx_gpio_port;
    mr_uint16_t tx_gpio_pin;
    GPIO_TypeDef *rx_gpio_port;
    mr_uint16_t rx_gpio_pin;
    IRQn_Type irqno;
};

#endif

#endif /* _DRV_UART_H_ */
