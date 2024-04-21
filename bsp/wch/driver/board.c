/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-16    MacRsh       First version
 */

#include "../mr-library/driver/include/mr_board.h"

void mr_delay_us(size_t us)
{
    Delay_Us(us);
}

void mr_delay_ms(size_t ms)
{
    Delay_Ms(ms);
}
