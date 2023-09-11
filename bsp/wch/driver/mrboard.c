/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-11     MacRsh       first version
 */

#include "mrboard.h"

void mr_delay_us(mr_size_t us)
{
    Delay_Us(us);
}

void mr_delay_ms(mr_size_t ms)
{
    Delay_Ms(ms);
}