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

/* Default config for mr_pwm_config structure */
#define MR_PWM_CONFIG_DEFAULT           \
{                                       \
    1000,                               \
}

struct mr_pwm_config
{
    mr_uint32_t freq;
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
    mr_err_t (*write)(mr_pwm_t pwm, mr_uint8_t channel, mr_uint32_t duty);
    mr_uint32_t (*read)(mr_pwm_t pwm, mr_uint8_t channel);
};

struct mr_pwm
{
    struct mr_device device;

    struct mr_pwm_config config;
    struct mr_pwm_info info;

    const struct mr_pwm_ops *ops;
};

#endif /* MR_CONF_PWM */

#endif /* _PWM_H_ */
