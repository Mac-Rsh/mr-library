/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-17     MacRsh       first version
 */

#ifndef _DRV_TIMER_H_
#define _DRV_TIMER_H_

#include "device/timer/timer.h"

#if (MR_CONF_TIMER == MR_CONF_ENABLE)

struct ch32_timer_info
{
    TIM_TypeDef *Instance;
    mr_uint32_t timer_periph_clock;
    IRQn_Type irqno;
};

struct ch32_timer
{
    const char *name;

    struct ch32_timer_info info;
};

mr_err_t ch32_timer_init(void);

#endif /* MR_CONF_TIMER */

#endif /* _DRV_TIMER_H_ */
