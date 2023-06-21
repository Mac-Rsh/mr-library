/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _MR_DRV_H_
#define _MR_DRV_H_

#include "mrconfig.h"

#if (MR_CONF_PIN == MR_CONF_ENABLE)

#include "drv_gpio.h"

#endif /* MR_CONF_PIN */

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)

#include "drv_uart.h"

#endif /* MR_CONF_SERIAL */

#if (MR_CONF_SPI == MR_CONF_ENABLE)

#include "drv_spi.h"

#endif /* MR_CONF_SPI */

#if (MR_CONF_ADC == MR_CONF_ENABLE)

#include "drv_adc.h"

#endif /* MR_CONF_ADC */

#if (MR_CONF_DAC == MR_CONF_ENABLE)

#include "drv_dac.h"

#endif /* MR_CONF_DAC */

#if (MR_CONF_TIMER == MR_CONF_ENABLE)

#include "drv_timer.h"

#endif /* MR_CONF_TIMER */

#if (MR_CONF_I2C == MR_CONF_ENABLE)

#include "drv_i2c.h"

#endif /* MR_CONF_I2C */

#endif /* _MR_DRV_H_ */