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

#if (MR_CONF_FSM == MR_ENABLE)

#define DEBUG_TAG   "fsm"

static mr_err_t _err_io_fsm_cb(mr_fsm_t cb, void *args)
{
    return -MR_ERR_IO;
}

static mr_err_t _err_io_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    return -MR_ERR_IO;
}

/**
 * @brief This function adds a state machine to the container.
 *
 * @param fsm The state machine.
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
    ret = mr_object_add(&fsm->object, name, MR_OBJECT_TYPE_FSM);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s add failed: %d\r\n", fsm->object.name, ret);
        return ret;
    }

    /* Initialize the private fields */
    fsm->table = table;
    fsm->table_size = table_size;
    fsm->current_state = 0;
    fsm->next_state = 0;

    /* Protect callback and signal function of each state */
    for (count = 0; count < table_size; count++)
    {
        table[count].cb = table[count].cb ? table[count].cb : _err_io_fsm_cb;
        table[count].signal = table[count].signal ? table[count].signal : _err_io_fsm_signal;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function handles the state machine.
 *
 * @param fsm The state machine.
 */
void mr_fsm_handle(mr_fsm_t fsm)
{
    MR_ASSERT(fsm != MR_NULL);

    /* Transition to the next state */
    if (fsm->next_state != fsm->current_state)
    {
        fsm->current_state = fsm->next_state;
    }

    /* Call the current state callback */
    fsm->table[fsm->current_state].cb(fsm, fsm->table[fsm->current_state].args);
}

/**
 * @brief This function signals the state machine.
 *
 * @param fsm The state machine.
 * @param signal The signal to be signaled.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    MR_ASSERT(fsm != MR_NULL);

    /* Call the state signal */
    return fsm->table[fsm->current_state].signal(fsm, signal);
}

/**
 * @brief This function shifts current state of the state machine.
 *
 * @param fsm The state machine.
 * @param state The state to be shifted.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_fsm_shift(mr_fsm_t fsm, mr_uint32_t state)
{
    MR_ASSERT(fsm != MR_NULL);

    /* Check whether the state is valid */
    if(state >= fsm->table_size)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s shift failed: %d\r\n", fsm->object.name, -MR_ERR_INVALID);
        mr_object_set_err(&fsm->object, -MR_ERR_INVALID);
        return -MR_ERR_INVALID;
    }

    /* Check whether the state of the shift was the next state */
    if(state == fsm->next_state)
    {
        return MR_ERR_OK;
    }

    /* Check the fsm is busy */
    if (fsm->next_state != fsm->current_state)
    {
        MR_DEBUG_E(DEBUG_TAG, "%s shift failed: %d\r\n", fsm->object.name, -MR_ERR_BUSY);
        mr_object_set_err(&fsm->object, -MR_ERR_BUSY);
        return -MR_ERR_BUSY;
    }

    /* Shift to the next state */
    fsm->next_state = state;

    return MR_ERR_OK;
}

#endif  /* MR_FSM_ENABLE */