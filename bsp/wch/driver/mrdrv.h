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

#if (MR_CFG_ADC == MR_CFG_ENABLE)
#include "drv_adc.h"
#endif

#if (MR_CFG_DAC == MR_CFG_ENABLE)
#include "drv_dac.h"
#endif

#if (MR_CFG_I2C == MR_CFG_ENABLE)
#include "drv_i2c.h"
#endif

#if (MR_CFG_PIN == MR_CFG_ENABLE)
#include "drv_gpio.h"
#endif

#if (MR_CFG_PWM == MR_CFG_ENABLE)
#include "drv_pwm.h"
#endif

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)
#include "drv_uart.h"
#endif

#if (MR_CFG_SPI == MR_CFG_ENABLE)
#include "drv_spi.h"
#endif

#if (MR_CFG_TIMER == MR_CFG_ENABLE)
#include "drv_timer.h"
#endif

#endif /* _MR_DRV_H_ */