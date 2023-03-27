/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-27     MacRsh       first version
 */

#ifndef _ADC_H_
#define _ADC_H_

#include <mrlib.h>

#if (MR_DEVICE_ADC == MR_CONF_ENABLE)

struct mr_adc_config
{
	mr_uint16_t channel;
	mr_state_t state;
};

typedef struct mr_adc *mr_adc_t;
struct mr_adc_ops
{
	mr_err_t (*configure)(mr_adc_t adc, mr_state_t state);
	mr_err_t (*channel_configure)(mr_adc_t adc, mr_uint16_t channel, mr_state_t state);
	mr_uint16_t (*read)(mr_adc_t adc, mr_uint16_t channel);
};

struct mr_adc
{
	struct mr_device device;

	const struct mr_adc_ops *ops;
};

mr_err_t mr_hw_adc_add_to_container(mr_adc_t adc, const char *name, struct mr_adc_ops *ops, void *data);

#endif

#endif
