/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include <mrlib.h>

#undef LOG_TAG
#define LOG_TAG "manager"

mr_manager_t mr_manager_find(const char *name)
{
	MR_ASSERT(name != MR_NULL);

	/* Find the manager object from the manager-container */
	return (mr_manager_t)mr_object_find(name, MR_CONTAINER_TYPE_MANAGER);
}

mr_err_t mr_manager_add(mr_manager_t manager,
						const char *name,
						mr_err_t (*err_cb)(struct mr_manager *manager, mr_uint32_t agent_id, mr_err_t err),
						enum mr_manager_type type)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);

	/* Add the object to the container */
	ret = mr_object_add(&manager->object, name, MR_CONTAINER_TYPE_MANAGER);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	manager->type = type;
	manager->ref_count = 0;
	manager->avl = MR_NULL;
	manager->err_cb = err_cb;
	mr_fifo_init(&manager->queue, manager->pool, mr_align_down(sizeof(manager->pool), 4));

	return MR_ERR_OK;
}

mr_err_t mr_manager_remove(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);

	/* Check the manager list for events */
	if (manager->avl != MR_NULL)
		return - MR_ERR_BUSY;

	/* Remove the object from the container */
	ret = mr_object_remove(&manager->object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Reset the private fields */
	manager->ref_count = 0;
	manager->avl = MR_NULL;
	manager->err_cb = MR_NULL;
	mr_fifo_reset(&manager->queue);

	return MR_ERR_OK;
}

mr_err_t mr_manager_notify(mr_manager_t manager, mr_uint32_t agent_id)
{
	mr_size_t send_size = 0;

	send_size = mr_fifo_write(&manager->queue, (mr_uint8_t *)&agent_id, sizeof(agent_id));
	if (send_size != sizeof(agent_id))
	{
		MR_LOG_D(LOG_TAG, "Manager queue is full, failed notify agent %d\r\n", agent_id);
		return - MR_ERR_BUSY;
	}

	return MR_ERR_OK;
}

void mr_manager_handler(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;
	mr_avl_t node = MR_NULL;
	mr_agent_t agent = MR_NULL;
	mr_uint32_t agent_id = 0;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_length(&manager->queue) >= sizeof(agent_id))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&agent_id, sizeof(agent_id));

		node = mr_avl_find(manager->avl, agent_id);
		if (node == MR_NULL)
		{
			MR_LOG_D(LOG_TAG, "Agent %d not found\r\n", agent_id);
			continue;
		}

		agent = mr_struct_of(node, struct mr_agent, avl);

		/* Increase the reference count */
		manager->ref_count ++;
		MR_LOG_D(LOG_TAG, "Agent %d occurred, ref-count %d\r\n", agent_id, manager->ref_count);

		/* Call the agent callback */
		agent->ref_count ++;
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_D(LOG_TAG, "Agent %d failed, error %d\r\n", agent_id, ret);
			if (manager->err_cb != MR_NULL)
				manager->err_cb(manager, agent_id, ret);
		}
	}
}

mr_agent_t mr_agent_find(mr_uint32_t agent_id, mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	return (mr_agent_t)mr_avl_find(manager->avl, agent_id);
}

mr_err_t mr_agent_create(mr_uint32_t agent_id,
						 mr_err_t (*callback)(mr_manager_t manager, void *args),
						 void *args,
						 mr_manager_t agent_manager)
{
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(callback != MR_NULL);
	MR_ASSERT(agent_manager != MR_NULL);

	/* Check if the agent is already exists in the manager */
	if (mr_avl_find(agent_manager->avl, agent_id) != MR_NULL)
		return - MR_ERR_GENERIC;

	/* Allocate the agent object */
	agent = (mr_agent_t)mr_malloc(sizeof(struct mr_agent));
	if (agent == MR_NULL)
		return - MR_ERR_NO_MEMORY;

	/* Initialize the private fields */
	mr_avl_init(&agent->avl, agent_id);
	agent->ref_count = 0;
	agent->cb = callback;
	agent->args = args;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Insert the agent into the manager's list */
	mr_avl_insert(&agent_manager->avl, &agent->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return MR_ERR_OK;
}

mr_err_t mr_agent_delete(mr_uint32_t agent_id, mr_manager_t agent_manager)
{
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(agent_manager != MR_NULL);

	/* Find the agent from the manager */
	agent = mr_agent_find(agent_id, agent_manager);
	if (agent == MR_NULL)
		return - MR_ERR_NOT_FOUND;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Remove the agent from the manager's list */
	mr_avl_remove(&agent_manager->avl, &agent->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	/* Free the agent object */
	mr_free(agent);

	return MR_ERR_OK;
}