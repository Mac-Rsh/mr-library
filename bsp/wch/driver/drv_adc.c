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

static struct mr_adc adc_device[mr_array_num(ch32_adc_data)];

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
    ADC_Init(adc_data->instance, &ADC_InitStructure);
    ADC_Cmd(adc_data->instance, (FunctionalState)state);

    return MR_ERR_OK;
}

static mr_err_t ch32_adc_channel_configure(mr_adc_t adc, mr_adc_config_t config)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_TypeDef *GPIOx = {0};
    mr_size_t count = 0;

    for (count = 0; count <= 17; count++)
    {
        if (count <= 7)
        {
            if (((1 << count) & config->channel._mask))
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

                GPIOx = GPIOA;
                GPIO_InitStructure.GPIO_Pin = (1 << count);
                GPIO_Init(GPIOx, &GPIO_InitStructure);
            }

        } else if (count <= 10)
        {
            if (((1 << count) & config->channel._mask))
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

                GPIOx = GPIOB;
                GPIO_InitStructure.GPIO_Pin = (1 << count);
                GPIO_Init(GPIOx, &GPIO_InitStructure);
            }
        } else if (count <= 15)
        {
            if (((1 << count) & config->channel._mask))
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

                GPIOx = GPIOC;
                GPIO_InitStructure.GPIO_Pin = (1 << count);
                GPIO_Init(GPIOx, &GPIO_InitStructure);
            }
        } else if (count <= 17)
        {
            if (((1 << count) & config->channel._mask))
            {
                ADC_TempSensorVrefintCmd(ENABLE);
            } else
            {
                ADC_TempSensorVrefintCmd(DISABLE);
            }
        }
    }

    return MR_ERR_OK;
}

static mr_uint32_t ch32_adc_read(mr_adc_t adc, mr_off_t channel)
{
    struct ch32_adc_data *adc_data = (struct ch32_adc_data *)adc->device.data;
    mr_size_t i = 0;

    if (channel > 17)
    {
        return 0;
    }

    ADC_RegularChannelConfig(adc_data->instance, channel, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(adc_data->instance, ENABLE);
    while (!ADC_GetFlagStatus(adc_data->instance, ADC_FLAG_EOC))
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }
    ADC_ClearFlag(adc_data->instance, ADC_FLAG_EOC);

    return ADC_GetConversionValue(adc_data->instance);
}

mr_err_t drv_adc_init(void)
{
    static struct mr_adc_ops drv_ops =
        {
            ch32_adc_configure,
            ch32_adc_channel_configure,
            ch32_adc_read,
        };
    mr_size_t count = mr_array_num(adc_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_adc_device_add(&adc_device[count], ch32_adc_data[count].name, &drv_ops, &ch32_adc_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_adc_init);

#endif
