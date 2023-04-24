/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _MR_LOG_H_
#define _MR_LOG_H_

#include <mrdef.h>

#define MR_LOG_A(TAG, FORMAT, ...)    mr_log_output(MR_LOG_LEVEL_ASSERT, TAG, FORMAT, ##__VA_ARGS__)
#define MR_LOG_E(TAG, FORMAT, ...)    mr_log_output(MR_LOG_LEVEL_ERROR, TAG, FORMAT, ##__VA_ARGS__)
#define MR_LOG_W(TAG, FORMAT, ...)    mr_log_output(MR_LOG_LEVEL_WARNING, TAG, FORMAT, ##__VA_ARGS__)
#define MR_LOG_I(TAG, FORMAT, ...)    mr_log_output(MR_LOG_LEVEL_INFO, TAG, FORMAT, ##__VA_ARGS__)
#define MR_LOG_D(TAG, FORMAT, ...)    mr_log_output(MR_LOG_LEVEL_DEBUG, TAG, FORMAT, ##__VA_ARGS__)

#if (MR_LOG_ASSERT == MR_CONF_ENABLE)
#define MR_ASSERT(EX)            			 \
    do{                          			 \
        if (!(EX))               			 \
        {                        			 \
          MR_LOG_A(__FUNCTION__,     		 \
		           "File: %s, Line: %d\r\n", \
		  		   __FILE__,                 \
		  		   __LINE__);                \
          mr_assert_handler();   		 	 \
        }                        			 \
    }while(0)
#else
#define MR_ASSERT(EX)
#endif

void mr_log_output(mr_base_t level, const char *tag, const char *format, ...);
void mr_assert_handler(void);

#endif