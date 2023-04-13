/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-02     MacRsh       first version
 */

#include <mrlib.h>

mr_event_manager_t mr_event_manager_find(const char *name)
{
	mr_event_manager_t event_manager = MR_NULL;

	MR_ASSERT(name != MR_NULL);

	/* Find the event-manager object from the device container */
	event_manager = (mr_event_manager_t)mr_object_find(name, MR_CONTAINER_TYPE_EVENT);

	return event_manager;
}

mr_err_t mr_event_manager_add_to_container(mr_event_manager_t event_manager,
										   const char *name,
										   enum mr_event_manager_type type,
										   mr_uint8_t *queue_pool,
										   mr_size_t queue_pool_size)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(event_manager != MR_NULL);
	MR_ASSERT(queue_pool != MR_NULL);
	MR_ASSERT(queue_pool_size >= sizeof(mr_uint16_t));

	/* Add the object to the container */
	ret = mr_object_add_to_container(&event_manager->object, name, MR_CONTAINER_TYPE_EVENT);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	event_manager->type = type;
	mr_list_init(&event_manager->list);
	mr_ringbuffer_init(&event_manager->queue, queue_pool, queue_pool_size);

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_remove_from_container(mr_event_manager_t event_manager)
{
	mr_err_t ret = MR_ERR_OK;
	mr_size_t length = 0;

	MR_ASSERT(event_manager != MR_NULL);

	length = mr_list_get_length(&event_manager->list);
	if (length != 0)
		return - MR_ERR_BUSY;

	/* Remove the object from the container */
	ret = mr_object_remove_from_container(&event_manager->object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Reset the private fields */
	mr_list_init(&event_manager->list);
	mr_ringbuffer_reset(&event_manager->queue);

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_notify(mr_event_manager_t event_manager, mr_uint16_t value)
{
	mr_size_t send_size = 0;

	MR_ASSERT(event_manager != MR_NULL);

	send_size = mr_ringbuffer_write(&event_manager->queue, (mr_uint8_t *)&value, sizeof(value));
	if (send_size != sizeof(value))
		return - MR_ERR_BUSY;

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_process(mr_event_manager_t event_manager)
{
	mr_list_t list = MR_NULL;
	mr_event_t event = MR_NULL;
	mr_uint16_t value = 0;

	MR_ASSERT(event_manager != MR_NULL);

	while (mr_ringbuffer_get_data_length(&event_manager->queue) >= 2)
	{
		mr_ringbuffer_read(&event_manager->queue, (mr_uint8_t *)&value, 2);

		/* Walk through the event-manager looking for event */
		for (list = (event_manager->list).next; list != &(event_manager->list); list = list->next)
		{
			event = mr_container_of(list, struct mr_event, list);
			if (event->value == value)
			{
				event->callback(event_manager, event->args);
				break;
			}
		}
	}

	return MR_ERR_OK;
}

mr_event_t mr_event_find(mr_event_manager_t event_manager, mr_uint16_t event_value)
{
	mr_list_t list = MR_NULL;
	mr_event_t event = MR_NULL;

	MR_ASSERT(event_manager != MR_NULL);

	mr_hw_interrupt_disable();

	/* Walk through the event-manager looking for event */
	for (list = (event_manager->list).next; list != &(event_manager->list); list = list->next)
	{
		event = mr_container_of(list, struct mr_event, list);
		if (event->value == event_value)
		{
			mr_hw_interrupt_enable();
			return event;
		}
	}

	mr_hw_interrupt_enable();
	return MR_NULL;
}

mr_err_t mr_event_add_to_manager(mr_event_manager_t event_manager,
								 mr_event_t event,
								 mr_uint16_t value,
								 mr_err_t (*callback)(mr_event_manager_t event_manager, void *args), void *args)
{
	MR_ASSERT(event_manager != MR_NULL);
	MR_ASSERT(event != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	/* Check if the object already exists in the container */
	if (mr_event_find(event_manager, value) != MR_NULL)
		return - MR_ERR_GENERIC;

	event->value = value;
	event->callback = callback;
	event->args = args;

	mr_hw_interrupt_disable();

	/* Insert the event into the event-manager's list */
	mr_list_insert_after(&(event_manager->list), &(event->list));

	mr_hw_interrupt_enable();
	return MR_ERR_OK;
}

mr_err_t mr_event_remove_from_manager(mr_event_t event)
{
	MR_ASSERT(event != MR_NULL);

	mr_hw_interrupt_disable();

	/* Remove the event from the event-manager's list */
	mr_list_remove(&(event->list));

	mr_hw_interrupt_enable();
	return MR_ERR_OK;
}

mr_err_t mr_event_create_to_manager(mr_event_manager_t event_manager,
									mr_uint16_t value,
									mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
									void *args)
{
	mr_event_t event = MR_NULL;

	MR_ASSERT(event_manager != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	/* Check if the object already exists in the container */
	if (mr_event_find(event_manager, value) != MR_NULL)
		return - MR_ERR_GENERIC;

	event = (mr_event_t)mr_malloc(sizeof(struct mr_event));
	if (event == MR_NULL)
		return - MR_ERR_NO_MEMORY;

	event->value = value;
	event->callback = callback;
	event->args = args;

	mr_hw_interrupt_disable();

	/* Insert the event into the event-manager's list */
	mr_list_insert_after(&(event_manager->list), &(event->list));

	mr_hw_interrupt_enable();
	return MR_ERR_OK;
}

mr_err_t mr_event_delete_from_manager(mr_event_t event)
{
	MR_ASSERT(event != MR_NULL);

	mr_hw_interrupt_disable();

	/* Remove the event from the event-manager's list */
	mr_list_remove(&(event->list));

	mr_hw_interrupt_enable();

	mr_free(event);

	return MR_ERR_OK;
}