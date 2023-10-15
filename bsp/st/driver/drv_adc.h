/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#ifndef _DRV_ADC_H_
#define _DRV_ADC_H_

#include "device/adc.h"
#include "mrboard.h"

#if (MR_CFG_ADC == MR_CFG_ENABLE)

/**
 * @struct Driver adc data
 */
struct drv_adc_data
{
    const char *name;

    ADC_HandleTypeDef handle;
    ADC_TypeDef *instance;
};

#endif

#endif /* _DRV_ADC_H_ */
