/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "drv_adc.h"

#ifdef MR_USING_ADC

#if !defined(MR_USING_ADC1) && !defined(MR_USING_ADC2)
#error "Please define at least one ADC macro like MR_USING_ADC1. Otherwise undefine MR_USING_ADC."
#else

enum drv_adc_index
{
#ifdef MR_USING_ADC1
    DRV_INDEX_ADC1,
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
    DRV_INDEX_ADC2,
#endif /* MR_USING_ADC2 */
};

static const char *adc_name[] =
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

static struct drv_adc_channel_data adc_channel_drv_data[] =
    {
        {ADC_Channel_0,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0},
        {ADC_Channel_1,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_1},
        {ADC_Channel_2,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_2},
        {ADC_Channel_3,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3},
        {ADC_Channel_4,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_4},
        {ADC_Channel_5,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5},
        {ADC_Channel_6,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_6},
        {ADC_Channel_7,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_7},
        {ADC_Channel_8,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_0},
        {ADC_Channel_9,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_1},
        {ADC_Channel_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_0},
        {ADC_Channel_11, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_1},
        {ADC_Channel_12, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_2},
        {ADC_Channel_13, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_3},
        {ADC_Channel_14, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_4},
        {ADC_Channel_15, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_5},
        {ADC_Channel_16, 0, MR_NULL,                  0},
        {ADC_Channel_17, 0, MR_NULL,                  0},
    };

static struct mr_adc adc_dev[mr_array_num(adc_drv_data)];

static struct drv_adc_channel_data *drv_adc_get_channel_data(int channel)
{
    if (channel >= mr_array_num(adc_channel_drv_data))
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
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

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

    /* Configure temp-sensor */
    if ((adc_channel_data->channel == ADC_Channel_16) || (adc_channel_data->channel == ADC_Channel_17))
    {
        ADC_TempSensorVrefintCmd(ENABLE);
    }
    return MR_EOK;
}

static uint32_t drv_adc_read(struct mr_adc *adc, int channel)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->dev.drv->data;
    struct drv_adc_channel_data *adc_channel_data = drv_adc_get_channel_data(channel);
    int i = 0;

    /* Check channel is valid */
    if (adc_channel_data == NULL)
    {
        return 0;
    }

    /* Read data */
    ADC_RegularChannelConfig(adc_data->instance, adc_channel_data->channel, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(adc_data->instance, ENABLE);
    while (ADC_GetFlagStatus(adc_data->instance, ADC_FLAG_EOC) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return 0;
        }
    }
    ADC_ClearFlag(adc_data->instance, ADC_FLAG_EOC);
    return ADC_GetConversionValue(adc_data->instance);
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
            Mr_Drv_Type_ADC,
            &adc_drv_ops,
            &adc_drv_data[DRV_INDEX_ADC1],
        },
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        {
            Mr_Drv_Type_ADC,
            &adc_drv_ops,
            &adc_drv_data[DRV_INDEX_ADC2],
        },
#endif /* MR_USING_ADC2 */
    };

int drv_adc_init(void)
{
    int index = 0;

    for (index = 0; index < mr_array_num(adc_dev); index++)
    {
        mr_adc_register(&adc_dev[index], adc_name[index], &adc_drv[index]);
    }
    return MR_EOK;
}
MR_DRV_EXPORT(drv_adc_init);

#endif /* !defined(MR_USING_ADC1) && !defined(MR_USING_ADC2) */

#endif /* MR_USING_ADC */
