/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-16     MacRsh       first version
 */

#include "etask.h"

#if (MR_CFG_ETASK == MR_CFG_ENABLE)

#define DEBUG_TAG   "etask"

/**
 * @struct etask event
 */
struct mr_event
{
    struct mr_avl list;
    struct mr_list tlist;
    union
    {
        struct
        {
            mr_uint32_t timer: 1;
            mr_uint32_t hard: 1;
            mr_uint32_t oneshot: 1;
        };
        mr_uint32_t _sflags: 3;
    } sflags;
    mr_uint32_t interval: 29;
    mr_uint32_t timeout;

    mr_err_t (*cb)(mr_etask_t loop, void *args);
    void *args;
};
typedef struct mr_event *mr_event_t;

void mr_etask_timing(mr_etask_t etask, mr_event_t event, mr_uint32_t time)
{
    mr_list_t list = MR_NULL;

    if (event->sflags.timer == MR_DISABLE)
    {
        return;
    }

    if (time == 0)
    {
        /* Disable interrupt */
        mr_interrupt_disable();

        /* Insert the event into the etask list */
        mr_avl_remove(&etask->list, &event->list);
        if (mr_list_is_empty(&etask->tlist) == MR_FALSE)
        {
            mr_list_remove(&event->tlist);
        }

        /* Enable interrupt */
        mr_interrupt_enable();

        /* Free the event */
        mr_free(event);
        return;
    }

    event->interval = (event->sflags.oneshot == MR_ENABLE) ? 0 : time;
    event->timeout = etask->tick + time;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask timer-list */
    for (list = etask->tlist.next; list != &etask->tlist; list = list->next)
    {
        mr_event_t be_insert = (mr_event_t)mr_container_of(list, struct mr_event, tlist);

        if (event->timeout < be_insert->timeout)
        {
            mr_list_insert_before(&be_insert->tlist, &event->tlist);
            break;
        }
    }
    if (mr_list_is_empty(&event->tlist) == MR_TRUE)
    {
        mr_list_insert_before(&etask->tlist, &event->tlist);
    }

    /* Enable interrupt */
    mr_interrupt_enable();
}

static void mr_etask_free_event(mr_avl_t tree)
{
    if (tree->left_child != MR_NULL)
    {
        mr_etask_free_event(tree->left_child);
        tree->left_child = MR_NULL;
    }

    if (tree->right_child != MR_NULL)
    {
        mr_etask_free_event(tree->right_child);
        tree->right_child = MR_NULL;
    }

    mr_event_t event = (mr_event_t)mr_container_of(tree, struct mr_event, list);
    mr_free(event);
}

/**
 * @brief This function finds a etask.
 *
 * @param name The name of the etask.
 *
 * @return A pointer to the found etask, or MR_NULL if not found.
 */
mr_etask_t mr_etask_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the etask object from the container */
    return (mr_etask_t)mr_object_find(name, Mr_Object_Type_Module);
}

/**
 * @brief This function adds an etask to the container.
 *
 * @param etask The etask to be added.
 * @param name The name of the etask.
 * @param size The size of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 *
 * @note If events are lost, increase the queue size or processing frequency.
 */
mr_err_t mr_etask_add(mr_etask_t etask, const char *name, mr_size_t size)
{
    void *pool = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.magic != MR_OBJECT_MAGIC);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(size > 0);

    pool = mr_malloc(size * sizeof(mr_uint32_t));
    if (pool == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, MR_ERR_NO_MEMORY);
        return MR_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    etask->tick = 0;
    mr_rb_init(&etask->queue, pool, size * sizeof(mr_uint32_t));
    etask->list = MR_NULL;
    mr_list_init(&etask->tlist);
    etask->state = MR_NULL;

    /* Add the object to the container */
    ret = mr_object_add(&etask->object, name, Mr_Object_Type_Module);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, ret);
        mr_free(pool);
    }

    return ret;
}

/**
 * @brief This function removes an etask from the container.
 *
 * @param etask The etask to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_etask_remove(mr_etask_t etask)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.type == Mr_Object_Type_Module);

    /* Remove the object from the container */
    ret = mr_object_remove(&etask->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] remove failed: [%d]\r\n", etask->object.name, ret);
        return ret;
    }

    /* Free the queue */
    mr_free(etask->queue.buffer);
    mr_rb_init(&etask->queue, MR_NULL, 0);
    mr_etask_free_event(etask->list);
    etask->list = MR_NULL;
    mr_list_init(&etask->tlist);
    etask->state = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function update ticks the etask.
 *
 * @param etask The etask to be updated.
 */
void mr_etask_tick_update(mr_etask_t etask)
{
    mr_list_t list = MR_NULL;

    MR_ASSERT(etask != MR_NULL);

    etask->tick++;

    for (list = etask->tlist.next; list != &etask->tlist; list = list->next)
    {
        mr_event_t event = (mr_event_t)mr_container_of(list, struct mr_event, tlist);

        /* Check whether the current tick is larger than the timeout */
        if ((etask->tick - event->timeout) >= MR_UINT16_MAX)
        {
            break;
        }

        /* Remove a timeout event */
        list = list->prev;
        mr_list_remove(&event->tlist);

        /* Call the callback */
        if (event->sflags.hard == MR_ENABLE)
        {
            event->cb(etask, event->args);
        } else
        {
            mr_etask_wakeup(etask, event->list.value, MR_ETASK_WFLAG_DELAY);
        }

        /* Update the timing */
        mr_etask_timing(etask, event, event->interval);
    }
}

/**
 * @brief This function handles an task.
 *
 * @param etask The etask to be handled.
 *
 * @note Only deal with what has already happened.
 */
void mr_etask_handle(mr_etask_t etask)
{
    mr_size_t count = 0;
    mr_uint32_t id = 0;

    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.type == Mr_Object_Type_Module);

    /* Get the number of current events */
    count = mr_rb_get_data_size(&etask->queue);

    /* Read the event id from the queue */
    while (count != 0)
    {
        /* Read the event id */
        count -= mr_rb_read(&etask->queue, &id, sizeof(id));

        /* Find the event */
        mr_event_t event = (mr_event_t)mr_avl_find(etask->list, id);
        if (event == MR_NULL)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] handle [%u] failed: [%d]\r\n", etask->object.name, id, MR_ERR_NOT_FOUND);
            continue;
        }

        /* Call the event callback */
        event->cb(etask, event->args);
    }

    if (etask->state != MR_NULL)
    {
        mr_event_t event = (mr_event_t)etask->state;

        /* Call the state callback */
        event->cb(etask, event->args);
    }
}

/**
 * @brief This function starts an event.
 *
 * @param etask The etask to be started.
 * @param id The id of the event.
 * @param sflags The start flags of the event.
 * @param time The time of the event.
 * @param cb The callback of the event.
 * @param args The args of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_etask_start(mr_etask_t etask,
                        mr_uint32_t id,
                        mr_uint8_t sflags,
                        mr_size_t time,
                        mr_err_t (*cb)(mr_etask_t et, void *args),
                        void *args)
{
    mr_event_t event = MR_NULL;

    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.type == Mr_Object_Type_Module);
    MR_ASSERT(((sflags & MR_ETASK_SFLAG_TIMER) != MR_ETASK_SFLAG_TIMER) || time != 0);
    MR_ASSERT(cb != MR_NULL);

    /* Check if the event already exists */
    if (mr_avl_find(etask->list, id) != MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] start [%u] failed: [%d]\r\n", etask->object.name, id, MR_ERR_BUSY);
        return MR_ERR_BUSY;
    }

    /* Allocate the event */
    event = (mr_event_t)mr_malloc(sizeof(struct mr_event));
    if (event == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] start [%u] failed: [%d]\r\n", etask->object.name, id, MR_ERR_BUSY);
        return MR_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    mr_avl_init(&event->list, id);
    mr_list_init(&event->tlist);
    event->sflags._sflags = sflags;
    event->cb = cb;
    event->args = args;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask list */
    mr_avl_insert(&(etask->list), &(event->list));

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Start the timer */
    mr_etask_timing(etask, event, time);

    return MR_ERR_OK;
}

/**
 * @brief This function stops an event.
 *
 * @param etask The etask to be stopped.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_etask_stop(mr_etask_t etask, mr_uint32_t id)
{
    mr_event_t event = MR_NULL;

    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.type == Mr_Object_Type_Module);

    /* Check if the event already exists */
    event = (mr_event_t)mr_avl_find(etask->list, id);
    if (event == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] stop [%u] failed: [%d]\r\n", etask->object.name, id, MR_ERR_NOT_FOUND);
        return MR_ERR_NOT_FOUND;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask list */
    mr_avl_remove(&etask->list, &event->list);
    if (mr_list_is_empty(&etask->tlist) == MR_FALSE)
    {
        mr_list_remove(&event->tlist);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the event */
    mr_free(event);

    return MR_ERR_OK;
}

/**
 * @brief This function wakes up an event.
 *
 * @param etask The etask to be waked.
 * @param id The id of the event.
 * @param wflag The flag of the wakeup.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_etask_wakeup(mr_etask_t etask, mr_uint32_t id, mr_uint8_t wflag)
{
    MR_ASSERT(etask != MR_NULL);
    MR_ASSERT(etask->object.type == Mr_Object_Type_Module);
    MR_ASSERT(wflag == MR_ETASK_WFLAG_DELAY || wflag == MR_ETASK_WFLAG_NOW || wflag == MR_ETASK_WFLAG_STATE);

    if (wflag == MR_ETASK_WFLAG_DELAY)
    {
        if (mr_rb_write(&etask->queue, &id, sizeof(id)) != sizeof(id))
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] wakeup [%u] failed: [%d]\r\n", etask->object.name, id, MR_ERR_BUSY);
            return MR_ERR_BUSY;
        }
    } else
    {
        mr_event_t event = (mr_event_t)mr_avl_find(etask->list, id);
        if (event == MR_NULL)
        {
            MR_DEBUG_D(DEBUG_TAG,
                       "[%s] wakeup [%u] failed: [%d]\r\n",
                       etask->object.name,
                       id,
                       MR_ERR_NOT_FOUND);
            return MR_ERR_NOT_FOUND;
        }

        if (wflag == MR_ETASK_WFLAG_NOW)
        {
            /* Call the event callback */
            event->cb(etask, event->args);
        } else
        {
            /* Set the state machine */
            etask->state = (void *)event;
        }
    }

    return MR_ERR_OK;
}

/**
 * @brief This function converts a string to an id.
 *
 * @param string The string to be converted.
 *
 * @return The id of the string.
 */
mr_uint32_t mr_etask_str2id(const char *string)
{
    mr_uint32_t id = 2166136261u;

    while (*string != '\0')
    {
        id ^= (mr_uint32_t)*string++;
        id *= 16777619u;
    }

    return id;
}

#endif