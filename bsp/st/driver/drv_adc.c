/*
 * @copyright (c) 2023, MR Development Team
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
    adc_data->handle.Instance = adc_data->instance;

    if (state == ENABLE)
    {
        /* Configure ADC */
        adc_data->handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
#if defined(ADC_RESOLUTION_12B)
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_12B;
#elif defined(ADC_RESOLUTION_10B)
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_10B;
#else
        adc_data->handle.Init.Resolution = ADC_RESOLUTION_8B;
#endif /* ADC_RESOLUTION_12B */
        adc_data->handle.Init.ScanConvMode = DISABLE;
        adc_data->handle.Init.ContinuousConvMode = DISABLE;
        adc_data->handle.Init.DiscontinuousConvMode = DISABLE;
        adc_data->handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
        adc_data->handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
        adc_data->handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
        adc_data->handle.Init.NbrOfConversion = 1;
        adc_data->handle.Init.DMAContinuousRequests = DISABLE;
        adc_data->handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
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
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    HAL_ADC_ConfigChannel(&adc_data->handle, &sConfig);
    HAL_ADC_Start(&adc_data->handle);
    HAL_ADC_PollForConversion(&adc_data->handle, INT16_MAX);
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
    int index = 0;

    for (index = 0; index < mr_array_num(adc_dev); index++)
    {
        mr_adc_register(&adc_dev[index], adc_name[index], &adc_drv[index]);
    }
    return MR_EOK;
}
MR_DRV_EXPORT(drv_adc_init);

#endif /* MR_USING_ADC */
