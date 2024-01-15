/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-16    MacRsh       First version
 */

#include "mr_board.h"

void mr_delay_us(uint32_t us)
{
    Delay_Us(us);
}

void mr_delay_ms(uint32_t ms)
{
    Delay_Ms(ms);
}
