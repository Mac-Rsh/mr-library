/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 */

#include "drv_adc.h"

#undef LOG_TAG
#define LOG_TAG "drv_adc"

#if (MR_CONF_ADC == MR_CONF_ENABLE)

static struct ch32_adc ch32_adc[] =
	{
#ifdef BSP_ADC_1
		{"adc1",
		 {ADC1,
		  RCC_APB2Periph_ADC1}},
#endif
#ifdef BSP_ADC_2
		{"adc2",
		 {ADC2,
		  RCC_APB2Periph_ADC2}},
#endif
	};

static struct mr_adc hw_adc[mr_array_get_length(ch32_adc)];

mr_err_t ch32_adc_configure(mr_adc_t adc, mr_state_t state)
{
	struct ch32_adc *hw = (struct ch32_adc *)adc->device.data;
	ADC_InitTypeDef ADC_InitStructure = {0};

	RCC_APB2PeriphClockCmd(hw->hw_adc.adc_periph_clock, (FunctionalState)state);
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(hw->hw_adc.Instance, &ADC_InitStructure);
	ADC_Cmd(hw->hw_adc.Instance, (FunctionalState)state);

	return MR_ERR_OK;
}

mr_err_t ch32_adc_channel_configure(mr_adc_t adc, struct mr_adc_config *config)
{
	struct ch32_adc *hw = (struct ch32_adc *)adc->device.data;
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	GPIO_TypeDef *GPIOx = {0};

	if (config->channel <= 7)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOx = GPIOA;
	} else if (config->channel <= 10)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIOx = GPIOB;
	} else if (config->channel <= 15)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIOx = GPIOC;
	}
	else if(config->channel <= 17)
	{
		ADC_TempSensorVrefintCmd(ENABLE);
	}
	else
		return -MR_ERR_INVALID;

	if (config->state == MR_ENABLE)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	else
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_InitStructure.GPIO_Pin = (1 << config->channel);
	GPIO_Init(GPIOx, &GPIO_InitStructure);

	MR_LOG_D(LOG_TAG,
			 "Config %s %d %d\r\n",
			 hw->name,
			 config->channel,
			 config->state);

	return MR_ERR_OK;
}

mr_uint32_t ch32_adc_read(mr_adc_t adc, mr_uint16_t channel)
{
	struct ch32_adc *hw = (struct ch32_adc *)adc->device.data;
	mr_uint32_t data = 0;

	if (channel >17)
		return 0;

	ADC_RegularChannelConfig(hw->hw_adc.Instance, channel, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(hw->hw_adc.Instance, ENABLE);
	while (! ADC_GetFlagStatus(hw->hw_adc.Instance, ADC_FLAG_EOC));
	data = ADC_GetConversionValue(hw->hw_adc.Instance);
	ADC_ClearFlag(hw->hw_adc.Instance, ADC_FLAG_EOC);

	return data;
}

mr_err_t mr_hw_adc_init(void)
{
	mr_err_t ret = MR_ERR_OK;
	mr_size_t count = mr_array_get_length(hw_adc);
	static struct mr_adc_ops ops =
		{
			ch32_adc_configure,
			ch32_adc_channel_configure,
			ch32_adc_read,
		};

	while (count --)
	{
		ret = mr_hw_adc_add(&hw_adc[count], ch32_adc[count].name, &ops, &ch32_adc[count]);
		MR_ASSERT(ret == MR_ERR_OK);
	}

	return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(mr_hw_adc_init);

#endif