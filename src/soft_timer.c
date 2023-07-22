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

#define DEBUG_TAG   "soft_timer"

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
    return (mr_soft_timer_server_t)mr_object_find(name, MR_OBJECT_TYPE_SOFT_TIMER);
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
    ret = mr_object_add(&server->object, name, MR_OBJECT_TYPE_SOFT_TIMER);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s add failed: %d.\r\n", server->object.name, ret);
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
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);
    MR_ASSERT(mr_list_is_empty(&server->list));

    /* Remove the object from the container */
    ret = mr_object_remove(&server->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s remove failed: %d.\r\n", server->object.name, ret);
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
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);
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
    mr_soft_timer_t timer = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    for (list = server->list.next; list != &server->list; list = list->next)
    {
        /* Get the timer from the list */
        timer = mr_container_of(list, struct mr_soft_timer, list);

        /* Without timer timeout */
        if ((server->time - timer->timeout) >= MR_UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous timer */
        list = list->prev;

        /* Restart the timer */
        mr_list_remove(&timer->list);
        mr_soft_timer_start(timer);

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
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_add(mr_soft_timer_t timer,
                           mr_uint32_t time,
                           mr_err_t (*cb)(mr_soft_timer_t timer, void *args),
                           void *args,
                           mr_soft_timer_server_t server)
{
    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(time != 0);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    /* Check timer has not been added */
    if (timer->server != MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "soft-timer has been added.\r\n");
        return -MR_ERR_BUSY;
    }

    /* Initialize the private fields */
    mr_list_init(&timer->list);
    timer->server = server;
    timer->interval = time;
    timer->timeout = 0;
    timer->cb = cb;
    timer->args = args;

    return MR_ERR_OK;
}

/**
 * @brief This function removes a soft-timer from the server.
 *
 * @param timer The soft-timer to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_remove(mr_soft_timer_t timer)
{
    MR_ASSERT(timer != MR_NULL);

    /* Remove the timer from the server's list */
    mr_soft_timer_stop(timer);
    timer->server = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function starts a soft-timer.
 *
 * @param timer The soft-timer to be started.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_start(mr_soft_timer_t timer)
{
    mr_list_t list = MR_NULL;
    mr_soft_timer_t be_insert_timer = MR_NULL;

    MR_ASSERT(timer != MR_NULL);

    /* Check if the timer is already running */
    if (!mr_list_is_empty(&timer->list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Get the next timeout */
    timer->timeout = timer->server->time + timer->interval;

    /* Find the timer to be inserted */
    for (list = timer->server->list.next; list != &timer->server->list; list = list->next)
    {
        be_insert_timer = mr_container_of(list, struct mr_soft_timer, list);
        if (timer->timeout < be_insert_timer->timeout)
        {
            mr_list_insert_before(&be_insert_timer->list, &timer->list);
            break;
        }
    }

    /* The timer to be inserted was not found, insert before server */
    if (mr_list_is_empty(&timer->list))
    {
        mr_list_insert_before(&timer->server->list, &timer->list);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function stops a soft-timer.
 *
 * @param timer The soft-timer to be stopped.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_stop(mr_soft_timer_t timer)
{
    MR_ASSERT(timer != MR_NULL);

    /* Check if the timer is already running */
    if (mr_list_is_empty(&timer->list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the timer from the list */
    mr_list_remove(&timer->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
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
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_add_then_start(mr_soft_timer_t timer,
                                      mr_uint32_t time,
                                      mr_err_t (*cb)(mr_soft_timer_t timer, void *args),
                                      void *args,
                                      mr_soft_timer_server_t server)
{
    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    mr_soft_timer_add(timer, time, cb, args, server);
    return mr_soft_timer_start(timer);
}

#endif /* MR_CONF_SOFT_TIMER */