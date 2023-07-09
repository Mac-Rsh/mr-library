/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-12     MacRsh       first version
 */

#include "mrlib.h"

#if (MR_CONF_SOFT_TIMER == MR_CONF_ENABLE)

/**
 * @brief This function finds a soft-timer server object.
 *
 * @param name The name of the soft-timer server.
 *
 * @return A handle to the found soft-timer server, or MR_NULL if not found.
 */
mr_soft_timer_server_t mr_soft_timer_server_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the soft-timer server object from the server container */
    return (mr_soft_timer_server_t)mr_object_find(name, MR_OBJECT_TYPE_SERVER);
}

/**
 * @brief This function adds a soft-timer server to the container.
 *
 * @param server The soft-timer server to be added.
 * @param name The name of the soft-timer server.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_server_add(mr_soft_timer_server_t server, const char *name)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(name != MR_NULL);

    /* Add the object to the container */
    ret = mr_object_add(&server->object, name, MR_OBJECT_TYPE_SERVER);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Initialize the private fields */
    server->time = 0;
    mr_list_init(&server->list);

    return MR_ERR_OK;
}

/**
 * @brief This function removes a soft-timer server from the container.
 *
 * @param server The soft-timer server to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_server_remove(mr_soft_timer_server_t server)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->list.next != MR_NULL);

    /* Remove the object from the container */
    ret = mr_object_remove(&server->object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Reset the private fields */
    server->time = 0;
    mr_list_init(&server->list);

    return ret;
}

/**
 * @brief This function update the soft-timer server time.
 *
 * @param server The soft-timer server to be updated.
 * @param time The time to be updated.
 */
void mr_soft_timer_server_update(mr_soft_timer_server_t server, mr_uint32_t time)
{
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(time != 0);

    server->time += time;
}

/**
 * @brief This function handle the soft-timer server.
 *
 * @param server The soft-timer server to be handled.
 */
void mr_soft_timer_server_handle(mr_soft_timer_server_t server)
{
    mr_list_t list = MR_NULL;
    mr_soft_timer_client_t client = MR_NULL;

    MR_ASSERT(server != MR_NULL);

    for (list = server->list.next; list != &server->list; list = list->next)
    {
        /* Get the client from the list */
        client = mr_container_of(list, struct mr_soft_timer_client, list);

        /* Without client timeout */
        if ((server->time - client->timeout) >= MR_UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous client */
        list = list->prev;

        /* Restart the client */
        mr_list_remove(&client->list);
        mr_soft_timer_client_start(client);

        /* Call the client callback */
        client->cb(client, client->args);
    }
}

/**
 * @brief This function adds a soft-timer client to the server.
 *
 * @param client The soft-timer client to be added.
 * @param time The time to be set.
 * @param cb The soft-timer client callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer client belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_client_add(mr_soft_timer_client_t client,
                                  mr_uint32_t time,
                                  mr_err_t (*cb)(mr_soft_timer_client_t client, void *args),
                                  void *args,
                                  mr_soft_timer_server_t server)
{
    MR_ASSERT(client != MR_NULL);
    MR_ASSERT(time != 0);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);

    /* Check client has not been added */
    if (client->server != MR_NULL)
    {
        return -MR_ERR_BUSY;
    }

    /* Initialize the private fields */
    mr_list_init(&client->list);
    client->server = server;
    client->interval = time;
    client->timeout = 0;
    client->cb = cb;
    client->args = args;

    return MR_ERR_OK;
}

/**
 * @brief This function removes a soft-timer client from the server.
 *
 * @param client The soft-timer client to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_client_remove(mr_soft_timer_client_t client)
{
    MR_ASSERT(client != MR_NULL);

    /* Remove the client from the server's list */
    mr_soft_timer_client_stop(client);
    client->server = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function starts a soft-timer client.
 *
 * @param client The soft-timer client to be started.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_client_start(mr_soft_timer_client_t client)
{
    mr_list_t list = MR_NULL;
    mr_soft_timer_client_t be_insert_client = MR_NULL;

    MR_ASSERT(client != MR_NULL);

    /* Check if the client is already running */
    if (!mr_list_is_empty(&client->list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    client->timeout = client->server->time + client->interval;

    /* Find the client to be inserted */
    for (list = client->server->list.next; list != &client->server->list; list = list->next)
    {
        be_insert_client = mr_container_of(list, struct mr_soft_timer_client, list);
        if (client->timeout < be_insert_client->timeout)
        {
            mr_list_insert_before(&be_insert_client->list, &client->list);
            break;
        }
    }

    /* The client to be inserted was not found, insert before server */
    if (mr_list_is_empty(&client->list))
    {
        mr_list_insert_before(&client->server->list, &client->list);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function stops a soft-timer client.
 *
 * @param client The soft-timer client to be stopped.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_client_stop(mr_soft_timer_client_t client)
{
    MR_ASSERT(client != MR_NULL);

    /* Check if the client is already running */
    if (mr_list_is_empty(&client->list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the client from the list */
    mr_list_remove(&client->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function adds a soft-timer client to the server, then starts the soft-timer client.
 *
 * @param client The soft-timer client to be added, then started.
 * @param time The time to be set.
 * @param cb The soft-timer client callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer client belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_client_add_then_start(mr_soft_timer_client_t client,
                                             mr_uint32_t time,
                                             mr_err_t (*cb)(mr_soft_timer_client_t client, void *args),
                                             void *args,
                                             mr_soft_timer_server_t server)
{
    MR_ASSERT(client != MR_NULL);
    MR_ASSERT(server != MR_NULL);

    mr_soft_timer_client_add(client, time, cb, args, server);
    return mr_soft_timer_client_start(client);
}

#endif /* MR_CONF_SOFT_TIMER */