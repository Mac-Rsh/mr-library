/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 */

#ifndef _DRV_ADC_H_
#define _DRV_ADC_H_

#include "device/adc/adc.h"

#if (MR_CONF_ADC == MR_CONF_ENABLE)

struct ch32_adc_info
{
    ADC_TypeDef *Instance;
    mr_uint32_t adc_periph_clock;
};

struct ch32_adc
{
    char *name;

    struct ch32_adc_info info;
};

mr_err_t ch32_adc_init(void);

#endif /* MR_CONF_ADC */

#endif /* _DRV_ADC_H_ */