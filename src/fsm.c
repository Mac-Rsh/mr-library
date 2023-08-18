/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-11     MacRsh       first version
 */

#include "mrlib.h"

#if (MR_CFG_FSM == MR_ENABLE)

#define DEBUG_TAG   "fsm"

static mr_err_t err_io_fsm_cb(mr_fsm_t fsm, void *args)
{
    return -MR_ERR_IO;
}

static mr_err_t err_io_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    return -MR_ERR_IO;
}

/**
 * @brief This function finds a state machine.
 *
 * @param name The name of the state machine.
 *
 * @return A handle to the found state machine, or MR_NULL if not found.
 */
mr_fsm_t mr_fsm_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the fsm object from the fsm container */
    return (mr_fsm_t)mr_object_find(name, Mr_Object_Type_Fsm);
}

/**
 * @brief This function adds a state machine to the container.
 *
 * @param fsm The state machine to be added.
 * @param name The name of the state machine.
 * @param table The table of the state machine.
 * @param table_size The size of the table.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_add(mr_fsm_t fsm, const char *name, mr_fsm_table_t table, mr_size_t table_size)
{
    mr_size_t count = 0;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(fsm != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(table != MR_NULL);
    MR_ASSERT(table_size != 0);

    /* Add the object to the container */
    ret = mr_object_add(&fsm->object, name, Mr_Object_Type_Fsm);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", name, ret);
        return ret;
    }

    /* Initialize the private fields */
    fsm->table = table;
    fsm->table_size = table_size;
    fsm->current_state = 0;
    fsm->next_state = 0;

    /* Protect the callback and signal function of each state */
    for (count = 0; count < table_size; count++)
    {
        table[count].cb = table[count].cb ? table[count].cb : err_io_fsm_cb;
        table[count].signal = table[count].signal ? table[count].signal : err_io_fsm_signal;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function removes a state machine from the container.
 *
 * @param fsm The state machine to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_remove(mr_fsm_t fsm)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(fsm != MR_NULL);
    MR_ASSERT(fsm->object.type == Mr_Object_Type_Fsm);

    /* Remove the object from the container */
    ret = mr_object_remove(&fsm->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s remove failed: %d\r\n", fsm->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    fsm->table = MR_NULL;
    fsm->table_size = 0;
    fsm->current_state = 0;
    fsm->next_state = 0;

    return MR_ERR_OK;
}

/**
 * @brief This function handles the state machine.
 *
 * @param fsm The state machine to be handled.
 */
void mr_fsm_handle(mr_fsm_t fsm)
{
    MR_ASSERT(fsm != MR_NULL);
    MR_ASSERT(fsm->object.type == Mr_Object_Type_Fsm);

    /* Transition to the next state */
    if (fsm->next_state != fsm->current_state)
    {
        fsm->current_state = fsm->next_state;
    }

    /* Check whether the current state is valid */
    if (fsm->current_state >= fsm->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG,
                   "[%s -> %d] handle failed: %d\r\n",
                   fsm->object.name,
                   fsm->current_state,
                   -MR_ERR_GENERIC);
        return;
    }

    /* Call the current state callback function */
    fsm->table[fsm->current_state].cb(fsm, fsm->table[fsm->current_state].args);
}

/**
 * @brief This function signals the state machine.
 *
 * @param fsm The state machine to be signaled.
 * @param signal The signal to be signaled.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    MR_ASSERT(fsm != MR_NULL);
    MR_ASSERT(fsm->object.type == Mr_Object_Type_Fsm);

    /* Check whether the current state is valid */
    if (fsm->current_state >= fsm->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG,
                   "[%s -> %d] signal failed: %d\r\n",
                   fsm->object.name,
                   fsm->current_state,
                   -MR_ERR_GENERIC);
        return -MR_ERR_GENERIC;
    }

    /* Call the current state signal function */
    return fsm->table[fsm->current_state].signal(fsm, signal);
}

/**
 * @brief This function shifts current state of the state machine.
 *
 * @param fsm The index machine.
 * @param index The index to be shifted.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_shift(mr_fsm_t fsm, mr_uint32_t index)
{
    MR_ASSERT(fsm != MR_NULL);
    MR_ASSERT(fsm->object.type == Mr_Object_Type_Fsm);

    /* Check whether the index is valid */
    if (index >= fsm->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] shift failed: %d\r\n", fsm->object.name, index, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Check whether the index of the shift was the next index */
    if (index == fsm->next_state)
    {
        return MR_ERR_OK;
    }

    /* Check the fsm is busy */
    if (fsm->next_state != fsm->current_state)
    {
        MR_DEBUG_E(DEBUG_TAG, "[%s -> %d] shift failed: %d\r\n", fsm->object.name, index, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    /* Shift to the next index */
    fsm->next_state = index;

    return MR_ERR_OK;
}

#endif /* MR_CFG_FSM */