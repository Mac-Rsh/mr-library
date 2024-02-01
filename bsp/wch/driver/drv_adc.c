/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "drv_adc.h"

#ifdef MR_USING_ADC

#if !defined(MR_USING_ADC1) && !defined(MR_USING_ADC2)
#warning "Please enable at least one ADC driver"
#endif /* !defined(MR_USING_ADC1) && !defined(MR_USING_ADC2) */

enum drv_adc_index
{
#ifdef MR_USING_ADC1
    DRV_INDEX_ADC1,
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
    DRV_INDEX_ADC2,
#endif /* MR_USING_ADC2 */
    DRV_INDEX_ADC_MAX
};

static const char *adc_path[] =
    {
#ifdef MR_USING_ADC1
        "adc1",
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        "adc2",
#endif /* MR_USING_ADC2 */
    };

static struct drv_adc_data adc_drv_data[] =
    {
#ifdef MR_USING_ADC1
        {ADC1, RCC_APB2Periph_ADC1},
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        {ADC2, RCC_APB2Periph_ADC2},
#endif /* MR_USING_ADC2 */
    };

static struct drv_adc_channel_data adc_channel_drv_data[] = DRV_ADC_CHANNEL_CONFIG;

static struct mr_adc adc_dev[MR_ARRAY_NUM(adc_drv_data)];

static struct drv_adc_channel_data *drv_adc_get_channel_data(int channel)
{
    if (channel >= MR_ARRAY_NUM(adc_channel_drv_data))
    {
        return NULL;
    }
    return &adc_channel_drv_data[channel];
}

static int drv_adc_configure(struct mr_adc *adc, int state)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->dev.drv->data;
    ADC_InitTypeDef ADC_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(adc_data->clock, state);
#ifdef MR_USING_CH32V00X
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);
#else
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
#endif /* MR_USING_CH32V00X */

    /* Configure ADC */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(adc_data->instance, &ADC_InitStructure);
    ADC_Cmd(adc_data->instance, state);
    return MR_EOK;
}

static int drv_adc_channel_configure(struct mr_adc *adc, int channel, int state)
{
    struct drv_adc_channel_data *adc_channel_data = drv_adc_get_channel_data(channel);
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* Check channel is valid */
    if (adc_channel_data == NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
    RCC_APB2PeriphClockCmd(adc_channel_data->gpio_clock, ENABLE);

    /* Configure pin */
    if (adc_channel_data->port != MR_NULL)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStructure.GPIO_Pin = adc_channel_data->pin;
        GPIO_Init(adc_channel_data->port, &GPIO_InitStructure);
    }

#ifndef MR_USING_CH32V00X
    /* Configure temp-sensor */
    if ((adc_channel_data->channel == ADC_Channel_16) || (adc_channel_data->channel == ADC_Channel_17))
    {
        ADC_TempSensorVrefintCmd(ENABLE);
    }
#endif /* MR_USING_CH32V00X */
    return MR_EOK;
}

static int drv_adc_read(struct mr_adc *adc, int channel, uint32_t *data)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->dev.drv->data;
    struct drv_adc_channel_data *adc_channel_data = drv_adc_get_channel_data(channel);
    size_t i = 0;

#ifdef MR_USING_ADC_CHANNEL_CHECK
    /* Check channel is valid */
    if (adc_channel_data == NULL)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_ADC_CHANNEL_CHECK */

    /* Read data */
#ifdef MR_USING_CH32V00X
    ADC_RegularChannelConfig(adc_data->instance, adc_channel_data->channel, 1, ADC_SampleTime_15Cycles);
#else
    ADC_RegularChannelConfig(adc_data->instance, adc_channel_data->channel, 1, ADC_SampleTime_13Cycles5);
#endif /* MR_USING_CH32V00X */
    ADC_SoftwareStartConvCmd(adc_data->instance, ENABLE);
    while (ADC_GetFlagStatus(adc_data->instance, ADC_FLAG_EOC) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    ADC_ClearFlag(adc_data->instance, ADC_FLAG_EOC);
    *data = ADC_GetConversionValue(adc_data->instance);
    return MR_EOK;
}

static struct mr_adc_ops adc_drv_ops =
    {
        drv_adc_configure,
        drv_adc_channel_configure,
        drv_adc_read
    };

static struct mr_drv adc_drv[] =
    {
#ifdef MR_USING_ADC1
        {
            &adc_drv_ops,
            &adc_drv_data[DRV_INDEX_ADC1],
        },
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        {
            &adc_drv_ops,
            &adc_drv_data[DRV_INDEX_ADC2],
        },
#endif /* MR_USING_ADC2 */
    };

static void drv_adc_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(adc_dev); i++)
    {
        mr_adc_register(&adc_dev[i], adc_path[i], &adc_drv[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_adc_init);

#endif /* MR_USING_ADC */
