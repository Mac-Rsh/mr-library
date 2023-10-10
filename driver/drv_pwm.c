/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-10     MacRsh       first version
 */

#include "drv_pwm.h"

#if (MR_CFG_PWM == MR_CFG_ENABLE)

static struct drv_pwm_data drv_pwm_data[] =
    {
#ifdef MR_BSP_PWM_1
        {"pwm1", /* ... */},
#endif
#ifdef MR_BSP_PWM_2
        {"pwm2", /* ... */},
#endif
#ifdef MR_BSP_PWM_3
        {"pwm3", /* ... */},
#endif
        /* ... */
    };

static struct mr_pwm pwm_device[mr_array_num(drv_pwm_data)];

static mr_err_t ch32_pwm_configure(mr_pwm_t pwm, mr_pwm_config_t config)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->device.data;

    /* ... */

    return MR_ERR_OK;
}

static void ch32_pwm_write(mr_pwm_t pwm, mr_off_t channel, mr_uint32_t duty)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->device.data;

    /* ... */
}

static mr_uint32_t ch32_pwm_read(mr_pwm_t pwm, mr_off_t channel)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->device.data;
    mr_uint32_t data = 0;

    /* ... */

    return data;
}

mr_err_t drv_pwm_init(void)
{
    static struct mr_pwm_ops drv_ops =
        {
            ch32_pwm_configure,
            ch32_pwm_write,
            ch32_pwm_read,
        };
    mr_size_t count = mr_array_num(pwm_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_pwm_device_add(&pwm_device[count],
                                drv_pwm_data[count].name,
                                &drv_ops,&drv_pwm_data[count]
        );
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_pwm_init);

#endif