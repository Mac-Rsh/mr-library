/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#ifndef _DRV_TIMER_H_
#define _DRV_TIMER_H_

#include "device/timer.h"
#include "mrboard.h"

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

/**
 * @struct Driver timer data
 */
struct drv_timer_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_TIMER_H_ */
