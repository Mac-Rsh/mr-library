/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _DRV_ADC_H_
#define _DRV_ADC_H_

#include "include/device/mr_adc.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ADC

struct drv_adc_data
{
    ADC_TypeDef *instance;
    uint32_t clock;
};

struct drv_adc_channel_data
{
    uint32_t channel;
    uint32_t gpio_clock;
    GPIO_TypeDef *port;
    uint32_t pin;
};

#endif /* MR_USING_ADC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_ADC_H_ */
