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

/**
 * @brief This function find the manager object.
 *
 * @param name The name of the manager.
 *
 * @return A handle to the found manager, or MR_NULL if not found.
 */
mr_manager_t mr_manager_find(const char *name)
{
	MR_ASSERT(name != MR_NULL);

	/* Find the manager object from the manager-container */
	return (mr_manager_t)mr_object_find(name, MR_CONTAINER_TYPE_MANAGER);
}

/**
 * @brief This function adds a manager to the container.
 *
 * @param manager The manager to be added.
 * @param name The name of the manager.
 * @param type The type of the manager.
 * @param queue_number The number of the queue.
 * @param ops The operations of the manager.
 * @param data The data of the manager.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_manager_add(mr_manager_t manager,
						const char *name,
						enum mr_manager_type type,
						mr_size_t queue_number,
						struct mr_manager_ops *ops,
						void *data)
{
	mr_err_t ret = MR_ERR_OK;
	static struct mr_manager_ops null_ops = {MR_NULL};
	mr_uint8_t *pool = MR_NULL;
	mr_size_t pool_size = 0;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);
	MR_ASSERT(queue_number != 0);

	/* Allocate memory for the queue, pool_size = queue_number * sizeof(id) */
	pool_size = queue_number * sizeof(mr_uint32_t);
	pool = mr_malloc(pool_size);
	if (pool == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Failed to allocate memory for manager %s\r\n", name);
		return - MR_ERR_NO_MEMORY;
	}

	/* Initialize the private fields */
	manager->type = type;
	manager->avl = MR_NULL;
	manager->data = data;
	mr_fifo_init(&manager->queue, pool, pool_size);

	/* Set operations as null-ops if ops is null */
	manager->ops = (ops == MR_NULL) ? &null_ops : ops;

	/* Call the manager-add function, if provided */
	if (manager->ops->add != MR_NULL)
	{
		ret = manager->ops->add(manager);
		if (ret != MR_ERR_OK)
		{
			mr_fifo_init(&manager->queue, pool, 0);
			mr_free(pool);
			return ret;
		}
	}

	/* Add the object to the container */
	ret = mr_object_add(&manager->object, name, MR_CONTAINER_TYPE_MANAGER);
	if (ret != MR_ERR_OK)
		return ret;

	MR_LOG_D(LOG_TAG, "Manager %s added, type %d\r\n", name, type);

	return MR_ERR_OK;
}

/**
 * @brief This function remove a manager object.
 *
 * @param manager The manager to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_manager_remove(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;
	static struct mr_manager_ops null_ops = {MR_NULL};

	MR_ASSERT(manager != MR_NULL);

	/* Check the manager list for events */
	if (manager->avl != MR_NULL)
		return - MR_ERR_BUSY;

	/* Remove the object from the container */
	ret = mr_object_remove(&manager->object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Reset the private fields */
	manager->avl = MR_NULL;
	mr_fifo_init(&manager->queue, MR_NULL, 0);

	/* Free the memory */
	mr_free(manager->queue.buffer);

	/* Call the manager-remove function, if provided */
	if (manager->ops->remove != MR_NULL)
	{
		ret = manager->ops->remove(manager);
		if (ret != MR_ERR_OK)
			return ret;
	}

	/* Set operations as null-ops */
	manager->ops = &null_ops;

	MR_LOG_D(LOG_TAG, "Manager %s removed\r\n", manager->object.name);

	return MR_ERR_OK;
}

/**
 * @brief This function notify the manager that the agent has occurred.
 *
 * @param manager The manager to be notified.
 *
 * @param id The id of the agent.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_manager_notify(mr_manager_t manager, mr_uint32_t id)
{
	mr_size_t send_size = 0;

	MR_ASSERT(manager != MR_NULL);

	send_size = mr_fifo_write(&manager->queue, (mr_uint8_t *)&id, sizeof(id));
	if (send_size < sizeof(id))
	{
		MR_LOG_E(LOG_TAG, "Manager %s queue is full, failed notify agent %d\r\n", manager->object.name, id);
		return - MR_ERR_BUSY;
	}

	return MR_ERR_OK;
}

/**
 * @brief This function handle the manager events.
 *
 * @param manager The manager to be handled.
 */
void mr_manager_handler(mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	if (manager->ops->handler != MR_NULL)
		manager->ops->handler(manager);
}

/**
 * @brief This function finds a agent.
 *
 * @param id The id of the agent.
 * @param manager The target manager.
 *
 * @return A handle to the found agent, or MR_NULL if not found.
 */
mr_agent_t mr_agent_find(mr_uint32_t id, mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	return (mr_agent_t)mr_avl_find(manager->avl, id);
}

/**
 * @brief This function creates a agent to the manager.
 *
 * @param id The id of the agent.
 * @param callback The callback function.
 * @param args The arguments of the callback function.
 * @param target_manager The target manager.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_agent_create(mr_uint32_t id,
						 mr_err_t (*callback)(mr_manager_t manager, void *args),
						 void *args,
						 mr_manager_t target_manager)
{
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(callback != MR_NULL);
	MR_ASSERT(target_manager != MR_NULL);

	/* Check if the agent is already exists in the manager */
	if (mr_avl_find(target_manager->avl, id) != MR_NULL)
		return - MR_ERR_GENERIC;

	/* Allocate the agent object */
	agent = (mr_agent_t)mr_malloc(sizeof(struct mr_agent));
	if (agent == MR_NULL)
		return - MR_ERR_NO_MEMORY;

	/* Initialize the private fields */
	mr_avl_init(&agent->avl, id);
	agent->cb = callback;
	agent->args = args;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Insert the agent into the manager's list */
	mr_avl_insert(&target_manager->avl, &agent->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	MR_LOG_D(LOG_TAG, "Manager %s, agent %d created\r\n", target_manager->object.name, id);

	return MR_ERR_OK;
}

/**
 * @brief This function delete an agent.
 *
 * @param id The id of the agent.
 * @param manager The target manager.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_agent_delete(mr_uint32_t id, mr_manager_t manager)
{
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager != MR_NULL);

	/* Find the agent from the manager */
	agent = mr_agent_find(id, manager);
	if (agent == MR_NULL)
		return - MR_ERR_NOT_FOUND;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Remove the agent from the manager's list */
	mr_avl_remove(&manager->avl, &agent->avl);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	/* Free the agent object */
	mr_free(agent);

	MR_LOG_D(LOG_TAG, "Manager %s, agent %d deleted\r\n", manager->object.name, id);

	return MR_ERR_OK;
}

#undef LOG_TAG
#define LOG_TAG "event"

static void event_handler(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint32_t id = 0;
	mr_avl_t node = MR_NULL;
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_data_size(&manager->queue) >= sizeof(id))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&id, sizeof(id));

		node = mr_avl_find(manager->avl, id);
		if (node == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, event %d not found\r\n", manager->object.name, id);
			continue;
		}

		agent = mr_struct_of(node, struct mr_agent, avl);

		/* Increase the reference count */
		MR_LOG_D(LOG_TAG, "Manager %s, event %d occurred\r\n", manager->object.name, id);

		/* Call the agent baud */
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, event %d failed, error %d\r\n", manager->object.name, id, ret);
		}
	}
}

/**
 * @brief This function adds an event-manager to the container.
 *
 * @param manager The manager to be added.
 * @param name The name of the manager.
 * @param queue_number The number of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_manager_add(mr_manager_t manager, const char *name, mr_size_t queue_number)
{
	static struct mr_manager_ops manager_ops =
		{
			MR_NULL,
			MR_NULL,
			event_handler,
		};

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(manager != MR_NULL);

	return mr_manager_add(manager, name, MR_MANAGER_TYPE_EVENT, queue_number, &manager_ops, MR_NULL);
}

mr_err_t mr_event_create(mr_uint32_t event,
						 mr_err_t (*callback)(mr_manager_t manager, void *args),
						 void *args,
						 mr_manager_t target_manager)
{
	MR_ASSERT(callback != MR_NULL);
	MR_ASSERT(target_manager != MR_NULL);

	return mr_agent_create(event, callback, args, target_manager);
}

mr_err_t mr_event_delete(mr_uint32_t event, mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	return mr_agent_delete(event, manager);
}

#undef LOG_TAG
#define LOG_TAG "fsm"

static void fsm_handler(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint32_t id = 0, state = 0;
	mr_avl_t node = MR_NULL;
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_data_size(&manager->queue) >= sizeof(id))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&id, sizeof(id));

		node = mr_avl_find(manager->avl, id);
		if (node == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, fsm %d not found\r\n", manager->object.name, id);
			continue;
		}

		agent = mr_struct_of(node, struct mr_agent, avl);
		state = agent->avl.value;

		/* Increase the reference count */
		MR_LOG_D(LOG_TAG, "Manager %s, fsm %d occurred\r\n", manager->object.name, id);

		/* Call the agent baud */
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, fsm %d failed, error %d\r\n", manager->object.name, id, ret);
		}
	}

	mr_manager_notify(manager, state);
}

mr_err_t mr_fsm_state_create(mr_uint32_t state,
							 mr_err_t (*callback)(mr_manager_t manager, void *args),
							 void *args,
							 mr_manager_t target_manager)
{
	MR_ASSERT(callback != MR_NULL);
	MR_ASSERT(target_manager != MR_NULL);

	return mr_agent_create(state, callback, args, target_manager);
}

mr_err_t mr_fsm_state_delete(mr_uint32_t state, mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	return mr_agent_delete(state, manager);
}

/**
 * @brief This function adds an fsm-manager to the container.
 *
 * @param manager The manager to be added.
 * @param name The name of the manager.
 * @param queue_number The number of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_manager_add(mr_manager_t manager, const char *name, mr_size_t queue_number)
{
	static struct mr_manager_ops manager_ops =
		{
			MR_NULL,
			MR_NULL,
			fsm_handler,
		};

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(manager != MR_NULL);

	return mr_manager_add(manager, name, MR_MANAGER_TYPE_FSM, queue_number, &manager_ops, MR_NULL);
}

#undef LOG_TAG
#define LOG_TAG "at-parser"

mr_err_t at_parser_add(mr_manager_t manager)
{
	char *at_buffer = MR_NULL;
	mr_size_t queue_number = 0;

	MR_ASSERT(manager != MR_NULL);

	queue_number = mr_fifo_get_buffer_size(&manager->queue) / sizeof(mr_uint32_t);

	/* Allocate memory for the parse-buffer, the one cmd = state(1byte) + cmd(4byte) + parse-buffer(MR_CONF_MANAGER_PARSER_BUFSZ) */
	at_buffer = mr_malloc(queue_number * (MR_CONF_MANAGER_PARSER_BUFSZ + 5));
	if (at_buffer == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Failed to allocate memory for at-parser buffer\r\n");
		mr_free(manager->queue.buffer);
		return - MR_ERR_NO_MEMORY;
	}
	mr_memset(at_buffer, 0, queue_number * (MR_CONF_MANAGER_PARSER_BUFSZ + 5));
	manager->data = at_buffer;

	return MR_ERR_OK;
}

mr_err_t at_parser_remove(mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	/* Free the memory */
	mr_free(manager->data);

	return MR_ERR_OK;
}

static void at_parser_handler(mr_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint32_t id = 0;
	mr_avl_t node = MR_NULL;
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager != MR_NULL);

	while (mr_fifo_get_data_size(&manager->queue) >= sizeof(id))
	{
		mr_fifo_read(&manager->queue, (mr_uint8_t *)&id, sizeof(id));

		node = mr_avl_find(manager->avl, id);
		if (node == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, event %d not found\r\n", manager->object.name, id);
			continue;
		}

		agent = mr_struct_of(node, struct mr_agent, avl);

		/* Increase the reference count */
		MR_LOG_D(LOG_TAG, "Manager %s, event %d occurred\r\n", manager->object.name, id);

		/* Call the agent baud */
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_E(LOG_TAG, "Manager %s, event %d failed, error %d\r\n", manager->object.name, id, ret);
		}

		/* Empty the AT-buffer */
		if (agent->args != MR_NULL)
		{
			mr_memset(agent->args - 5, 0, 5 + MR_CONF_MANAGER_PARSER_BUFSZ);
		}
	}
}

mr_err_t mr_at_parser_manager_add(mr_manager_t manager, const char *name, mr_size_t queue_number)
{
	static struct mr_manager_ops manager_ops =
		{
			at_parser_add,
			at_parser_remove,
			at_parser_handler,
		};

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(manager != MR_NULL);

	return mr_manager_add(manager, name, MR_MANAGER_TYPE_AT_PARSER, queue_number, &manager_ops, MR_NULL);
}

mr_err_t mr_at_cmd_create(const char *at_cmd,
						  mr_err_t (*callback)(mr_manager_t manager, void *args),
						  mr_manager_t target_manager)
{
	MR_ASSERT(callback != MR_NULL);
	MR_ASSERT(target_manager != MR_NULL);

	if (mr_strncmp(at_cmd, "AT+", 3) != 0)
		return - MR_ERR_INVALID;

	return mr_agent_create(mr_strhase(at_cmd + 3), callback, MR_NULL, target_manager);
}

mr_err_t mr_at_cmd_delete(mr_uint32_t at_cmd, mr_manager_t manager)
{
	MR_ASSERT(manager != MR_NULL);

	return mr_agent_delete(at_cmd, manager);
}

void mr_at_parser_isr(mr_manager_t manager, char data)
{
	mr_uint8_t *state = MR_NULL;
	mr_size_t index = 0, queue_number = 0;
	mr_uint32_t id = 0;
	mr_agent_t agent = MR_NULL;

	MR_ASSERT(manager->type == MR_MANAGER_TYPE_AT_PARSER);

	/* Check the at-buffer */
	if (manager->data == MR_NULL)
		return;

	/* Find the idle buffer */
	queue_number = mr_fifo_get_buffer_size(&manager->queue) / sizeof(mr_uint32_t);
	for (index = 0; index < queue_number; index ++)
	{
		state = manager->data + index * (MR_CONF_MANAGER_PARSER_BUFSZ + 5);
		if (*state < MR_MANAGER_AT_STATE_HANDLE)
			break;
	}
	/* No idle buffer */
	if (index == queue_number)
		return;

	switch (*state)
	{
		case MR_MANAGER_AT_STATE_NONE:
		{
			if (data == 'A')
				*state = MR_MANAGER_AT_STATE_START;
			break;
		}

		case MR_MANAGER_AT_STATE_START:
		{
			if (data == 'T')
				*state = MR_MANAGER_AT_STATE_FLAG;
			else
				*state = MR_MANAGER_AT_STATE_NONE;
			break;
		}

		case MR_MANAGER_AT_STATE_FLAG:
		{
			if (data == '+')
				*state = MR_MANAGER_AT_STATE_ID;
			else
				*state = MR_MANAGER_AT_STATE_NONE;
			break;
		}

		case MR_MANAGER_AT_STATE_ID:
		{
			if (data == '\r' || data == '\n' || data == ';' || data == '\0')
			{
				*state = MR_MANAGER_AT_STATE_STOP;
				break;
			}

			/* Find the idle buffer store id */
			for (index = 1; index < 4 + MR_CONF_MANAGER_PARSER_BUFSZ; index ++)
			{
				if (*(state + index) == 0)
				{
					*(state + index) = data;
					break;
				}
			}

			if (data == '?')
				*state = MR_MANAGER_AT_STATE_STOP;

			if (data == '=')
				*state = MR_MANAGER_AT_STATE_CHECK;

			break;
		}

		case MR_MANAGER_AT_STATE_CHECK:
		{
			if (data == '?')
			{
				/* Find the idle buffer store id */
				for (index = 1; index < 4 + MR_CONF_MANAGER_PARSER_BUFSZ; index ++)
				{
					if (*(state + index) == 0)
					{
						*(state + index) = data;
						break;
					}
				}

				*state = MR_MANAGER_AT_STATE_STOP;
			} else
			{
				/* Store the id */
				id = mr_strhase((char *)(state + 1));
				mr_memcpy(state + 1, &id, sizeof(id));

				/* Release the occupied buffer */
				mr_memset(state + 5, 0, MR_CONF_MANAGER_PARSER_BUFSZ);

				/* Find the idle buffer store arguments */
				for (index = 5; index < 4 + MR_CONF_MANAGER_PARSER_BUFSZ; index ++)
				{
					if (*(state + index) == 0)
					{
						*(state + index) = data;
						break;
					}
				}

				*state = MR_MANAGER_AT_STATE_ARGS;
			}
			break;
		}

		case MR_MANAGER_AT_STATE_ARGS:
		{
			if (data == '\r' || data == '\n' || data == ';' || data == '\0')
			{
				/* Get the id from buffer */
				mr_memcpy(&id, state + 1, sizeof(id));

				/* Find the agent in the manager */
				agent = mr_agent_find(id, manager);
				if (agent == MR_NULL)
				{
					/* Release the occupied buffer */
					mr_memset(state, 0, 5 + MR_CONF_MANAGER_PARSER_BUFSZ);

					*state = MR_MANAGER_AT_STATE_NONE;
					break;
				}

				/* Notify the agent happened */
				agent->args = state + 5;
				mr_manager_notify(manager, id);

				*state = MR_MANAGER_AT_STATE_HANDLE;
				break;
			}

			/* Find the idle buffer store arguments */
			for (index = 5; index < 4 + MR_CONF_MANAGER_PARSER_BUFSZ; index ++)
			{
				if (*(state + index) == 0)
				{
					*(state + index) = data;
					break;
				}
			}
			break;
		}

		case MR_MANAGER_AT_STATE_STOP:
		{
			/* Store the id */
			id = mr_strhase((char *)(state + 1));
			mr_memcpy(state + 1, &id, sizeof(id));

			/* Release the occupied buffer */
			mr_memset(state + 5, 0, MR_CONF_MANAGER_PARSER_BUFSZ);

			/* Notify the agent happened */
			mr_manager_notify(manager, id);
			mr_memset(state, 0, 5);

			*state = MR_MANAGER_AT_STATE_NONE;
			break;
		}

		default:break;
	}
}