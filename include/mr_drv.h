/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _MR_DRV_H_
#define _MR_DRV_H_

#include "include/mr_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ADC
#include "driver/drv_adc.h"
#endif

#ifdef MR_USING_CAN
#include "driver/drv_can.h"
#endif

#ifdef MR_USING_DAC
#include "driver/drv_dac.h"
#endif

#ifdef MR_USING_I2C
#include "driver/drv_i2c.h"
#endif

#ifdef MR_USING_PIN
#include "driver/drv_pin.h"
#endif

#ifdef MR_USING_SERIAL
#include "driver/drv_serial.h"
#endif

#ifdef MR_USING_SPI
#include "driver/drv_spi.h"
#endif

#ifdef MR_USING_TIMER
#include "driver/drv_timer.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DRV_H_ */
