/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#ifndef _ADC_H_
#define _ADC_H_

#include "stdint.h"

#define ADC_ASSERT(x)

struct adc_config
{
	uint16_t channel;
	uint8_t state;
};

typedef struct adc *adc_t;
struct adc_ops
{
	int (*configure)(adc_t adc, uint8_t state);
	int (*channel_configure)(adc_t adc, struct adc_config *config);
	uint32_t (*read)(adc_t adc, uint16_t channel);
};

struct adc
{
	void *data;

	struct adc_ops *ops;
};

#define ADC_ERR_OK						0
#define ADC_ERR_IO						1

int adc_init(adc_t adc, struct adc_ops *ops, void *data);
int adc_configure(adc_t adc, struct adc_config *config);
int adc_mode(adc_t adc, uint16_t channel, uint8_t state);
uint32_t adc_read(adc_t adc, uint16_t channel);

#endif
