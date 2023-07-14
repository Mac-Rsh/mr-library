/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-12     MacRsh       first version
 */

#ifndef _DRV_PWM_H
#define _DRV_PWM_H

#include "device/pwm/pwm.h"

#if (MR_CONF_PWM == MR_CONF_ENABLE)

struct ch32_pwm_info
{

};

struct ch32_pwm
{
    const char *name;

    struct ch32_pwm_info info;
};

#endif /* MR_CONF_PWM */

#endif /* _DRV_PWM_H */
