/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-18     MacRsh       first version
 */

#include "eloop_sp.h"

#if (MR_CFG_ELOOP_SP == MR_CFG_ENABLE)

#define DEBUG_TAG   "eloop_sp"

/**
 * @struct simple eloop event
 */
struct mr_event_sp
{
    mr_err_t (*cb)(mr_eloop_sp_t loop, void *args);                 /* Event callback */
    void *args;                                                     /* Event args */
};
typedef struct mr_event_sp *mr_event_sp_t;                          /* Type for event */

/**
 * @brief This function finds a simple eloop.
 *
 * @param name The name of the simple eloop.
 *
 * @return A pointer to the found simple eloop, or MR_NULL if not found.
 */
mr_eloop_sp_t mr_eloop_sp_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the simple eloop object from the container */
    return (mr_eloop_sp_t)mr_object_find(name, Mr_Object_Type_Module);
}

/**
 * @brief This function adds a simple eloop to the container.
 *
 * @param eloop The simple eloop to be added.
 * @param name The name of the simple eloop.
 * @param queue_size The size of the queue.
 * @param event_size The size of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 *
 * @note If events are lost, increase the queue size or processing frequency.
 */
mr_err_t mr_eloop_sp_add(mr_eloop_sp_t eloop, const char *name, mr_size_t queue_size, mr_size_t event_size)
{
    mr_uint8_t *queue_mem = MR_NULL;
    void *list_mem = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(queue_size != 0);
    MR_ASSERT(event_size != 0 && event_size <= MR_UINT8_MAX);

    queue_mem = mr_malloc(queue_size * sizeof(*queue_mem));
    if (queue_mem == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, MR_ERR_NO_MEMORY);
        return MR_ERR_NO_MEMORY;
    }

    list_mem = mr_malloc(event_size * sizeof(struct mr_event_sp));
    if (list_mem == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, MR_ERR_NO_MEMORY);
        mr_free(queue_mem);
        return MR_ERR_NO_MEMORY;
    }
    mr_memset(list_mem, 0, event_size * sizeof(struct mr_event_sp));

    /* Initialize the private fields */
    mr_rb_init(&eloop->queue, queue_mem, queue_size * sizeof(*queue_mem));
    eloop->list = list_mem;
    eloop->list_size = event_size;

    /* Add the object to the container */
    ret = mr_object_add(&eloop->object, name, Mr_Object_Type_Module);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, ret);
        mr_rb_init(&eloop->queue, MR_NULL, 0);
        eloop->list = MR_NULL;
        eloop->list_size = 0;
        mr_free(queue_mem);
        mr_free(list_mem);
    }

    return ret;
}

/**
 * @brief This function removes a simple eloop from the container.
 *
 * @param eloop The simple eloop to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_sp_remove(mr_eloop_sp_t eloop)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);

    /* Remove the object from the container */
    ret = mr_object_remove(&eloop->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] remove failed: [%d]\r\n", eloop->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    mr_free(eloop->queue.buffer);
    mr_free(eloop->list);
    mr_rb_init(&eloop->queue, MR_NULL, 0);
    eloop->list = MR_NULL;
    eloop->list_size = 0;

    return MR_ERR_OK;
}

/**
 * @brief This function handles events.
 *
 * @param eloop The simple eloop.
 *
 * @note Only events that have already occurred will be handled.
 */
void mr_eloop_sp_handle(mr_eloop_sp_t eloop)
{
    mr_event_sp_t event = (mr_event_sp_t)eloop->list;
    mr_size_t count = 0;
    mr_uint8_t id = 0;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);

    /* Get the number of current events */
    count = mr_rb_get_data_size(&eloop->queue);

    /* Read the event id from the queue */
    while (count != 0)
    {
        /* Read the event id */
        count -= mr_rb_pop(&eloop->queue, &id);

        if (event[id].cb == MR_NULL)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] handle [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_NOT_FOUND);
            continue;
        }

        /* Call the event callback */
        event[id].cb(eloop, event[id].args);
    }
}

/**
 * @brief This function creates an event.
 *
 * @param eloop The simple eloop.
 * @param id The id of the event.
 * @param cb The event callback.
 * @param args The args of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 *
 * @note The event id range is 0 ~ (event_size-1).
 */
mr_err_t mr_eloop_sp_create_event(mr_eloop_sp_t eloop,
                                  mr_uint8_t id,
                                  mr_err_t (*cb)(mr_eloop_sp_t ep, void *args),
                                  void *args)
{
    mr_event_sp_t event = (mr_event_sp_t)eloop->list;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);
    MR_ASSERT(cb != MR_NULL);

    /* Check if the event id is valid */
    if (id >= eloop->list_size)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] create event [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_INVALID);
        return MR_ERR_INVALID;
    }

    /* Check if the event is busy */
    if (event[id].cb != MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] create event [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_BUSY);
        return MR_ERR_BUSY;
    }

    /* Initialize the private fields */
    event[id].cb = cb;
    event[id].args = args;

    return MR_ERR_OK;
}

/**
 * @brief This function deletes an event.
 *
 * @param eloop The simple eloop.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_sp_delete_event(mr_eloop_sp_t eloop, mr_uint8_t id)
{
    mr_event_sp_t event = (mr_event_sp_t)eloop->list;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);

    /* Check if the event id is valid */
    if (id >= eloop->list_size)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] delete event [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_INVALID);
        return MR_ERR_INVALID;
    }

    /* Check if the event is idle */
    if (event[id].cb == MR_NULL)
    {
        return MR_ERR_OK;
    }

    /* Reset the private fields */
    event[id].cb = MR_NULL;
    event[id].args = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function notifies an event.
 *
 * @param eloop The simple eloop.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_sp_notify_event(mr_eloop_sp_t eloop, mr_uint8_t id)
{
    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);

    /* Check if the event id is valid */
    if (id >= eloop->list_size)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] notify event [%d] failed: [%d]\r\n", eloop->object.name, id - MR_ERR_INVALID);
        return MR_ERR_INVALID;
    }

    if (mr_rb_push(&eloop->queue, id) != sizeof(id))
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] notify event [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_BUSY);
        return MR_ERR_BUSY;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function triggers an event.
 *
 * @param eloop The simple eloop.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_sp_trigger_event(mr_eloop_sp_t eloop, mr_uint8_t id)
{
    mr_event_sp_t event = (mr_event_sp_t)eloop->list;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Module);

    /* Check if the event id is valid */
    if (id >= eloop->list_size)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] trigger event [%d] failed: [%d]\r\n", eloop->object.name, id - MR_ERR_INVALID);
        return MR_ERR_INVALID;
    }

    /* Check if the event is idle */
    if (event[id].cb == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] handle [%d] failed: [%d]\r\n", eloop->object.name, id, MR_ERR_NOT_FOUND);
        return MR_ERR_NOT_FOUND;
    }

    /* Call the event callback */
    event[id].cb(eloop, event[id].args);

    return MR_ERR_OK;
}

#endif