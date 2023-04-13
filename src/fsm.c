/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     MacRsh       first version
 */

#include <mrlib.h>

mr_fsm_manager_t mr_fsm_manager_find(const char *name)
{
	mr_event_manager_t event_manager = MR_NULL;

	MR_ASSERT(name != MR_NULL);

	event_manager = mr_event_manager_find(name);
	if (event_manager == MR_NULL || event_manager->type != MR_EVENT_MANAGER_TYPE_FSM)
		return MR_NULL;

	return (mr_fsm_manager_t)event_manager;
}

mr_err_t mr_fsm_manager_add_to_container(mr_fsm_manager_t fsm_manager, const char *name, mr_uint16_t state)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(fsm_manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);

	/* Add the event-manager to the container */
	ret = mr_event_manager_add_to_container(&fsm_manager->event_manager,
											name,
											MR_EVENT_MANAGER_TYPE_FSM,
											fsm_manager->event_pool,
											sizeof(fsm_manager->event_pool));
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	fsm_manager->state = state;

	return MR_ERR_OK;
}

void mr_fsm_manager_transfer_state(mr_fsm_manager_t fsm_manager, mr_uint16_t state)
{
	MR_ASSERT(fsm_manager != MR_NULL);

	fsm_manager->state = state;
}

mr_err_t mr_fsm_manager_process(mr_fsm_manager_t fsm_manager)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(fsm_manager != MR_NULL);

	ret = mr_event_manager_notify(&fsm_manager->event_manager, fsm_manager->state);
	if (ret != MR_ERR_OK)
		return ret;

	return mr_event_manager_process(&fsm_manager->event_manager);
}

mr_err_t mr_fsm_create_to_manager(mr_fsm_manager_t fsm_manager,
								  mr_uint16_t value,
								  mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
								  void *args)
{
	MR_ASSERT(fsm_manager != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	return mr_event_create_to_manager(&fsm_manager->event_manager, value, callback, args);
}

