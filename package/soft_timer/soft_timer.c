/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-03     MacRsh       first version
 */

#include "soft_timer.h"

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

static void soft_timer_list_init(soft_timer_list_t list)
{
    list->next = list;
    list->prev = list;
}

static void soft_timer_list_remove(soft_timer_list_t node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;

    node->next = node->prev = node;
}

static void soft_timer_list_insert_before(soft_timer_list_t list, soft_timer_list_t node)
{
    list->prev->next = node;
    node->prev = list->prev;

    list->prev = node;
    node->next = list;
}

static int soft_timer_list_is_empty(soft_timer_list_t list)
{
    return (int)(list->next == list);
}

/**
 * @brief This function init a soft-timer server.
 *
 * @param server The soft-timer server to be inited.
 */
void soft_timer_server_init(soft_timer_server_t server)
{
    SOFT_TIMER_ASSERT(server != NULL);

    server->time = 0;
    soft_timer_list_init(&server->list);
}

/**
 * @brief This function update the soft-timer server time.
 *
 * @param server The soft-timer server to be updated.
 * @param time The time to be updated.
 */
void soft_timer_server_update(soft_timer_server_t server, uint32_t time)
{
    SOFT_TIMER_ASSERT(server != NULL);
    SOFT_TIMER_ASSERT(time != 0);

    server->time += time;
}

/**
 * @brief This function handle the soft-timer server.
 *
 * @param server The soft-timer server to be handled.
 */
void soft_timer_server_handle(soft_timer_server_t server)
{
    soft_timer_list_t list = NULL;
    soft_timer_client_t client = NULL;

    SOFT_TIMER_ASSERT(server != NULL);

    for (list = server->list.next; list != &server->list; list = list->next)
    {
        /* Get the client from the list */
        client = container_of(list, struct soft_timer_client, list);

        /* Without client timeout */
        if ((server->time - client->timeout) >= UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous client */
        list = list->prev;

        /* Restart the client */
        soft_timer_list_remove(&client->list);
        soft_timer_client_start(client);

        /* Call the client callback */
        client->cb(client, client->args);
    }
}

static int _err_io_soft_timer_client_cb(soft_timer_client_t client, void *args)
{
    return -SOFT_TIMER_ERR_IO;
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
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_client_add(soft_timer_client_t client,
                          uint32_t time,
                          int (*cb)(soft_timer_client_t client, void *args),
                          void *args,
                          soft_timer_server_t server)
{
    SOFT_TIMER_ASSERT(client != NULL);
    SOFT_TIMER_ASSERT(server != NULL);

    /* Check client has not been added */
    if (client->server != NULL)
    {
        return -SOFT_TIMER_ERR_BUSY;
    }

    /* Initialize the private fields */
    soft_timer_list_init(&client->list);
    client->server = server;
    client->interval = time;
    client->timeout = 0;
    client->cb = cb ? cb : _err_io_soft_timer_client_cb;
    client->args = args;

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function removes a soft-timer client from the server.
 *
 * @param client The soft-timer client to be removed.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_client_remove(soft_timer_client_t client)
{
    SOFT_TIMER_ASSERT(client != NULL);

    /* Remove the client from the server's list */
    soft_timer_client_stop(client);
    client->server = NULL;

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function starts a soft-timer client.
 *
 * @param client The soft-timer client to be started.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_client_start(soft_timer_client_t client)
{
    soft_timer_list_t list = NULL;
    soft_timer_client_t be_insert_client = NULL;

    SOFT_TIMER_ASSERT(client != NULL);

    /* Check if the client is already running */
    if (!soft_timer_list_is_empty(&client->list))
    {
        return SOFT_TIMER_ERR_OK;
    }

    client->timeout = client->server->time + client->interval;

    /* Find the client to be inserted */
    for (list = client->server->list.next; list != &client->server->list; list = list->next)
    {
        be_insert_client = container_of(list, struct soft_timer_client, list);
        if (client->timeout < be_insert_client->timeout)
        {
            soft_timer_list_insert_before(&be_insert_client->list, &client->list);
            break;
        }
    }

    /* The client to be inserted was not found, insert before server */
    if (soft_timer_list_is_empty(&client->list))
    {
        soft_timer_list_insert_before(&client->server->list, &client->list);
    }

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function stops a soft-timer client.
 *
 * @param client The soft-timer client to be stopped.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_client_stop(soft_timer_client_t client)
{
    SOFT_TIMER_ASSERT(client != NULL);

    /* Check if the client is already running */
    if (soft_timer_list_is_empty(&client->list))
    {
        return SOFT_TIMER_ERR_OK;
    }

    /* Remove the client from the list */
    soft_timer_list_remove(&client->list);

    return SOFT_TIMER_ERR_OK;
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
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_client_add_then_start(soft_timer_client_t client,
                                     uint32_t time,
                                     int (*cb)(soft_timer_client_t client, void *args),
                                     void *args,
                                     soft_timer_server_t server)
{
    SOFT_TIMER_ASSERT(client != NULL);
    SOFT_TIMER_ASSERT(server != NULL);

    soft_timer_client_add(client, time, cb, args, server);
    return soft_timer_client_start(client);
}