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

#if (MR_CONF_EVENT == MR_CONF_ENABLE)

/**
 * @brief This function find the event server object.
 *
 * @param name The name of the event server.
 *
 * @return A handle to the found event server, or MR_NULL if not found.
 */
mr_event_server_t mr_event_server_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the event server object from the server container */
    return (mr_event_server_t)mr_object_find(name, MR_OBJECT_TYPE_SERVER);
}

/**
 * @brief This function adds a event server to the container.
 *
 * @param server The event server to be added.
 * @param name The name of the event server.
 * @param queue_length The length of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_server_add(mr_event_server_t server, const char *name, mr_size_t queue_length)
{
    mr_err_t ret = MR_ERR_OK;
    mr_uint8_t *pool = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(queue_length > 0 && queue_length < MR_UINT16_MAX);

    /* Allocate the queue memory */
    pool = mr_malloc(queue_length * sizeof(mr_uint8_t));
    if (pool == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }

    /* Add the object to the container */
    ret = mr_object_add(&server->object, name, MR_OBJECT_TYPE_SERVER);
    if (ret != MR_ERR_OK)
    {
        /* Free the queue memory */
        mr_free(pool);
        return ret;
    }

    /* Initialize the private fields */
    mr_fifo_init(&server->queue, pool, queue_length);
    server->list = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function remove a event server object from the container.
 *
 * @param server The event server to be removed.
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
    {
        return ret;
    }

    /* Free the queue memory */
    mr_free(server->queue.buffer);

    /* Reset the private fields */
    server->list = MR_NULL;
    mr_fifo_init(&server->queue, MR_NULL, 0);

    return MR_ERR_OK;
}

/**
 * @brief This function handle the event server.
 *
 * @param server The event server to be handled.
 */
void mr_event_server_handle(mr_event_server_t server)
{
    mr_uint8_t id = 0;
    mr_avl_t node = MR_NULL;
    mr_event_t event = MR_NULL;

    MR_ASSERT(server != MR_NULL);

    /* Read the event id from the queue */
    while (mr_fifo_read(&server->queue, &id, sizeof(id)))
    {
        node = mr_avl_find(server->list, id);
        if (node == MR_NULL)
        {
            continue;
        }

        /* Get the event from the list */
        event = mr_container_of(node, struct mr_event, list);

        /* Call the event callback */
        event->cb(server, event->args);
    }
}

/**
 * @brief This function creates a new event.
 *
 * @param id The id of the event.
 * @param cb The event callback function.
 * @param args The arguments of the callback function.
 * @param server The event server to which the event belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_create(mr_uint8_t id,
                         mr_err_t (*cb)(mr_event_server_t server, void *args),
                         void *args,
                         mr_event_server_t server)
{
    mr_event_t event = MR_NULL;

    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);

    /* Check if the event is already exists in the server */
    if (mr_avl_find(server->list, id) != MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Allocate the event object */
    event = (mr_event_t)mr_malloc(sizeof(struct mr_event));
    if (event == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(event, 0, sizeof(struct mr_event));

    /* Initialize the private fields */
    mr_avl_init(&event->list, id);
    event->cb = cb;
    event->args = args;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the server's list */
    mr_avl_insert(&server->list, &event->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function delete an event.
 *
 * @param id The id of the event.
 * @param server The event server to which the event belongs.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_delete(mr_uint8_t id, mr_event_server_t server)
{
    mr_avl_t node = MR_NULL;
    mr_event_t event = MR_NULL;

    MR_ASSERT(server != MR_NULL);

    /* Find the event from the server */
    node = mr_avl_find(server->list, id);
    if (node == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the event from the list */
    event = mr_container_of(node, struct mr_event, list);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the event from the server's list */
    mr_avl_remove(&server->list, &event->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the event */
    mr_free(event);

    return MR_ERR_OK;
}

/**
 * @brief This function notify the event server to wake up a event.
 *
 * @param id The id of the event to be wake up.
 * @param server The event server to be notified.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_notify(mr_uint8_t id, mr_event_server_t server)
{
    MR_ASSERT(server != MR_NULL);

    /* Write the event id to the queue */
    if (!mr_fifo_write(&server->queue, &id, sizeof(id)))
    {
        return -MR_ERR_NO_MEMORY;
    }

    return MR_ERR_OK;
}

#endif /* MR_CONF_EVENT */