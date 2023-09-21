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

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_PWM == MR_CFG_ENABLE)

/**
 * @def PWM device mode
 */
#define MR_PWM_MODE_NORMAL              0
#define MR_PWM_MODE_COMPLEMENTARY       1

/**
 * @def PWM device default config
 */
#define MR_PWM_CONFIG_DEFAULT           \
{                                       \
    1000,                               \
    MR_PWM_MODE_NORMAL,                 \
    0,                                  \
    0,                                  \
}

/**
 * @struct PWM device config
 */
struct mr_pwm_config
{
    mr_uint32_t freq;
    mr_uint32_t mode: 1;
    mr_uint32_t dead_time: 31;
    struct mr_device_channel channel;
};
typedef struct mr_pwm_config *mr_pwm_config_t;

typedef struct mr_pwm *mr_pwm_t;

/**
 * @struct PWM device operations
 */
struct mr_pwm_ops
{
    mr_err_t (*configure)(mr_pwm_t pwm, mr_pwm_config_t config);
    void (*write)(mr_pwm_t pwm, mr_off_t channel, mr_uint32_t duty);
    mr_uint32_t (*read)(mr_pwm_t pwm, mr_off_t channel);
};

/**
 * @struct PWM device
 */
struct mr_pwm
{
    struct mr_device device;

    struct mr_pwm_config config;

    const struct mr_pwm_ops *ops;
};

mr_err_t mr_pwm_device_add(mr_pwm_t pwm, const char *name, struct mr_pwm_ops *ops, void *data);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _PWM_H_ */
