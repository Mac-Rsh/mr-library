/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-14     MacRsh       first version
 */

#ifndef _MR_LOG_H_
#define _MR_LOG_H_

#include <mrdef.h>

MR_INLINE void mr_assert_handler()
{
	while (1)
	{

	}
}

#define MR_ASSERT(EX)           \
    do{                         \
        if (!(EX))              \
        {                       \
          mr_printf("ASSERT: [%s] [%d]\r\n", __FUNCTION__ , __LINE__);                      \
          mr_assert_handler();  \
        }                       \
    }while(0)

#endif
