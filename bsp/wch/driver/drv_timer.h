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

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

/**
 * @struct ch32 Timer data
 */
struct ch32_timer_data
{
    const char *name;

    TIM_TypeDef *instance;
    mr_uint32_t timer_periph_clock;
    IRQn_Type irqno;
};

#endif

#endif /* _DRV_TIMER_H_ */