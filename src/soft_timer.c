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

#if (MR_CFG_SOFT_TIMER == MR_CFG_ENABLE)

#define DEBUG_TAG   "soft_timer"

static mr_err_t err_io_soft_timer_cb(mr_soft_timer_t timer, void *args)
{
    return -MR_ERR_IO;
}

/**
 * @brief This function finds a soft timer.
 * 
 * @param name The name of the soft timer.
 * 
 * @return A handle to the found soft timer, or MR_NULL if not found.
 */
mr_soft_timer_t mr_soft_timer_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the soft timer object from the container */
    return (mr_soft_timer_t)mr_object_find(name, Mr_Object_Type_SoftTimer);
}

/**
 * @brief This function adds a soft timer to the container.
 * 
 * @param timer The soft timer to be added.
 * @param name The name of the soft timer.
 * @param table The table of the soft timer.
 * @param table_size The size of the table.
 * 
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_add(mr_soft_timer_t timer, const char *name, mr_soft_timer_table_t table, mr_size_t table_size)
{
    mr_uint8_t *memory = MR_NULL;
    mr_size_t count = 0;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(table != MR_NULL);
    MR_ASSERT(table_size != 0);

    /* Allocate memory for the soft timer information */
    memory = mr_malloc(table_size * sizeof(struct mr_soft_timer_data));
    if (memory == MR_NULL)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", name, -MR_ERR_NO_MEMORY);
        return -MR_ERR_NO_MEMORY;
    }

    /* Add the object to the container */
    ret = mr_object_add(&timer->object, name, Mr_Object_Type_SoftTimer);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", name, ret);
        mr_free(memory);
        return ret;
    }

    /* Initialize the private fields */
    timer->table = table;
    timer->table_size = table_size;
    timer->current_time = 0;
    mr_list_init(&timer->timeout_list);
    timer->data = (mr_soft_timer_data_t)memory;

    /* Protect the callback function of each timer */
    for (count = 0; count < table_size; count++)
    {
        timer->table[count].cb = timer->table[count].cb ? timer->table[count].cb : err_io_soft_timer_cb;

        /* Initialize the information of each timer  */
        timer->data[count].timeout = 0;
        mr_list_init(&timer->data[count].timeout_list);
    }

    return MR_ERR_OK;
}

/**
 * @brief This function removes a soft timer from the container.
 *
 * @param timer The soft timer to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_remove(mr_soft_timer_t timer)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(timer->object.type == Mr_Object_Type_SoftTimer);

    /* Remove the object from the container */
    ret = mr_object_remove(&timer->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s remove failed: %d\r\n", timer->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    timer->table = MR_NULL;
    timer->table_size = 0;
    timer->current_time = 0;
    mr_list_init(&timer->timeout_list);
    mr_free(timer->data);
    timer->data = MR_NULL;

    return ret;
}

/**
 * @brief This function updates the current time of a soft timer.
 * 
 * @param timer The soft timer to be updated.
 * @param time The time to be updated.
 */
void mr_soft_timer_update(mr_soft_timer_t timer, mr_uint32_t time)
{
    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(timer->object.type == Mr_Object_Type_SoftTimer);
    MR_ASSERT(time != 0 && time < MR_UINT32_MAX / 2);

    timer->current_time += time;
}

/**
 * @brief This function handles the timeout timers of a soft timer.
 * 
 * @param timer The soft timer to be handled.
 */
void mr_soft_timer_handle(mr_soft_timer_t timer)
{
    mr_uint32_t current_time = 0;
    mr_list_t list = MR_NULL;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(timer->object.type == Mr_Object_Type_SoftTimer);

    /* Get the current time */
    current_time = timer->current_time;

    /* Handle the timeout timers */
    for (list = timer->timeout_list.next; list != &timer->timeout_list; list = list->next)
    {
        /* Get the timer data from the list */
        mr_soft_timer_data_t timer_data = mr_container_of(list, struct mr_soft_timer_data, timeout_list);

        /* Timeout or not */
        if ((current_time - timer_data->timeout) >= MR_UINT32_MAX / 2)
        {
            break;
        }

        /* Go back to the previous timer */
        list = list->prev;

        /* Stop the timer */
        mr_list_remove(&timer_data->timeout_list);

        /* Get the index of the timer */
        mr_uint32_t index = ((mr_uint32_t)timer_data - (mr_uint32_t)timer->data) / sizeof(struct mr_soft_timer_data);

        /* Check whether the index is valid */
        if (index >= timer->table_size)
        {
            MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] handle failed: %d\r\n", timer->object.name, index, -MR_ERR_GENERIC);
            continue;
        }

        /* Start the timer if it is a periodic timer */
        if (timer->table[index].type == Mr_Soft_Timer_Type_Period)
        {
            mr_soft_timer_start(timer, index);
        }

        /* Call the timer callback */
        timer->table[index].cb(timer, timer->table[index].args);
    }
}

/**
 * @brief This function starts a soft timer.
 * 
 * @param timer The soft timer to be started.
 * @param index The index of the soft timer.
 * 
 * @return MR_ERR_OK on success, otherwise an error code. 
 */
mr_err_t mr_soft_timer_start(mr_soft_timer_t timer, mr_uint32_t index)
{
    mr_list_t list = MR_NULL;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(timer->object.type == Mr_Object_Type_SoftTimer);

    /* Check whether the index is valid */
    if (index >= timer->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] start failed: %d\r\n", timer->object.name, index, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Check if the timer is already running */
    if (mr_list_is_empty(&timer->data[index].timeout_list) == MR_FALSE)
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Set the next timeout */
    timer->data[index].timeout = timer->current_time + timer->table[index].time;

    /* Insert to the front if the timeout time is less than the timer be inserted */
    for (list = timer->timeout_list.next; list != &timer->timeout_list; list = list->next)
    {
        mr_soft_timer_data_t be_insert = mr_container_of(list, struct mr_soft_timer_data, timeout_list);

        /* Compare the timeout */
        if (timer->data[index].timeout < be_insert->timeout)
        {
            mr_list_insert_before(&be_insert->timeout_list, &timer->data[index].timeout_list);
            break;
        }
    }

    /* Insert it behind all the timers */
    if (mr_list_is_empty(&timer->data[index].timeout_list) == MR_TRUE)
    {
        mr_list_insert_before(&timer->timeout_list, &timer->data[index].timeout_list);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function stops a soft timer.
 *
 * @param timer The soft timer to be stopped.
 * @param index The index of the soft timer.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_soft_timer_stop(mr_soft_timer_t timer, mr_uint32_t index)
{
    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(timer->object.type == Mr_Object_Type_SoftTimer);

    /* Check whether the index is valid */
    if (index >= timer->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] stop failed: %d\r\n", timer->object.name, index, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Check if the timer is already running */
    if (mr_list_is_empty(&timer->data[index].timeout_list) == MR_TRUE)
    {
        return MR_ERR_OK;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the timer from the list */
    mr_list_remove(&timer->data[index].timeout_list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function gets the current time of a soft timer.
 *
 * @param timer The soft timer to be gotten.
 *
 * @return Current time of the soft timer.
 */
mr_uint32_t mr_soft_timer_get_time(mr_soft_timer_t timer)
{
    return timer->current_time;
}

#endif /* MR_CFG_SOFT_TIMER */