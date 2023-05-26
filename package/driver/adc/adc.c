/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#include "adc.h"

static int _err_io_adc_configure(adc_t adc, uint8_t state)
{
	ADC_ASSERT(0);
	return - ADC_ERR_IO;
}

static int _err_io_adc_channel_configure(adc_t adc, struct adc_config *config)
{
	ADC_ASSERT(0);
	return - ADC_ERR_IO;
}

static uint32_t _err_io_adc_read(adc_t adc, uint16_t channel)
{
	ADC_ASSERT(0);
	return 0;
}

int adc_init(adc_t adc, struct adc_ops *ops, void *data)
{
	ADC_ASSERT(adc != NULL);
	ADC_ASSERT(ops != NULL);

	adc->data = data;

	ops->configure = ops->configure ? ops->configure : _err_io_adc_configure;
	ops->channel_configure = ops->channel_configure ? ops->channel_configure : _err_io_adc_channel_configure;
	ops->read = ops->read ? ops->read : _err_io_adc_read;
	adc->ops = ops;

	return adc->ops->configure(adc, 1);
}

int adc_configure(adc_t adc, struct adc_config *config)
{
	ADC_ASSERT(adc != NULL);
	ADC_ASSERT(config != NULL);

	return adc->ops->channel_configure(adc, config);
}

int adc_mode(adc_t adc, uint16_t channel, uint8_t state)
{
	struct adc_config config = {channel,
								state};
	ADC_ASSERT(adc != NULL);

	return adc->ops->channel_configure(adc, &config);
}

uint32_t adc_read(adc_t adc, uint16_t channel)
{
	ADC_ASSERT(adc != NULL);

	return adc->ops->read(adc, channel);
}