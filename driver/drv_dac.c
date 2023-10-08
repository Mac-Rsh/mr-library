/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#include "drv_dac.h"

#if (MR_CFG_DAC == MR_CFG_ENABLE)

static struct drv_dac_data drv_dac_data[] =
    {
#ifdef MR_BSP_DAC_1
        {"dac1", /* ... */},
#endif
#ifdef MR_BSP_DAC_2
        {"dac2", /* ... */},
#endif
        /* ... */
    };

static struct mr_dac dac_device[mr_array_num(drv_dac_data)];

static mr_err_t drv_dac_configure(mr_dac_t dac, mr_state_t state)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->device.data;

    /* ... */

    return MR_ERR_OK;
}

static mr_err_t drv_dac_channel_configure(mr_dac_t dac, mr_dac_config_t config)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->device.data;

    /* ... */

    return MR_ERR_OK;
}

static void drv_dac_write(mr_dac_t dac, mr_off_t channel, mr_uint32_t value)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->device.data;

    /* ... */
}

mr_err_t drv_dac_init(void)
{
    static struct mr_dac_ops drv_ops =
        {
            drv_dac_configure,
            drv_dac_channel_configure,
            drv_dac_write,
        };
    mr_size_t count = mr_array_num(dac_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_dac_device_add(&dac_device[count], drv_dac_data[count].name, &drv_ops, &drv_dac_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_dac_init);

#endif