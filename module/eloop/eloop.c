/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-16     MacRsh       first version
 */

#include "eloop.h"

#if (MR_CFG_ELOOP == MR_CFG_ENABLE)

#define DEBUG_TAG   "eloop"

/**
 * @struct eloop event
 */
struct mr_event
{
    struct mr_avl list;                                             /* Event list */

    mr_err_t (*cb)(mr_eloop_t loop, void *args);                    /* Event callback */
    void *args;                                                     /* Event args */
};
typedef struct mr_event *mr_event_t;                                /* Type for event */

/**
 * @brief This function finds a eloop.
 *
 * @param name The name of the eloop.
 *
 * @return A pointer to the found eloop, or MR_NULL if not found.
 */
mr_eloop_t mr_eloop_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the eloop object from the container */
    return (mr_eloop_t)mr_object_find(name, Mr_Object_Type_Eloop);
}

/**
 * @brief This function adds an eloop to the container.
 *
 * @param eloop The eloop to be added.
 * @param name The name of the eloop.
 * @param queue_size The size of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 *
 * @note If events are lost, increase the queue size or processing frequency.
 */
mr_err_t mr_eloop_add(mr_eloop_t eloop, const char *name, mr_size_t queue_size)
{
    mr_uint32_t *mem = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(queue_size != 0);

    mem = mr_malloc(queue_size * sizeof(*mem));
    if (mem == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, -MR_ERR_NO_MEMORY);
        return -MR_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    mr_rb_init(&eloop->queue, mem, queue_size * sizeof(*mem));
    eloop->list = MR_NULL;

    /* Add the object to the container */
    ret = mr_object_add(&eloop->object, name, Mr_Object_Type_Eloop);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, ret);
        mr_rb_init(&eloop->queue, MR_NULL, 0);
        mr_free(mem);
    }

    return ret;
}

/**
 * @brief This function removes an eloop from the container.
 *
 * @param eloop The eloop to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_remove(mr_eloop_t eloop)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);

    /* Remove the object from the container */
    ret = mr_object_remove(&eloop->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] remove failed: [%d]\r\n", eloop->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    mr_free(eloop->queue.buffer);
    mr_rb_init(&eloop->queue, MR_NULL, 0);
    eloop->list = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function handles events.
 *
 * @param eloop The eloop to be handled.
 *
 * @note Only events that have already occurred will be handled.
 */
void mr_eloop_handle(mr_eloop_t eloop)
{
    mr_size_t count = 0;
    mr_uint32_t id = 0;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);

    /* Get the number of current events */
    count = mr_rb_get_data_size(&eloop->queue);

    /* Read the event id from the queue */
    while (count != 0)
    {
        /* Read the event id */
        count -= mr_rb_read(&eloop->queue, &id, sizeof(id));

        /* Find the event */
        mr_event_t event = (mr_event_t)mr_avl_find(eloop->list, id);
        if (event == MR_NULL)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] handle [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_NOT_FOUND);
            continue;
        }

        /* Call the event callback */
        event->cb(eloop, event->args);
    }
}

/**
 * @brief This function creates an event.
 *
 * @param eloop The eloop to be created.
 * @param id The id of the event.
 * @param cb The callback of the event.
 * @param args The args of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_create_event(mr_eloop_t eloop, mr_uint32_t id, mr_err_t (*cb)(mr_eloop_t ep, void *args), void *args)
{
    mr_event_t event = MR_NULL;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);
    MR_ASSERT(cb != MR_NULL);

    /* Check if the event already exists */
    if (mr_avl_find(eloop->list, id) != MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] created event [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    event = (mr_event_t)mr_malloc(sizeof(struct mr_event));
    if (event == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] created event [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_BUSY);
        return -MR_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    mr_avl_init(&event->list, id);
    event->cb = cb;
    event->args = args;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the eloop list */
    mr_avl_insert(&(eloop->list), &(event->list));

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function deletes an event.
 *
 * @param eloop The eloop to be deleted.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_delete_event(mr_eloop_t eloop, mr_uint32_t id)
{
    mr_event_t event = MR_NULL;

    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);

    /* Check if the event already exists */
    event = (mr_event_t)mr_avl_find(eloop->list, id);
    if (event == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] delete event [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_NOT_FOUND);
        return -MR_ERR_NOT_FOUND;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the eloop list */
    mr_avl_remove(&(eloop->list), &(event->list));

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the event memory */
    mr_free(event);

    return MR_ERR_OK;
}

/**
 * @brief This function notifies an event.
 *
 * @param eloop The eloop to be notified.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_notify_event(mr_eloop_t eloop, mr_uint32_t id)
{
    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);

    if (mr_rb_write(&eloop->queue, &id, sizeof(id)) != sizeof(id))
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] notify event [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function triggers an event.
 *
 * @param eloop The eloop to be triggered.
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_eloop_trigger_event(mr_eloop_t eloop, mr_uint32_t id)
{
    MR_ASSERT(eloop != MR_NULL);
    MR_ASSERT(eloop->object.type == Mr_Object_Type_Eloop);

    /* Find the event */
    mr_event_t event = (mr_event_t)mr_avl_find(eloop->list, id);
    if (event == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] trigger event [%d] failed: [%d]\r\n", eloop->object.name, id, -MR_ERR_NOT_FOUND);
        return -MR_ERR_NOT_FOUND;
    }

    /* Call the event callback */
    event->cb(eloop, event->args);

    return MR_ERR_OK;
}

/**
 * @brief This function converts a string to an id.
 *
 * @param string The string to be converted.
 * @param size The size of the string.
 *
 * @return The id of the string.
 */
mr_uint32_t mr_eloop_string_to_id(const char *string, mr_size_t size)
{
    mr_uint32_t id = 2166136261u;

    for (mr_size_t count = 0; count < size; count++)
    {
        id ^= (mr_uint32_t)string[count];
        id *= 16777619u;
    }

    return id;
}

#endif