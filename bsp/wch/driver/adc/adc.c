/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "../mr-library/driver/include/mr_adc.h"

#ifdef MR_USE_ADC

#if !defined(MR_USE_ADC1) && !defined(MR_USE_ADC2)
#warning "Please enable at least one ADC driver"
#else

enum _adc_driver_index
{
#ifdef MR_USE_ADC1
    _DRIVER_ADC1_INDEX,
#endif /* MR_USE_ADC1 */
#ifdef MR_USE_ADC2
    _DRIVER_ADC2_INDEX,
#endif /* MR_USE_ADC2 */
    _DRIVER_ADC_INDEX_MAX
};

static struct mr_adc_driver _adc_driver[] = {
#ifdef MR_USE_ADC1
    _DRIVER_ADC1_CONFIG,
#endif /* MR_USE_ADC1 */
#ifdef MR_USE_ADC2
    _DRIVER_ADC2_CONFIG,
#endif /* MR_USE_ADC2 */
};

static struct mr_adc_driver_channel _adc_driver_channel[] =
    _DRIVER_ADC_CHANNEL_CONFIG;

static const char *_adc_device_path[] = {
#ifdef MR_USE_ADC1
    "adc1",
#endif /* MR_USE_ADC1 */
#ifdef MR_USE_ADC2
    "adc2",
#endif /* MR_USE_ADC2 */
};

static struct mr_adc _adc_device[MR_ARRAY_NUM(_adc_device_path)];

MR_INLINE struct mr_adc_driver_channel *_adc_channel_get(uint32_t channel)
{
    if (channel >= MR_ARRAY_NUM(_adc_driver_channel))
    {
        return NULL;
    }
    return &_adc_driver_channel[channel];
}

static int adc_driver_configure(struct mr_driver *driver, bool enable)
{
    struct mr_adc_driver *adc = (struct mr_adc_driver *)driver;
    ADC_InitTypeDef ADC_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(adc->clock, enable);
#ifdef MR_USE_CH32V00X
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);
#else
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
#endif /* MR_USE_CH32V00X */

    /* Configure ADC */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(adc->instance, &ADC_InitStructure);
    ADC_Cmd(adc->instance, enable);
    return MR_EOK;
}

static int adc_driver_channel_configure(struct mr_driver *driver,
                                        uint32_t channel, bool enable)
{
    struct mr_adc_driver_channel *adc_channel = _adc_channel_get(channel);
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    if (adc_channel == NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
    RCC_APB2PeriphClockCmd(adc_channel->gpio_clock, ENABLE);

    /* Configure pin */
    if (adc_channel->port != NULL)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStructure.GPIO_Pin = adc_channel->pin;
        GPIO_Init(adc_channel->port, &GPIO_InitStructure);
    }

#ifndef MR_USE_CH32V00X
    /* Configure temp-sensor */
    if ((adc_channel->channel == ADC_Channel_16) ||
        (adc_channel->channel == ADC_Channel_17))
    {
        ADC_TempSensorVrefintCmd(ENABLE);
    }
#endif /* MR_USE_CH32V00X */
    return MR_EOK;
}

static int adc_driver_read(struct mr_driver *driver, uint32_t channel,
                           uint32_t *data)
{
    struct mr_adc_driver *adc = (struct mr_adc_driver *)driver;
    struct mr_adc_driver_channel *adc_channel = _adc_channel_get(channel);
    size_t i = 0;

    if (adc_channel == NULL)
    {
        return MR_EINVAL;
    }

    /* Read data */
#ifdef MR_USE_CH32V00X
    ADC_RegularChannelConfig(adc->instance, adc_channel->channel, 1,
                             ADC_SampleTime_15Cycles);
#else
    ADC_RegularChannelConfig(adc->instance, adc_channel->channel, 1,
                             ADC_SampleTime_13Cycles5);
#endif /* MR_USE_CH32V00X */
    ADC_SoftwareStartConvCmd(adc->instance, ENABLE);
    while (ADC_GetFlagStatus(adc->instance, ADC_FLAG_EOC) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    ADC_ClearFlag(adc->instance, ADC_FLAG_EOC);
    *data = ADC_GetConversionValue(adc->instance);
    return MR_EOK;
}

static void adc_driver_init(void)
{
    static struct mr_adc_driver_ops ops = {.configure = adc_driver_configure,
                                           .channel_configure =
                                               adc_driver_channel_configure,
                                           .read = adc_driver_read};
    static struct mr_adc_driver_data data = {.channels = _DRIVER_ADC_CHANNELS,
                                             .resolution =
                                                 _DRIVER_ADC_RESOLUTION};

    for (size_t i = 0; i < MR_ARRAY_NUM(_adc_device); i++)
    {
        _adc_driver[i].driver.ops = &ops;
        _adc_driver[i].driver.data = &data;
        mr_adc_register(&_adc_device[i], _adc_device_path[i],
                        (struct mr_driver *)&_adc_driver[i]);
    }
}
MR_INIT_DRIVER_EXPORT(adc_driver_init);

#endif /* !defined(MR_USE_ADC1) && !defined(MR_USE_ADC2) */

#endif /* MR_USE_ADC */
