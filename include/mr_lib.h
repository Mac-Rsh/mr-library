/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_LIB_H_
#define _MR_LIB_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ADC
#include "device/adc.h"
#endif

#ifdef MR_USING_CAN
#include "device/can.h"
#endif

#ifdef MR_USING_DAC
#include "device/dac.h"
#endif

#ifdef MR_USING_I2C
#include "device/i2c.h"
#ifdef MR_USING_SOFT_I2C
#include "device/soft_i2c.h"
#endif
#endif

#ifdef MR_USING_PIN
#include "device/pin.h"
#endif

#ifdef MR_USING_SERIAL
#include "device/serial.h"
#endif

#ifdef MR_USING_SPI
#include "device/spi.h"
#endif

#ifdef MR_USING_TIMER
#include "device/timer.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_LIB_H_ */
