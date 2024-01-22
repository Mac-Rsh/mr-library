/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-22    MacRsh       First version
 */

#ifndef _DRV_DAC_H_
#define _DRV_DAC_H_

#include "include/device/mr_dac.h"
#include "mr_board.h"

#ifdef MR_USING_DAC

struct drv_dac_data
{
    DAC_HandleTypeDef handle;
    DAC_TypeDef *instance;
};

struct drv_dac_channel_data
{
    uint32_t channel;
};

#endif /* MR_USING_DAC */

#endif /* _DRV_DAC_H_ */
