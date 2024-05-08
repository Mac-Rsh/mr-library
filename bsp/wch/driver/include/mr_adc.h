/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#ifndef _MR_ADC_DRIVER_H_
#define _MR_ADC_DRIVER_H_

#include "../mr-library/include/device/mr_adc.h"
#include "../mr-library/driver/include/mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_ADC

struct mr_adc_driver
{
    struct mr_driver driver;
    ADC_TypeDef *instance;
    uint32_t clock;
};

struct mr_adc_driver_channel
{
    uint32_t channel;
    uint32_t gpio_clock;
    GPIO_TypeDef *port;
    uint32_t pin;
};

#endif /* MR_USE_ADC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_ADC_DRIVER_H_ */
