/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-16     MacRsh       first version
 */

#ifndef _DRV_SPI_H_
#define _DRV_SPI_H_

#include "device/spi/spi.h"

#if (MR_CFG_SPI == MR_CFG_ENABLE)

/**
 * @struct ch32 SPI bus data
 */
struct ch32_spi_bus_data
{
    const char *name;

    SPI_TypeDef *instance;
    mr_uint32_t spi_periph_clock;
    mr_uint32_t gpio_periph_clock;
    GPIO_TypeDef *gpio_port;
    mr_uint16_t clk_gpio_pin;
    mr_uint16_t miso_gpio_pin;
    mr_uint16_t mosi_gpio_pin;
    IRQn_Type irqno;
};

#endif

#endif /* _DRV_SPI_H_ */