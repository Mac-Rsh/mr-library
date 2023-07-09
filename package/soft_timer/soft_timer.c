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
    soft_timer_t timer = NULL;

    SOFT_TIMER_ASSERT(server != NULL);

    for (list = server->list.next; list != &server->list; list = list->next)
    {
        /* Get the timer from the list */
        timer = container_of(list, struct soft_timer, list);

        /* Without timer timeout */
        if ((server->time - timer->timeout) >= UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous timer */
        list = list->prev;

        /* Restart the timer */
        soft_timer_list_remove(&timer->list);
        soft_timer_start(timer);

        /* Call the timer callback */
        timer->cb(timer, timer->args);
    }
}

/**
 * @brief This function adds a soft-timer to the server.
 *
 * @param timer The soft-timer to be added.
 * @param time The time to be set.
 * @param cb The soft-timer callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return SOFT_TIMER_ERR_OK on success, otherwise an error code.
 */
int soft_timer_add(soft_timer_t timer,
                   uint32_t time,
                   int (*cb)(soft_timer_t timer, void *args),
                   void *args,
                   soft_timer_server_t server)
{
    SOFT_TIMER_ASSERT(timer != NULL);
    SOFT_TIMER_ASSERT(time != 0);
    SOFT_TIMER_ASSERT(cb != NULL);
    SOFT_TIMER_ASSERT(server != NULL);

    /* Check timer has not been added */
    if (timer->server != NULL)
    {
        return -SOFT_TIMER_ERR_BUSY;
    }

    /* Initialize the private fields */
    soft_timer_list_init(&timer->list);
    timer->server = server;
    timer->interval = time;
    timer->timeout = 0;
    timer->cb = cb;
    timer->args = args;

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function removes a soft-timer from the server.
 *
 * @param timer The soft-timer to be removed.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_remove(soft_timer_t timer)
{
    SOFT_TIMER_ASSERT(timer != NULL);

    /* Remove the timer from the server's list */
    soft_timer_stop(timer);
    timer->server = NULL;

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function starts a soft-timer.
 *
 * @param timer The soft-timer to be started.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_start(soft_timer_t timer)
{
    soft_timer_list_t list = NULL;
    soft_timer_t be_insert_timer = NULL;

    SOFT_TIMER_ASSERT(timer != NULL);

    /* Check if the timer is already running */
    if (!soft_timer_list_is_empty(&timer->list))
    {
        return SOFT_TIMER_ERR_OK;
    }

    timer->timeout = timer->server->time + timer->interval;

    /* Find the timer to be inserted */
    for (list = timer->server->list.next; list != &timer->server->list; list = list->next)
    {
        be_insert_timer = container_of(list, struct soft_timer, list);
        if (timer->timeout < be_insert_timer->timeout)
        {
            soft_timer_list_insert_before(&be_insert_timer->list, &timer->list);
            break;
        }
    }

    /* The timer to be inserted was not found, insert before server */
    if (soft_timer_list_is_empty(&timer->list))
    {
        soft_timer_list_insert_before(&timer->server->list, &timer->list);
    }

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function stops a soft-timer.
 *
 * @param timer The soft-timer to be stopped.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_stop(soft_timer_t timer)
{
    SOFT_TIMER_ASSERT(timer != NULL);

    /* Check if the timer is already running */
    if (soft_timer_list_is_empty(&timer->list))
    {
        return SOFT_TIMER_ERR_OK;
    }

    /* Remove the timer from the list */
    soft_timer_list_remove(&timer->list);

    return SOFT_TIMER_ERR_OK;
}

/**
 * @brief This function adds a soft-timer to the server, then start.
 *
 * @param timer The soft-timer to be added, then start.
 * @param time The time to be set.
 * @param cb The soft-timer callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return ERR_OK on success, otherwise an error code.
 */
int soft_timer_add_then_start(soft_timer_t timer,
                              uint32_t time,
                              int (*cb)(soft_timer_t client, void *args),
                              void *args,
                              soft_timer_server_t server)
{
    SOFT_TIMER_ASSERT(timer != NULL);
    SOFT_TIMER_ASSERT(server != NULL);

    soft_timer_add(timer, time, cb, args, server);
    return soft_timer_start(timer);
}