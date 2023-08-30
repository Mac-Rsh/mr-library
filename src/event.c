/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 * 2023-08-16     MacRsh       code refactoring
 */

#include "mrlib.h"

#if (MR_CFG_EVENT == MR_CFG_ENABLE)

#define DEBUG_TAG   "event"

static mr_err_t err_io_event_cb(mr_event_t cb, void *args)
{
    return -MR_ERR_IO;
}

/**
 * @brief This function finds a event.
 *
 * @param name The name of the event.
 *
 * @return A handle to the found event, or MR_NULL if not found.
 */
mr_event_t mr_event_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the event object from the container */
    return (mr_event_t)mr_object_find(name, Mr_Object_Type_Event);
}

/**
 * @brief This function adds an event to the container.
 *
 * @param event The event to be added.
 * @param name The name of the event.
 * @param table The table of the event.
 * @param table_size The size of the table.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_add(mr_event_t event, const char *name, mr_event_table_t table, mr_size_t table_size)
{
    mr_uint8_t *memory = MR_NULL;
    mr_size_t count = 0;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(event != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(table != MR_NULL);
    MR_ASSERT(table_size != 0);

    /* Allocate memory for the event queue */
    memory = mr_malloc(table_size * sizeof(mr_uint32_t));
    if (memory == MR_NULL)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", name, -MR_ERR_NO_MEMORY);
        return -MR_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    event->table = table;
    event->table_size = table_size;
    mr_rb_init(&event->queue, memory, table_size * sizeof(mr_uint32_t));

    /* Protect the callback function of each event */
    for (count = 0; count < event->table_size; count++)
    {
        event->table[count].cb = event->table[count].cb ? event->table[count].cb : err_io_event_cb;
    }

    /* Add the object to the container */
    ret = mr_object_add(&event->object, name, Mr_Object_Type_Event);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", name, ret);
        mr_free(memory);
    }

    return ret;
}

/**
 * @brief This function removes an event from the container.
 *
 * @param event The event to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_remove(mr_event_t event)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(event != MR_NULL);
    MR_ASSERT(event->object.type == Mr_Object_Type_Event);

    /* Remove the object from the container */
    ret = mr_object_remove(&event->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s remove failed: %d\r\n", event->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    event->table = MR_NULL;
    event->table_size = 0;
    mr_free(event->queue.buffer);
    mr_rb_init(&event->queue, MR_NULL, 0);

    return MR_ERR_OK;
}

/**
 * @brief This function handles the events.
 *
 * @param event The event to be handled.
 */
void mr_event_handle(mr_event_t event)
{
    mr_size_t count = 0;
    mr_uint32_t index = 0;

    MR_ASSERT(event != MR_NULL);
    MR_ASSERT(event->object.type == Mr_Object_Type_Event);

    /* Get the number of events */
    count = mr_rb_get_data_size(&event->queue);

    while (count != 0)
    {
        /* Get the index from the queue */
        count -= mr_rb_read(&event->queue, &index, sizeof(index));

        /* Check whether the index is valid */
        if (index >= event->table_size)
        {
            MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] handle failed: %d\r\n", event->object.name, index, -MR_ERR_GENERIC);
            continue;
        }

        /* Call the event callback function */
        event->table[index].cb(event, event->table[index].args);
    }
}

/**
 * @brief This function notifies the event.
 *
 * @param event The event to be notified.
 * @param index The index to be notified.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_event_notify(mr_event_t event, mr_uint32_t index)
{
    mr_size_t count = 0;

    MR_ASSERT(event != MR_NULL);
    MR_ASSERT(event->object.type == Mr_Object_Type_Event);

    /* Check whether the index is valid */
    if (index >= event->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] index failed: %d\r\n", event->object.name, index, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Write the index to the queue */
    count = mr_rb_write(&event->queue, &index, sizeof(index));
    if (count != sizeof(index))
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] index failed: %d\r\n", event->object.name, index, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function triggers the event.
 *
 * @param event The event to be triggered.
 * @param index The index to be triggered.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 *
 * @note This function fires the event and immediately invokes the event callback.
 */
mr_err_t mr_event_trigger(mr_event_t event, mr_uint32_t index)
{
    MR_ASSERT(event != MR_NULL);
    MR_ASSERT(event->object.type == Mr_Object_Type_Event);

    /* Check whether the index is valid */
    if (index >= event->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] toggle failed: %d\r\n", event->object.name, index, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Call the event callback function */
    event->table[index].cb(event, event->table[index].args);

    return MR_ERR_OK;
}

#endif /* MR_CFG_EVENT */