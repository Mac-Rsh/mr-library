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

struct ch32_hw_adc
{
	ADC_TypeDef *Instance;

	mr_uint32_t adc_periph_clock;
};

struct ch32_adc
{
	char *name;

	struct ch32_hw_adc hw_adc;
};

mr_err_t mr_hw_adc_init(void);

#endif

#endif