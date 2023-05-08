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
	return (mr_manager_t)mr_object_find(name, Mr_Container_Type_Manager);
}

mr_err_t mr_manager_add(mr_manager_t manager,
						const char *name,
						enum mr_manager_type type,
						mr_size_t queue_number,
						mr_err_t (*err_cb)(struct mr_manager *manager, mr_uint32_t agent_id, mr_err_t err))
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint8_t *pool = MR_NULL;
	mr_size_t pool_size = 0;
	char *at_buffer = MR_NULL;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);

	/* Allocate memory for the queue, pool_size = queue_number * sizeof(agent_id) */
	pool_size = queue_number * sizeof(mr_uint32_t);
	pool = mr_malloc(pool_size);
	if (pool == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Failed to allocate memory for manager %s\r\n", name);
		return - MR_ERR_NO_MEMORY;
	}

	/* Add the object to the container */
	ret = mr_object_add(&manager->object, name, Mr_Container_Type_Manager);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	manager->type = type;
	manager->ref_count = 0;
	manager->avl = MR_NULL;
	manager->err_cb = err_cb;
	manager->data = MR_NULL;
	mr_fifo_init(&manager->queue, pool, pool_size);

	if (type == Mr_Manager_Type_At)
	{
		/* Allocate memory for the at-buffer, the one at_cmd = state(1byte) + cmd(4byte) + at_buffer(MR_CONF_MANAGER_AT_BUFSZ) */
		at_buffer = mr_malloc(queue_number * (MR_CONF_MANAGER_AT_BUFSZ + 5));
		if (at_buffer == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Failed to allocate memory for at-buffer %s\r\n", name);
			mr_free(manager->queue.buffer);
			return - MR_ERR_NO_MEMORY;
		}
		mr_memset(at_buffer, 0, queue_number * (MR_CONF_MANAGER_AT_BUFSZ + 5));
		manager->data = at_buffer;

		MR_LOG_D(LOG_TAG,
				 "Manager %s added, type %d, queue size %d, at-buffer size %d\r\n",
				 name,
				 type,
				 pool_size,
				 queue_number * (MR_CONF_MANAGER_AT_BUFSZ + 5));
		return MR_ERR_OK;
	}

	MR_LOG_D(LOG_TAG, "Manager %s added, type %d, queue size %d\r\n", name, type, pool_size);

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

	/* Free the memory */
	mr_free(manager->queue.buffer);
	mr_free(manager->data);

	MR_LOG_D(LOG_TAG, "Manager %s removed\r\n", manager->object.name);

	return MR_ERR_OK;
}

mr_err_t mr_manager_notify(mr_manager_t manager, mr_uint32_t agent_id)
{
	mr_size_t send_size = 0;

	send_size = mr_fifo_write(&manager->queue, (mr_uint8_t *)&agent_id, sizeof(agent_id));
	if (send_size < sizeof(agent_id))
	{
		MR_LOG_E(LOG_TAG, "Manager queue is full, failed notify agent %d\r\n", agent_id);
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
	mr_uint32_t fsm_agent_id = 0;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_length(&manager->queue) >= sizeof(agent_id))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&agent_id, sizeof(agent_id));

		node = mr_avl_find(manager->avl, agent_id);
		if (node == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Agent %d not found\r\n", agent_id);
			continue;
		}

		agent = mr_struct_of(node, struct mr_agent, avl);
		fsm_agent_id = agent->avl.value;

		/* Increase the reference count */
		manager->ref_count ++;
		MR_LOG_D(LOG_TAG, "Agent %d occurred, ref-count %d\r\n", agent_id, manager->ref_count);

		/* Call the agent baud */
		agent->ref_count ++;
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_E(LOG_TAG, "Agent %d failed, error %d\r\n", agent_id, ret);
			if (manager->err_cb != MR_NULL)
				manager->err_cb(manager, agent_id, ret);
		}

		/* Empty the AT-buffer */
		if (manager->type == Mr_Manager_Type_At)
		{
			mr_memset(agent->args - 5, 0, 5 + MR_CONF_MANAGER_AT_BUFSZ);
		}
	}

	if (manager->type == Mr_Manager_Type_Fsm)
	{
		mr_manager_notify(manager, fsm_agent_id);
	}
}

void mr_manager_at_isr(mr_manager_t manager, char data)
{
	mr_uint8_t *state = MR_NULL;
	mr_size_t index = 0, queue_number = 0;
	mr_uint32_t id = 0;
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager->type == Mr_Manager_Type_At);

	/* Check the at-buffer */
	if (manager->data == MR_NULL)
		return;

	/* Find the idle buffer */
	queue_number = mr_fifo_get_size(&manager->queue) / sizeof(mr_uint32_t);
	for (index = 0; index < queue_number; index ++)
	{
		state = manager->data + index * (MR_CONF_MANAGER_AT_BUFSZ + 5);
		if (*state < Mr_Manager_At_State_Handle)
			break;
		state = MR_NULL;
	}
	/* No idle buffer */
	if (state == MR_NULL)
		return;

	switch (*state)
	{
		case Mr_Manager_At_State_None:
		{
			if (data == 'A')
			{
				*state = Mr_Manager_At_State_Start;
			}
			break;
		}

		case Mr_Manager_At_State_Start:
		{
			if (data == 'T')
			{
				*state = Mr_Manager_At_State_Flag;
			} else
			{
				*state = Mr_Manager_At_State_None;
			}
			break;
		}

		case Mr_Manager_At_State_Flag:
		{
			if (data == '+')
			{
				*state = Mr_Manager_At_State_Id;
			} else
			{
				*state = Mr_Manager_At_State_None;
			}
			break;
		}

		case Mr_Manager_At_State_Id:
		{
			/* Find the idle buffer store id */
			for (index = 1; index < 4 + MR_CONF_MANAGER_AT_BUFSZ; index ++)
			{
				if (*(state + index) == 0)
				{
					*(state + index) = data;
					break;
				}
			}

			if (data == '?' || data == '=' || data == ' ' || data == ':')
			{
				/* Store the id */
				id = mr_strhase((char *)(state + 1));
				mr_memcpy(state + 1, &id, sizeof(id));

				/* Release the occupied buffer */
				mr_memset(state + 5, 0, MR_CONF_MANAGER_AT_BUFSZ);
				*state = Mr_Manager_At_State_Stop;
			}
			break;
		}

		case Mr_Manager_At_State_Stop:
		{
			/* Find the idle buffer store arguments */
			for (index = 5; index < 4 + MR_CONF_MANAGER_AT_BUFSZ; index ++)
			{
				if (*(state + index) == 0)
				{
					*(state + index) = data;
					break;
				}
			}

			if (data == '\0')
			{
				/* Get the id from buffer */
				mr_memcpy(&id, state + 1, sizeof(id));

				/* Find the agent in the manager */
				agent = mr_agent_find(id, manager);
				if (agent == MR_NULL)
				{
					/* Release the occupied buffer */
					mr_memset(state, 0, 5 + MR_CONF_MANAGER_AT_BUFSZ);
					*state = Mr_Manager_At_State_None;
					break;
				}

				/* Notify the agent happened */
				agent->args = state + 5;
				mr_manager_notify(manager, id);
				*state = Mr_Manager_At_State_Handle;
			}
			break;
		}

		default:break;
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

	MR_LOG_D(LOG_TAG, "Agent %d created\r\n", agent_id);

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

	MR_LOG_D(LOG_TAG, "Agent %d deleted\r\n", agent_id);

	return MR_ERR_OK;
}