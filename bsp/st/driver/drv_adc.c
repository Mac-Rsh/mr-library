/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "drv_adc.h"

#ifdef MR_USING_ADC

enum drv_adc_index
{
#ifdef MR_USING_ADC1
    DRV_INDEX_ADC1,
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
    DRV_INDEX_ADC2,
#endif /* MR_USING_ADC2 */
#ifdef MR_USING_ADC3
    DRV_INDEX_ADC3,
#endif /* MR_USING_ADC3 */
    DRV_INDEX_ADC_MAX
};

static const char *adc_name[] =
    {
#ifdef MR_USING_ADC1
        "adc1",
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        "adc2",
#endif /* MR_USING_ADC2 */
#ifdef MR_USING_ADC3
        "adc3",
#endif /* MR_USING_ADC3 */
    };

static struct drv_adc_data adc_drv_data[] =
    {
#ifdef MR_USING_ADC1
        {{0}, ADC1},
#endif /* MR_USING_ADC1 */
#ifdef MR_USING_ADC2
        {{0}, ADC2},
#endif /* MR_USING_ADC2 */
#ifdef MR_USING_ADC3
        {{0}, ADC3},
#endif /* MR_USING_ADC3 */
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
    adc_data->handle.Instance = adc_data->instance;

    if (state == ENABLE)
    {
        /* Configure ADC */
#if defined(ADC_RESOLUTION_12B)
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_12B;
#elif defined(ADC_RESOLUTION_10B)
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_10B;
#elif defined(ADC_RESOLUTION_8B)
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_8B;
#endif /* defined(ADC_RESOLUTION_12B) */
        adc_data->handle.Init.ScanConvMode = DISABLE;
        adc_data->handle.Init.ContinuousConvMode = DISABLE;
        adc_data->handle.Init.DiscontinuousConvMode = DISABLE;
        adc_data->handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
        adc_data->handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
        adc_data->handle.Init.NbrOfConversion = 1;
#if defined(ADC_EOC_SINGLE_CONV)
        adc_data->handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
#endif /* defined(ADC_EOC_SINGLE_CONV) */
        HAL_ADC_Init(&adc_data->handle);
    } else
    {
        /* Configure ADC */
        HAL_ADC_DeInit(&adc_data->handle);
    }
    return MR_EOK;
}

static int drv_adc_channel_configure(struct mr_adc *adc, int channel, int state)
{
    return MR_EOK;
}

static uint32_t drv_adc_read(struct mr_adc *adc, int channel)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->dev.drv->data;
    struct drv_adc_channel_data *adc_channel_data = drv_adc_get_channel_data(channel);
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Check channel is valid */
    if (adc_channel_data == NULL)
    {
        return 0;
    }

    /* Read data */
    sConfig.Channel = adc_channel_data->channel;
    sConfig.Rank = 1;
#if defined(ADC_SAMPLETIME_55CYCLES_5)
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
#elif defined(ADC_SAMPLETIME_56CYCLES)
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
#elif defined(ADC_SAMPLETIME_71CYCLES_5)
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
#elif defined(ADC_SAMPLETIME_84CYCLES)
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
#elif defined(ADC_SAMPLETIME_112CYCLES)
    sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
#elif defined(ADC_SAMPLETIME_144CYCLES)
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
#elif defined(ADC_SAMPLETIME_239CYCLES_5)
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
#elif defined(ADC_SAMPLETIME_247CYCLES_5)
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
#elif defined(ADC_SAMPLETIME_480CYCLES)
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
#endif /* defined(ADC_SAMPLETIME_55CYCLES_5) */
    HAL_ADC_ConfigChannel(&adc_data->handle, &sConfig);
    HAL_ADC_Start(&adc_data->handle);
    HAL_ADC_PollForConversion(&adc_data->handle, UINT16_MAX);
    HAL_ADC_Stop(&adc_data->handle);
    return HAL_ADC_GetValue(&adc_data->handle);
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
#ifdef MR_USING_ADC3
        {
            Mr_Drv_Type_ADC,
            &adc_drv_ops,
            &adc_drv_data[DRV_INDEX_ADC3],
        },
#endif /* MR_USING_ADC3 */
    };

int drv_adc_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(adc_dev); i++)
    {
        mr_adc_register(&adc_dev[i], adc_name[i], &adc_drv[i]);
    }
    return MR_EOK;
}
MR_INIT_DRV_EXPORT(drv_adc_init);

#endif /* MR_USING_ADC */
