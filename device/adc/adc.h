/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _ADC_H_
#define _ADC_H_

#include "mrlib.h"

#if (MR_CONF_ADC == MR_CONF_ENABLE)

#define MR_ADC_STATE_DISABLE            0
#define MR_ADC_STATE_ENABLE             1

struct mr_adc_config
{
	mr_uint16_t channel;
	mr_uint8_t state;
};

typedef struct mr_adc *mr_adc_t;
struct mr_adc_ops
{
	mr_err_t (*configure)(mr_adc_t adc, mr_uint8_t state);
	mr_err_t (*channel_configure)(mr_adc_t adc, struct mr_adc_config *config);
	mr_uint32_t (*read)(mr_adc_t adc, mr_uint16_t channel);
};

struct mr_adc
{
	struct mr_device device;

	const struct mr_adc_ops *ops;
};

mr_err_t mr_hw_adc_add(mr_adc_t adc, const char *name, struct mr_adc_ops *ops, void *data);

#endif

#endif