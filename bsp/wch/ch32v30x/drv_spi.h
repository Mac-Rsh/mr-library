/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _DRV_SPI_H_
#define _DRV_SPI_H_

#include "spi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SPI

struct drv_spi_bus_data
{
    SPI_TypeDef *instance;
    uint32_t clock;
    uint32_t gpio_clock;
    GPIO_TypeDef *sck_port;
    uint32_t sck_pin;
    GPIO_TypeDef *miso_port;
    uint32_t miso_pin;
    GPIO_TypeDef *mosi_port;
    uint32_t mosi_pin;
    IRQn_Type irq;
    uint32_t remap;
};

#endif /* MR_USING_SPI */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _DRV_SPI_H_ */
