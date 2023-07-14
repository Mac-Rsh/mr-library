/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-12     MacRsh       first version
 */

#include "drv_pwm.h"

#if (MR_CONF_PWM == MR_CONF_ENABLE)

enum
{
#ifdef BSP_PWM_1
    PWM1_INDEX,
#endif
#ifdef BSP_PWM_2
    PWM2_INDEX,
#endif
#ifdef BSP_PWM_3
    PWM3_INDEX,
#endif
#ifdef BSP_PWM_4
    PWM4_INDEX,
#endif
#ifdef BSP_PWM_5
    PWM5_INDEX,
#endif
#ifdef BSP_PWM_8
    PWM8_INDEX,
#endif
#ifdef BSP_PWM_9
    PWM9_INDEX,
#endif
#ifdef BSP_PWM_10
    PWM10_INDEX,
#endif
};

static struct ch32_pwm ch32_pwm[] =
        {
#ifdef BSP_PWM_1
                {"pwm1"},
#endif
#ifdef BSP_PWM_2
                {"pwm2"},
#endif
#ifdef BSP_PWM_3
                {"pwm3"},
#endif
#ifdef BSP_PWM_4
                {"pwm4"},
#endif
#ifdef BSP_PWM_5
                {"pwm5"},
#endif
#ifdef BSP_PWM_8
                {"pwm8"},
#endif
#ifdef BSP_PWM_9
                {"pwm9"},
#endif
#ifdef BSP_PWM_10
                {"pwm10"},
#endif
        };

static struct mr_pwm_info pwm_device_info = {10000000, 5000};

static struct mr_pwm pwm_device[mr_array_get_length(ch32_pwm)];

static mr_err_t ch32_pwm_configure(mr_pwm_t pwm, struct mr_pwm_config *config)
{

}

static mr_err_t ch32_pwm_write(mr_pwm_t pwm, mr_uint8_t channel, mr_uint32_t duty)
{

}

static mr_uint32_t ch32_pwm_read(mr_pwm_t pwm, mr_uint8_t channel)
{

}

mr_err_t ch32_pwm_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(pwm_device);
    static struct mr_pwm_ops driver =
            {
                    ch32_pwm_configure,
                    ch32_pwm_write,
                    ch32_pwm_read,
            };

    while (count--)
    {
        ret = mr_pwm_device_add(&pwm_device[count],
                                ch32_pwm[count].name,
                                &ch32_pwm[count],
                                &driver,
                                &pwm_device_info);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_pwm_init);

#endif /* MR_CONF_PWM */