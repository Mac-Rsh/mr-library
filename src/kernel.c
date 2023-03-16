/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-08     MacRsh       first version
 */

#include <mrlib.h>

static struct mr_container mr_kernel_container[_MR_CONTAINER_TYPE_MASK] =
	{
		{.type = MR_CONTAINER_TYPE_MISC,
			.list = {&mr_kernel_container[MR_CONTAINER_TYPE_MISC].list,
					 &mr_kernel_container[MR_CONTAINER_TYPE_MISC].list}},
		{.type = MR_CONTAINER_TYPE_DEVICE,
			.list = {&mr_kernel_container[MR_CONTAINER_TYPE_DEVICE].list,
					 &mr_kernel_container[MR_CONTAINER_TYPE_DEVICE].list}},
	};

mr_container_t mr_container_find(enum mr_container_type type)
{
	MR_ASSERT(type < _MR_CONTAINER_TYPE_MASK);

	return &mr_kernel_container[type];
}

mr_object_t mr_object_find(const char *name, enum mr_container_type type)
{
	mr_list_t list = MR_NULL;
	mr_container_t container = MR_NULL;
	mr_object_t object = MR_NULL;

	/* Get corresponding container */
	container = mr_container_find(type);

	mr_enter_critical();

	/* Walk through the container looking for objects */
	for (list = (container->list).next; list != &(container->list); list = list->next)
	{
		object = mr_container_of(list, struct mr_object, list);
		if (mr_strncmp(object->name, name, MR_NAME_MAX) == 0)
			return object;
	}

	mr_exit_critical();

	return MR_NULL;
}

mr_err_t mr_object_add_to_container(mr_object_t object, const char *name, enum mr_container_type container_type)
{
	mr_container_t container = MR_NULL;

	MR_ASSERT(object != MR_NULL);

	/* Check if the object is already registered */
	if (object->type & MR_OBJECT_TYPE_REGISTER)
		return - MR_ERR_BUSY;

	/* Check if the object already exists in the container */
	if (mr_object_find(name, container_type) != MR_NULL)
		return - MR_ERR_GENERIC;

	/* Copy the specified name to the object name */
	mr_strncpy(object->name, name, MR_NAME_MAX);

	/* Find the container for the specified type */
	container = mr_container_find(container_type);

	mr_enter_critical();

	/* Insert the object into the container's list */
	mr_list_insert_after(&(container->list), &(object->list));
	object->type |= MR_OBJECT_TYPE_REGISTER;

	mr_exit_critical();

	return MR_ERR_OK;
}

mr_err_t mr_object_remove_from_container(mr_object_t object)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(object != MR_NULL);

	/* Check if the object is registered */
	if ((object->type & MR_OBJECT_TYPE_REGISTER) == 0)
		return - MR_ERR_GENERIC;

	mr_enter_critical();

	/* Remove the object from the container's list */
	mr_list_remove(&(object->list));
	object->type &= ~ MR_OBJECT_TYPE_REGISTER;

	mr_exit_critical();

	return ret;
}

mr_err_t mr_object_move(mr_object_t object, enum mr_container_type dst_type)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(object != MR_NULL);

	/* Remove the object from its current container */
	ret = mr_object_remove_from_container(object);
	if (ret != MR_ERR_OK)
		return ret;

	/* Add the object to the new container */
	ret = mr_object_add_to_container(object, object->name, dst_type);

	return ret;
}

void mr_object_rename(mr_object_t object, char *name)
{
	MR_ASSERT(object != MR_NULL);

	mr_strncpy(object->name, name, MR_NAME_MAX);
}

void mr_mutex_init(mr_mutex_t mutex)
{
	MR_ASSERT(mutex != MR_NULL);

	mutex->owner = MR_NULL;
	mutex->lock = MR_UNLOCK;
}

mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner)
{
	MR_ASSERT(mutex != MR_NULL);
	MR_ASSERT(owner != MR_NULL);

	mr_enter_critical();

	if (mutex->owner != owner)
	{
		if (mutex->lock == MR_LOCK)
		{
			mr_hw_interrupt_enable();

			return - MR_ERR_BUSY;
		}

		mutex->owner = owner;
	}
	mutex->lock = MR_LOCK;

	mr_exit_critical();

	return MR_ERR_OK;
}

mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner)
{
	MR_ASSERT(mutex != MR_NULL);
	MR_ASSERT(owner != MR_NULL);

	mr_enter_critical();

	if (mutex->owner == owner)
	{
		mutex->lock = MR_UNLOCK;

		mr_exit_critical();

		return MR_ERR_OK;
	}

	mr_exit_critical();

	return - MR_ERR_GENERIC;
}