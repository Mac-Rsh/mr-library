/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#include "event.h"

static struct
{
	uint16_t in;
	uint16_t out;

	uint32_t buf[EVENT_QUEUE_SIZE];
} event_queue;

extern struct event event_section(".event") _event_start;
extern struct event event_section(".event") _event_end;

static void event_queue_put(uint32_t data)
{
	event_queue.buf[event_queue.in ++] = data;
	event_queue.in %= EVENT_QUEUE_SIZE;

	if (event_queue.in == event_queue.out)
	{
		event_queue.out ++;
		event_queue.out %= EVENT_QUEUE_SIZE;
	}
}

static uint32_t event_queue_get(void)
{
	uint32_t data = 0;

	data = event_queue.buf[event_queue.out ++];
	event_queue.out %= EVENT_QUEUE_SIZE;

	return data;
}

int event_queue_full(void)
{
	return (event_queue.in + 1) % EVENT_QUEUE_SIZE == event_queue.out;
}

int event_queue_empty(void)
{
	return event_queue.in == event_queue.out;
}

/**
 * @brief Notify an event happening.
 *
 * @param id The event id.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int event_notify(uint32_t id)
{
	if (event_queue_full())
		return - EVENT_ERR_NO_MEMORY;

	event_queue_put(id);

	return EVENT_ERR_OK;
}

/**
 * @brief Handle events.
 */
void event_handle(void)
{
	uint32_t id = 0;
	struct event *event = 0;

	while (!event_queue_empty())
	{
		id = event_queue_get();

		for (event = &_event_start; event < &_event_end; event ++)
		{
			if (event->id == id)
			{
				event->cb(event->args);
			}
		}
	}
}