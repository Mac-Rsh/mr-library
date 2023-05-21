/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 */

#include "mrlib.h"

#undef LOG_TAG
#define LOG_TAG "event"

/**
 * @brief This function find the event-server object.
 *
 * @param name The name of the event-server.
 *
 * @return A handle to the found event-server, or MR_NULL if not found.
 */
mr_event_server_t mr_event_server_find(const char *name)
{
	MR_ASSERT(name != MR_NULL);

	/* Find the event-server object from the server container */
	return (mr_event_server_t)mr_object_find(name, MR_CONTAINER_TYPE_SERVER);
}

/**
 * @brief This function adds a event-server to the container.
 *
 * @param server The event-server to be added.
 * @param name The name of the event-server.
 * @param queue_length The length of the client queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_server_add(mr_event_server_t server, const char *name, mr_size_t queue_length)
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint8_t *pool = MR_NULL;

	MR_ASSERT(server != MR_NULL);
	MR_ASSERT(name != MR_NULL);
	MR_ASSERT(queue_length > 0 && queue_length <= 256);

	pool = mr_malloc(queue_length);
	if (pool == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Server %s allocate memory failed\r\n", name);
		return - MR_ERR_NO_MEMORY;
	}

	/* Add the object to the container */
	ret = mr_object_add(&server->object, name, MR_CONTAINER_TYPE_SERVER);
	if (ret != MR_ERR_OK)
	{
		/* Free the queue memory */
		mr_free(pool);
		return ret;
	}

	/* Initialize the private fields */
	mr_fifo_init(&server->queue, pool, queue_length);
	server->list = MR_NULL;

	MR_LOG_D(LOG_TAG, "%s add, queue length : %d\r\n", name, queue_length);

	return MR_ERR_OK;
}

/**
 * @brief This function remove a event-server object from the container.
 *
 * @param server The event-server to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_server_remove(mr_event_server_t server)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(server != MR_NULL);
	MR_ASSERT(server->list != MR_NULL);

	/* Remove the object from the container */
	ret = mr_object_remove(&server->object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Free the queue memory */
	mr_free(server->queue.buffer);

	/* Reset the private fields */
	server->list = MR_NULL;
	mr_fifo_init(&server->queue, MR_NULL, 0);

	MR_LOG_D(LOG_TAG, "Server %s removed\r\n", server->object.name);

	return MR_ERR_OK;
}

/**
 * @brief This function notify the event-server to wake up a client.
 *
 * @param server The event-server to be notified.
 *
 * @param id The id of the client to be wake up.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_server_notify(mr_event_server_t server, mr_uint8_t id)
{
	MR_ASSERT(server != MR_NULL);

	if (! mr_fifo_write(&server->queue, &id, sizeof(id)))
	{
		MR_LOG_E(LOG_TAG, "Server %s notify client %d failed\r\n", server->object.name, id);
		return - MR_ERR_NO_MEMORY;
	}

	return MR_ERR_OK;
}

/**
 * @brief This function handle the event-server.
 *
 * @param server The event-server to be handled.
 */
void mr_event_server_handle(mr_event_server_t server)
{
	mr_uint8_t id = 0;
	mr_avl_t node = MR_NULL;
	mr_event_client_t client = MR_NULL;

	MR_ASSERT(server != MR_NULL);

	while (mr_fifo_read(&server->queue, &id, sizeof(id)))
	{
		node = mr_avl_find(server->list, id);
		if (node == MR_NULL)
		{
			MR_LOG_E(LOG_TAG, "Server %s not found client %d\r\n", server->object.name, id);
			continue;
		}

		client = mr_struct_of(node, struct mr_event_client, list);

		MR_LOG_D(LOG_TAG, "Server %s wake client %d\r\n", server->object.name, id);

		client->cb(server, client->args);
	}
}

/**
 * @brief This function find the event-client.
 *
 * @param id The id of the event-client.
 * @param server The event-server to which the event-client belongs.
 *
 * @return A handle to the found event-client, or MR_NULL if not found.
 */
mr_event_client_t mr_event_client_find(mr_uint8_t id, mr_event_server_t server)
{
	MR_ASSERT(server != MR_NULL);

	/* Find the event-client from the server */
	return (mr_event_client_t)mr_avl_find(server->list, id);
}

/**
 * @brief This function creates a new event-client.
 *
 * @param id The id of the event-client.
 * @param cb The event-client callback function.
 * @param args The arguments of the callback function.
 * @param server The event-server to which the event-client belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_client_create(mr_uint8_t id,
								mr_err_t (*cb)(mr_event_server_t server, void *args),
								void *args,
								mr_event_server_t server)
{
	mr_event_client_t client = MR_NULL;

	MR_ASSERT(cb != MR_NULL);
	MR_ASSERT(server != MR_NULL);

	/* Check if the client is already exists in the server */
	if (mr_avl_find(server->list, id) != MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Server %s client %d already exists\r\n", server->object.name, id);
		return - MR_ERR_GENERIC;
	}

	/* Allocate the client object */
	client = (mr_event_client_t)mr_malloc(sizeof(struct mr_event_client));
	if (client == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Server %s client %d allocate memory failed\r\n", server->object.name, id);
		return - MR_ERR_NO_MEMORY;
	}

	/* Initialize the private fields */
	client->cb = cb;
	client->args = args;
	mr_avl_init(&client->list, id);

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Insert the agent into the manager's list */
	mr_avl_insert(&server->list, &client->list);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	MR_LOG_D(LOG_TAG, "Server %s client %d created\r\n", server->object.name, id);

	return MR_ERR_OK;
}

/**
 * @brief This function delete an event-client.
 *
 * @param id The id of the event-client.
 * @param server The event-server to which the event-client belongs.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_client_delete(mr_uint8_t id, mr_event_server_t server)
{
	mr_event_client_t client = MR_NULL;

	MR_ASSERT(server != MR_NULL);

	/* Find the event-client from the server */
	client = mr_event_client_find(id, server);
	if (client == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "Server %s client %d not found\r\n", server->object.name, id);
		return - MR_ERR_NOT_FOUND;
	}

	/* Disable interrupt */
	mr_hw_interrupt_disable();


	/* Remove the agent from the manager's list */
	mr_avl_remove(&server->list, &client->list);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	/* Free the client */
	mr_free(client);

	MR_LOG_D(LOG_TAG, "Server %s client %d deleted\r\n", server->object.name, id);

	return MR_ERR_OK;
}