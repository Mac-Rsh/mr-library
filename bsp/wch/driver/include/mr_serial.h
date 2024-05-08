/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _MR_SERIAL_DRIVER_H_
#define _MR_SERIAL_DRIVER_H_

#include "../mr-library/include/device/mr_serial.h"
#include "../mr-library/driver/include/mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_SERIAL

struct mr_serial_driver
{
    struct mr_driver driver;
    USART_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *rx_port;
    uint32_t rx_pin;
    GPIO_TypeDef *tx_port;
    uint32_t tx_pin;
    IRQn_Type irq;
    uint32_t remap;

    DMA_Channel_TypeDef *rx_dma_channel;
    uint32_t rx_dma_clock;
    IRQn_Type rx_dma_irq;
    uint32_t rx_dma_irq_it;
    size_t rx_dma_count;
};

#endif /* MR_USE_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERIAL_DRIVER_H_ */
