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
        MR_DEBUG_D(DEBUG_TAG, "%s add failed: %d\r\n", server->object.name, ret);
        return ret;
    }

    /* Initialize the private fields */
    server->time = 0;
    mr_list_init(&server->run_list);
    server->list = MR_NULL;

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

    /* Remove the object from the container */
    ret = mr_object_remove(&server->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s remove failed: %d\r\n", server->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    server->time = 0;
    mr_list_init(&server->run_list);
    server->list = MR_NULL;

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
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    for (mr_list_t list = server->run_list.next; list != &server->run_list; list = list->next)
    {
        /* Get the timer from the running list */
        mr_soft_timer_t timer = mr_container_of(list, struct mr_soft_timer, run_list);

        /* Without timer timeout */
        if ((server->time - timer->timeout) >= MR_UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous timer */
        list = list->prev;

        /* Restart the timer */
        mr_list_remove(&timer->run_list);
        mr_soft_timer_start(timer->list.value, server);

        /* Call the timer callback */
        timer->cb(server, timer->args);
    }
}

/**
 * @brief This function creates a new soft-timer.
 *
 * @param id The id of the soft-timer.
 * @param timeout The timeout of the soft-timer.
 * @param cb The soft-timer callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_create(mr_uint8_t id,
                              mr_uint32_t timeout,
                              mr_err_t (*cb)(mr_soft_timer_server_t server, void *args),
                              void *args,
                              mr_soft_timer_server_t server)
{
    mr_soft_timer_t timer = MR_NULL;

    MR_ASSERT(timeout != 0);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    /* Check if the timer is already exists in the server */
    if (mr_avl_find(server->list, id) != MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s -> %d create failed: %d\r\n", server->object.name, id, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    /* Allocate the timer */
    timer = (mr_soft_timer_t)mr_malloc(sizeof(struct mr_soft_timer));
    if (timer == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s -> %d create failed: %d\r\n", server->object.name, id, -MR_ERR_NO_MEMORY);
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(timer, 0, sizeof(struct mr_soft_timer));

    /* Initialize the private fields */
    mr_avl_init(&timer->list, id);
    mr_list_init(&timer->run_list);
    timer->interval = timeout;
    timer->timeout = 0;
    timer->cb = cb;
    timer->args = args;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the timer into the server's list */
    mr_avl_insert(&server->list, &timer->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function delete a soft-timer.
 *
 * @param id The id of the soft-timer.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_delete(mr_uint8_t id, mr_soft_timer_server_t server)
{
    mr_soft_timer_t timer = MR_NULL;
    mr_avl_t node = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    /* Find the timer from the server */
    node = mr_avl_find(server->list, id);
    if (node == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s -> %d delete failed: %d\r\n", server->object.name, id, -MR_ERR_NOT_FOUND);
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the timer from the list */
    timer = mr_container_of(node, struct mr_soft_timer, list);

    /* Stop the timer */
    mr_soft_timer_stop(timer->list.value, server);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the timer from the server's list */
    mr_avl_remove(&server->list, &timer->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the timer */
    mr_free(timer);

    return MR_ERR_OK;
}

/**
 * @brief This function starts a soft-timer.
 *
 * @param id The id of the soft-timer.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_start(mr_uint8_t id, mr_soft_timer_server_t server)
{
    mr_soft_timer_t timer = MR_NULL;
    mr_avl_t node = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    /* Find the timer from the server */
    node = mr_avl_find(server->list, id);
    if (node == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s -> %d start failed: %d\r\n", server->object.name, id, -MR_ERR_NOT_FOUND);
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the timer from the list */
    timer = mr_container_of(node, struct mr_soft_timer, list);

    /* Check if the timer is already running */
    if (!mr_list_is_empty(&timer->run_list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Get the next timeout */
    timer->timeout = server->time + timer->interval;

    /* Find the timer to be inserted */
    for (mr_list_t list = server->run_list.next; list != &server->run_list; list = list->next)
    {
        mr_soft_timer_t be_insert_timer = mr_container_of(list, struct mr_soft_timer, run_list);
        if (timer->timeout < be_insert_timer->timeout)
        {
            mr_list_insert_before(&be_insert_timer->run_list, &timer->run_list);
            break;
        }
    }

    /* The timer to be inserted was not found, insert before server */
    if (mr_list_is_empty(&timer->run_list))
    {
        mr_list_insert_before(&server->run_list, &timer->run_list);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function stops a soft-timer.
 *
 * @param id The id of the soft-timer.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_stop(mr_uint8_t id, mr_soft_timer_server_t server)
{
    mr_soft_timer_t timer = MR_NULL;
    mr_avl_t node = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    /* Find the timer from the server */
    node = mr_avl_find(server->list, id);
    if (node == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s -> %d stop failed: %d\r\n", server->object.name, id, -MR_ERR_NOT_FOUND);
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the timer from the list */
    timer = mr_container_of(node, struct mr_soft_timer, list);

    /* Check if the timer is already running */
    if (mr_list_is_empty(&timer->run_list))
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the timer from the list */
    mr_list_remove(&timer->run_list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function adds a soft-timer to the server, then start.
 *
 * @param id The id of the soft-timer.
 * @param timeout The timeout of the soft-timer.
 * @param cb The soft-timer callback function.
 * @param args The arguments of the callback function.
 * @param server The soft-timer server to which the soft-timer belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_create_and_start(mr_uint8_t id,
                                        mr_uint32_t timeout,
                                        mr_err_t (*cb)(mr_soft_timer_server_t server, void *args),
                                        void *args,
                                        mr_soft_timer_server_t server)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(timeout != 0);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->object.type & MR_OBJECT_TYPE_SOFT_TIMER);

    ret = mr_soft_timer_create(id, timeout, cb, args, server);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    return mr_soft_timer_start(id, server);
}

#endif  /* MR_CONF_SOFT_TIMER */