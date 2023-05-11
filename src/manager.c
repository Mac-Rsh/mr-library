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
						enum mr_manager_type type,
						mr_size_t queue_number)
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint8_t *pool = MR_NULL;
	mr_size_t pool_size = 0;
	char *at_buffer = MR_NULL;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);
	MR_ASSERT(queue_number != 0);

	/* Allocate memory for the queue, pool_size = queue_number * sizeof(agent_id) */
	pool_size = queue_number * sizeof(mr_uint32_t);
	pool = mr_malloc(pool_size);
	if (pool == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Failed to allocate memory for manager %s\r\n", name);
		return - MR_ERR_NO_MEMORY;
	}

	/* Add the object to the container */
	ret = mr_object_add(&manager->object, name, MR_CONTAINER_TYPE_MANAGER);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	manager->type = type;
	manager->avl = MR_NULL;
	manager->data = MR_NULL;
	mr_fifo_init(&manager->queue, pool, pool_size);

	if (type == MR_MANAGER_TYPE_AT_PARSER || type == MR_MANAGER_TYPE_CMD_PARSER)
	{
		/* Allocate memory for the parse-buffer, the one cmd = state(1byte) + cmd(4byte) + parse-buffer(MR_CONF_MANAGER_PARSER_BUFSZ) */
		at_buffer = mr_malloc(queue_number * (MR_CONF_MANAGER_PARSER_BUFSZ + 5));
		if (at_buffer == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Failed to allocate memory for parse-buffer %s\r\n", name);
			mr_free(manager->queue.buffer);
			return - MR_ERR_NO_MEMORY;
		}
		mr_memset(at_buffer, 0, queue_number * (MR_CONF_MANAGER_PARSER_BUFSZ + 5));
		manager->data = at_buffer;

		MR_LOG_D(LOG_TAG,
				 "Manager %s added, type %d, queue size %d, parse-buffer size %d\r\n",
				 name,
				 type,
				 pool_size,
				 queue_number * (MR_CONF_MANAGER_PARSER_BUFSZ + 5));
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
	manager->avl = MR_NULL;
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
		MR_LOG_D(LOG_TAG, "Agent %d occurred\r\n", agent_id);

		/* Call the agent baud */
		ret = agent->cb(manager, agent->args);
		if (ret != MR_ERR_OK)
		{
			MR_LOG_E(LOG_TAG, "Agent %d failed, error %d\r\n", agent_id, ret);
		}

		/* Empty the AT-buffer */
		if ((manager->type == MR_MANAGER_TYPE_AT_PARSER || manager->type == MR_MANAGER_TYPE_CMD_PARSER)
			&& agent->args != MR_NULL)
		{
			mr_memset(agent->args - 5, 0, 5 + MR_CONF_MANAGER_PARSER_BUFSZ);
		}
	}

	if (manager->type == MR_MANAGER_TYPE_FSM)
	{
		mr_manager_notify(manager, fsm_agent_id);
	}
}

void mr_manager_at_parser_isr(mr_manager_t manager, char data)
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
	queue_number = mr_fifo_get_size(&manager->queue) / sizeof(mr_uint32_t);
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

mr_size_t mr_manager_at_parser_get_length(void *args)
{
	char *ch = (char *)args;
	mr_size_t length = 1;

	if (ch == MR_NULL)
		return 0;

	while (*ch)
	{
		if (*ch == ',')
			length ++;

		ch ++;
	}

	return length;
}

char *mr_manager_at_parser_get_arg(void *args, mr_size_t number)
{
	char *ch = (char *)args;
	char *arg = ch;

	if (number == 0)
	{
		return args;
	}

	while (*ch)
	{
		if (*ch == ',')
		{
			ch ++;
			number --;
		}

		if (number == 0)
		{
			return arg;
		}

		arg = ch ++;
	}

	if (ch == args + strlen(args))
	{
		return arg;
	}

	return MR_NULL;
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
	agent->cb = callback;
	agent_manager->type != MR_MANAGER_TYPE_AT_PARSER ? agent->args = args : MR_NULL;

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