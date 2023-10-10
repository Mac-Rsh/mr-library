/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-10     MacRsh       first version
 */

#ifndef _DRV_PWM_H_
#define _DRV_PWM_H_

#include "device/pwm.h"
#include "mrboard.h"

#if (MR_CFG_PWM == MR_CFG_ENABLE)

/**
 * @struct Driver pwm data
 */
struct drv_pwm_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_PWM_H_ */
