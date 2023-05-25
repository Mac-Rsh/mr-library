/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#ifndef _EVENT_H_
#define _EVENT_H_

#include "stdint.h"

#define EVENT_QUEUE_SIZE                16

struct event
{
	uint32_t id;

	void (*cb)(void *args);
	void *args;
};

#define EVENT_ERR_OK                    0
#define EVENT_ERR_QUEUE_FULL            1

#if defined(__ARMCC_VERSION)
#define event_section(x)              	__attribute__((section(x)))
#define event_used                    	__attribute__((used))

#elif defined (__IAR_SYSTEMS_ICC__)
#define event_section(x)               	@ x
#define event_used                     	__root

#elif defined (__GNUC__)
#define event_section(x)                __attribute__((section(x)))
#define event_used                      __attribute__((used))

#elif defined (__ADSPBLACKFIN__)
#define event_section(x)               	__attribute__((section(x)))
#define event_used                     	__attribute__((used))

#elif defined (_MSC_VER)
#define event_section(x)
#define event_used

#elif defined (__TASKING__)
#define event_section(x)               	__attribute__((section(x)))
#define event_used                     	__attribute__((used, protect))
#endif

#define EVENT_EXPORT(id, fn, args) \
    event_used const struct event event_section(".event") _event_##id = {id, fn, args}

int event_notify(uint32_t id);
void event_handle(void);

#endif