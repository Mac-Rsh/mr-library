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
	mr_event_manager_t manager = MR_NULL;

	MR_ASSERT(name != MR_NULL);

	/* Find the manager object from the device container */
	manager = (mr_event_manager_t)mr_object_find(name, MR_CONTAINER_TYPE_EVENT);

	return manager;
}

mr_err_t mr_event_manager_add_to_container(mr_event_manager_t manager,
										   const char *name,
										   enum mr_event_manager_type type,
										   mr_uint8_t *pool,
										   mr_size_t pool_size)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(pool != MR_NULL);
	MR_ASSERT(pool_size >= sizeof(mr_uint32_t));

	/* Add the object to the container */
	ret = mr_object_add_to_container(&manager->object, name, MR_CONTAINER_TYPE_EVENT);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	manager->type = type;
	manager->avl = MR_NULL;
	mr_fifo_init(&manager->queue, pool, pool_size);

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_remove_from_container(mr_event_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);

	/* Check the manager list for events */
	if (manager->avl != MR_NULL)
		return - MR_ERR_BUSY;

	/* Remove the object from the container */
	ret = mr_object_remove_from_container(&manager->object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Reset the private fields */
	manager->avl = MR_NULL;
	mr_fifo_reset(&manager->queue);

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_notify(mr_event_manager_t manager, mr_uint32_t value)
{
	mr_size_t send_size = 0;

	MR_ASSERT(manager != MR_NULL);

	send_size = mr_fifo_write(&manager->queue, (mr_uint8_t *)&value, sizeof(value));
	if (send_size != sizeof(value))
		return - MR_ERR_BUSY;

	return MR_ERR_OK;
}

mr_err_t mr_event_manager_handler(mr_event_manager_t manager)
{
	mr_avl_t node = MR_NULL;
	mr_event_t event = MR_NULL;
	mr_uint32_t value = 0;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_data_length(&manager->queue) >= sizeof(value))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&value, sizeof(value));

		node = mr_avl_find(manager->avl, value);
		if (node == MR_NULL)
			continue;

		event = mr_struct_of(node, struct mr_event, avl);
		event->callback(manager, event->args);
	}

	return MR_ERR_OK;
}

mr_event_t mr_event_find(mr_event_manager_t manager, mr_uint32_t value)
{
	MR_ASSERT(manager != MR_NULL);

	return (mr_event_t)mr_avl_find(manager->avl, value);
}

mr_err_t mr_event_add_to_manager(mr_event_manager_t manager,
								 mr_event_t event,
								 mr_uint32_t value,
								 mr_err_t (*callback)(mr_event_manager_t event_manager, void *args), void *args)
{
	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(event != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	/* Check if the event already exists in the manager */
	if (mr_event_find(manager, value) != MR_NULL)
		return - MR_ERR_GENERIC;

	mr_avl_init(&event->avl, value);
	event->callback = callback;
	event->args = args;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Insert the event into the manager's list */
	mr_avl_insert(&manager->avl, &event->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return MR_ERR_OK;
}

mr_err_t mr_event_remove_from_manager(mr_event_manager_t manager, mr_event_t event)
{
	MR_ASSERT(event != MR_NULL);

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Remove the event from the manager's list */
	mr_avl_remove(&manager->avl, &event->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return MR_ERR_OK;
}

mr_err_t mr_event_create_to_manager(mr_event_manager_t manager,
									mr_uint32_t value,
									mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
									void *args)
{
	mr_event_t event = MR_NULL;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	/* Check if the event already exists in the manager */
	if (mr_event_find(manager, value) != MR_NULL)
		return - MR_ERR_GENERIC;

	event = (mr_event_t)mr_malloc(sizeof(struct mr_event));
	if (event == MR_NULL)
		return - MR_ERR_NO_MEMORY;

	return mr_event_add_to_manager(manager, event, value, callback, args);
}

mr_err_t mr_event_delete_from_manager(mr_event_manager_t manager, mr_event_t event)
{
	MR_ASSERT(event != MR_NULL);

	/* Remove the event from the manager's list */
	mr_event_remove_from_manager(manager, event);

	mr_free(event);

	return MR_ERR_OK;
}