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

mr_err_t mr_fsm_manager_add_to_container(mr_fsm_manager_t manager, const char *name, mr_uint32_t state)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(name != MR_NULL);

	/* Add the manager to the container */
	ret = mr_event_manager_add_to_container(&manager->manager,
											name,
											MR_EVENT_MANAGER_TYPE_FSM,
											manager->pool,
											sizeof(manager->pool));
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	manager->state = state;

	return MR_ERR_OK;
}

mr_err_t mr_fsm_create_to_manager(mr_fsm_manager_t manager,
								  mr_uint32_t state,
								  mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
								  void *args)
{
	MR_ASSERT(manager != MR_NULL);
	MR_ASSERT(callback != MR_NULL);

	return mr_event_create_to_manager(&manager->manager, state, callback, args);
}

mr_err_t mr_fsm_manager_transfer_state(mr_fsm_manager_t manager, mr_uint32_t state)
{
	MR_ASSERT(manager != MR_NULL);

	/* Check if the state already exists in the manager */
	if (mr_event_find(&manager->manager, state) == MR_NULL)
		return - MR_ERR_NOT_FOUND;

	/* State transfer */
	manager->state = state;

	return MR_ERR_OK;
}

mr_err_t mr_fsm_manager_handler(mr_fsm_manager_t manager)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(manager != MR_NULL);

	ret = mr_event_manager_notify(&manager->manager, manager->state);
	if (ret != MR_ERR_OK)
		return ret;

	return mr_event_manager_handler(&manager->manager);
}

