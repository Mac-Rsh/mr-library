/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-15     MacRsh       first version
 */

#include "drv_adc.h"

#if (MR_CFG_ADC == MR_CFG_ENABLE)

static mr_uint32_t stm32_adc_channel[] =
    {
        ADC_CHANNEL_0,
        ADC_CHANNEL_1,
        ADC_CHANNEL_2,
        ADC_CHANNEL_3,
        ADC_CHANNEL_4,
        ADC_CHANNEL_5,
        ADC_CHANNEL_6,
        ADC_CHANNEL_7,
        ADC_CHANNEL_8,
        ADC_CHANNEL_9,
        ADC_CHANNEL_10,
        ADC_CHANNEL_11,
        ADC_CHANNEL_12,
        ADC_CHANNEL_13,
        ADC_CHANNEL_14,
        ADC_CHANNEL_15,
#ifdef ADC_CHANNEL_16
        ADC_CHANNEL_16,
#endif
#ifdef ADC_CHANNEL_17
        ADC_CHANNEL_17,
#endif
#ifdef ADC_CHANNEL_18
        ADC_CHANNEL_18,
#endif
    };

static struct drv_adc_data drv_adc_data[] =
    {
#ifdef MR_BSP_ADC_1
        {"adc1", {0}, ADC1},
#endif
#ifdef MR_BSP_ADC_2
        {"adc2", {0}, ADC2},
#endif
#ifdef MR_BSP_ADC_3
        {"adc3", {0}, ADC3},
#endif
    };

static struct mr_adc adc_device[mr_array_num(drv_adc_data)];

static mr_err_t drv_adc_configure(mr_adc_t adc, mr_state_t state)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;

    adc_data->handle.Instance = adc_data->instance;

    adc_data->handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    adc_data->handle.Init.Resolution = ADC_RESOLUTION_12B;
    adc_data->handle.Init.ScanConvMode = DISABLE;
    adc_data->handle.Init.ContinuousConvMode = DISABLE;
    adc_data->handle.Init.DiscontinuousConvMode = DISABLE;
    adc_data->handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_data->handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_data->handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_data->handle.Init.NbrOfConversion = 1;
    adc_data->handle.Init.DMAContinuousRequests = DISABLE;
    adc_data->handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&adc_data->handle) != HAL_OK)
    {
        return MR_ERR_GENERIC;
    }

    return MR_ERR_OK;
}

static mr_err_t drv_adc_channel_configure(mr_adc_t adc, mr_adc_config_t config)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;
    ADC_ChannelConfTypeDef channel_config = {0};
    mr_size_t count = 0;

    for (count = 0; count < (mr_size_t)mr_array_num(stm32_adc_channel); count++)
    {
        if (((1 << count) & config->channel._mask))
        {
            channel_config.Channel = stm32_adc_channel[count];
            channel_config.Rank = 1;
            channel_config.SamplingTime = ADC_SAMPLETIME_56CYCLES;
            if (HAL_ADC_ConfigChannel(&adc_data->handle, &channel_config) != HAL_OK)
            {
                return MR_ERR_GENERIC;
            }
        }
    }

    return MR_ERR_OK;
}

static mr_uint32_t drv_adc_read(mr_adc_t adc, mr_off_t channel)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;
    mr_size_t i = 0;

    HAL_ADC_Start(&adc_data->handle);
    while (__HAL_ADC_GET_FLAG(&adc_data->handle, ADC_FLAG_EOC) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }
    return (mr_uint32_t)HAL_ADC_GetValue(&adc_data->handle);
}

mr_err_t drv_adc_init(void)
{
    static struct mr_adc_ops drv_ops =
        {
            drv_adc_configure,
            drv_adc_channel_configure,
            drv_adc_read,
        };
    mr_size_t count = mr_array_num(adc_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_adc_device_add(&adc_device[count], drv_adc_data[count].name, &drv_ops, &drv_adc_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_adc_init);

#endif
