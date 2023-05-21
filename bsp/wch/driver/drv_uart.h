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

#include "device/serial/serial.h"

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)

#define CH32_UART_GPIO_REMAP_NONE		0

struct ch32_hw_uart
{
	USART_TypeDef *Instance;

	mr_uint32_t uart_periph_clock;
	mr_uint32_t gpio_periph_clock;
	GPIO_TypeDef *tx_gpio_port;
	mr_uint16_t tx_gpio_pin;
	GPIO_TypeDef *rx_gpio_port;
	mr_uint16_t rx_gpio_pin;
	mr_uint32_t remap;

	IRQn_Type irqno;
};

struct ch32_uart
{
	char *name;

	struct ch32_hw_uart hw_uart;
};

mr_err_t mr_hw_uart_init(void);

#endif

#endif