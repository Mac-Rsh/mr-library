/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-14    MacRsh       First version
 */

#ifndef _DRV_DAC_H_
#define _DRV_DAC_H_

#include "include/device/mr_dac.h"
#include "mr_board.h"

#ifdef MR_USING_DAC

struct drv_dac_data
{
    uint32_t clock;
};

struct drv_dac_channel_data
{
    uint32_t channel;
    uint32_t gpio_clock;
    GPIO_TypeDef *port;
    uint32_t pin;
};

#endif /* MR_USING_DAC */

#endif /* _DRV_DAC_H_ */
