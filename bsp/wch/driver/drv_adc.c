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

#if (MR_CFG_ADC == MR_CFG_ENABLE)

static struct ch32_adc_data ch32_adc_data[] =
    {
#ifdef MR_BSP_ADC_1
        {"adc1", ADC1, RCC_APB2Periph_ADC1},
#endif
#ifdef MR_BSP_ADC_2
        {"adc2", ADC2, RCC_APB2Periph_ADC2},
#endif
    };

static struct mr_adc adc_device[MR_ARRAY_SIZE(ch32_adc_data)];

static mr_err_t ch32_adc_configure(mr_adc_t adc, mr_state_t state)
{
    struct ch32_adc_data *adc_data = (struct ch32_adc_data *)adc->device.data;
    ADC_InitTypeDef ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(adc_data->periph_clock, (FunctionalState)state);
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(adc_data->Instance, &ADC_InitStructure);
    ADC_Cmd(adc_data->Instance, (FunctionalState)state);

    return MR_ERR_OK;
}

static mr_err_t ch32_adc_channel_configure(mr_adc_t adc, mr_adc_config_t config)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_TypeDef *GPIOx = {0};
    mr_size_t count = 0;

    for (count = 0; count <= 17; count++)
    {
        if ((1 << count) & config->channel.mask)
        {
            if (count <= 7)
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
                GPIOx = GPIOA;
            } else if (count <= 10)
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
                GPIOx = GPIOB;
            } else if (count <= 15)
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
                GPIOx = GPIOC;
            } else if (count <= 17)
            {
                ADC_TempSensorVrefintCmd(ENABLE);
            }

            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_InitStructure.GPIO_Pin = (1 << count);
            GPIO_Init(GPIOx, &GPIO_InitStructure);
        }
    }

    return MR_ERR_OK;
}

static mr_uint32_t ch32_adc_read(mr_adc_t adc, mr_pos_t channel)
{
    struct ch32_adc_data *adc_data = (struct ch32_adc_data *)adc->device.data;
    mr_uint32_t data = 0;

    if (channel > 17)
    {
        return 0;
    }

    ADC_RegularChannelConfig(adc_data->Instance, channel, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(adc_data->Instance, ENABLE);
    while (!ADC_GetFlagStatus(adc_data->Instance, ADC_FLAG_EOC));
    data = ADC_GetConversionValue(adc_data->Instance);
    ADC_ClearFlag(adc_data->Instance, ADC_FLAG_EOC);

    return data;
}

mr_err_t drv_adc_init(void)
{
    static struct mr_adc_ops drv_ops =
        {
            ch32_adc_configure,
            ch32_adc_channel_configure,
            ch32_adc_read,
        };
    mr_size_t count = MR_ARRAY_SIZE(adc_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_adc_device_add(&adc_device[count], ch32_adc_data[count].name, &drv_ops, &ch32_adc_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
MR_INIT_DRIVER_EXPORT(drv_adc_init);

#endif