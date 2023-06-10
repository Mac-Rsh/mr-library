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

#if (MR_CONF_SPI == MR_CONF_ENABLE)

struct ch32_spi_info
{
    SPI_TypeDef *Instance;
    mr_uint32_t spi_periph_clock;
    mr_uint32_t gpio_periph_clock;
    GPIO_TypeDef *gpio_port;
    mr_uint16_t clk_gpio_pin;
    mr_uint16_t miso_gpio_pin;
    mr_uint16_t mosi_gpio_pin;
};

struct ch32_spi
{
    char *name;

    struct ch32_spi_info info;
};

mr_err_t ch32_spi_init(void);

#endif

#endif