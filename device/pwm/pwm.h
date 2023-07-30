/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-12     MacRsh       first version
 */

#ifndef _PWM_H_
#define _PWM_H_

#include "mrlib.h"

#if (MR_CONF_PWM == MR_CONF_ENABLE)

#define MR_PWM_MODE_NORMAL              0
#define MR_PWM_MODE_COMPLEMENTARY       1

/* Default config for mr_pwm_config structure */
#define MR_PWM_CONFIG_DEFAULT           \
{                                       \
    1000,                               \
    MR_PWM_MODE_NORMAL,                 \
    0,                                  \
    0,                                  \
}

struct mr_pwm_config
{
    mr_uint32_t freq;
    mr_uint8_t mode;
    mr_uint32_t dead_time;
    union
    {
        struct
        {
            mr_pos_t channel0: 1;
            mr_pos_t channel1: 1;
            mr_pos_t channel2: 1;
            mr_pos_t channel3: 1;
            mr_pos_t channel4: 1;
            mr_pos_t channel5: 1;
            mr_pos_t channel6: 1;
            mr_pos_t channel7: 1;
            mr_pos_t channel8: 1;
            mr_pos_t channel9: 1;
            mr_pos_t channel10: 1;
            mr_pos_t channel11: 1;
            mr_pos_t channel12: 1;
            mr_pos_t channel13: 1;
            mr_pos_t channel14: 1;
            mr_pos_t channel15: 1;
            mr_pos_t channel16: 1;
            mr_pos_t channel17: 1;
            mr_pos_t channel18: 1;
            mr_pos_t channel19: 1;
            mr_pos_t channel20: 1;
            mr_pos_t channel21: 1;
            mr_pos_t channel22: 1;
            mr_pos_t channel23: 1;
            mr_pos_t channel24: 1;
            mr_pos_t channel25: 1;
            mr_pos_t channel26: 1;
            mr_pos_t channel27: 1;
            mr_pos_t channel28: 1;
            mr_pos_t channel29: 1;
            mr_pos_t channel30: 1;
            mr_pos_t channel31: 1;
        };
        mr_pos_t _channel_mask;
    };
};

struct mr_pwm_info
{
    mr_uint32_t max_freq;
    mr_uint32_t min_freq;
};

typedef struct mr_pwm *mr_pwm_t;

struct mr_pwm_ops
{
    mr_err_t (*configure)(mr_pwm_t pwm, struct mr_pwm_config *config);
    mr_err_t (*write)(mr_pwm_t pwm, mr_pos_t channel, mr_uint32_t duty);
    mr_uint32_t (*read)(mr_pwm_t pwm, mr_pos_t channel);
};

struct mr_pwm
{
    struct mr_device device;

    struct mr_pwm_config config;
    struct mr_pwm_info info;

    const struct mr_pwm_ops *ops;
};

mr_err_t mr_pwm_device_add(mr_pwm_t pwm,
                           const char *name,
                           void *data,
                           struct mr_pwm_ops *ops,
                           struct mr_pwm_info *info);

#endif  /* MR_CONF_PWM */

#endif  /* _PWM_H_ */
