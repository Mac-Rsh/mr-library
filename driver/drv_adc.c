/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#include "drv_adc.h"

#if (MR_CFG_ADC == MR_CFG_ENABLE)

static struct drv_adc_data drv_adc_data[] =
    {
#ifdef MR_BSP_ADC_1
        {"adc1", /* ... */},
#endif
#ifdef MR_BSP_ADC_2
        {"adc2", /* ... */},
#endif
        /* ... */
    };

static struct mr_adc adc_device[mr_array_num(drv_adc_data)];

static mr_err_t drv_adc_configure(mr_adc_t adc, mr_state_t state)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;

    /* ... */

    return MR_ERR_OK;
}

static mr_err_t drv_adc_channel_configure(mr_adc_t adc, mr_adc_config_t config)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;

    /* ... */

    return MR_ERR_OK;
}

static mr_uint32_t drv_adc_read(mr_adc_t adc, mr_off_t channel)
{
    struct drv_adc_data *adc_data = (struct drv_adc_data *)adc->device.data;
    mr_uint32_t data = 0;

    /* ... */

    return data;
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