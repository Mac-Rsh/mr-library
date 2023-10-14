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

int mr_board_init(void)
{
    return MR_ERR_OK;
}
MR_INIT_BOARD_EXPORT(mr_board_init);

void mr_delay_ms(mr_size_t ms)
{
    HAL_Delay(ms);
}
