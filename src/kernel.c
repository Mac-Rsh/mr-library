/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrlib.h"

static struct mr_container mr_kernel_container[_MR_CONTAINER_TYPE_MASK] =
        {
                {
                        .type = MR_CONTAINER_TYPE_MISC,
                        .list = {&mr_kernel_container[MR_CONTAINER_TYPE_MISC].list,
                                 &mr_kernel_container[MR_CONTAINER_TYPE_MISC].list}
                },
                {
                        .type = MR_CONTAINER_TYPE_DEVICE,
                        .list = {&mr_kernel_container[MR_CONTAINER_TYPE_DEVICE].list,
                                 &mr_kernel_container[MR_CONTAINER_TYPE_DEVICE].list}
                },
                {
                        .type = MR_CONTAINER_TYPE_SERVER,
                        .list = {&mr_kernel_container[MR_CONTAINER_TYPE_SERVER].list,
                                 &mr_kernel_container[MR_CONTAINER_TYPE_SERVER].list}
                },
        };

/**
 * @brief This function find the container.
 *
 * @param type The flag of the container.
 *
 * @return A handle to the find container, or MR_NULL if not find.
 */
mr_container_t mr_container_find(enum mr_container_type type)
{
    MR_ASSERT(type < _MR_CONTAINER_TYPE_MASK);

    return &mr_kernel_container[type];
}

/**
 * @brief This function find the object.
 *
 * @param name The name of the object.
 * @param type The container flag to which the object belongs.
 *
 * @return A handle to the find object, or MR_NULL if not find.
 */
mr_object_t mr_object_find(const char *name, enum mr_container_type type)
{
    mr_list_t list = MR_NULL;
    mr_container_t container = MR_NULL;
    mr_object_t object = MR_NULL;

    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < _MR_CONTAINER_TYPE_MASK);

    /* Get corresponding container */
    container = mr_container_find(type);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Walk through the container looking for objects */
    for (list = (container->list).next; list != &(container->list); list = list->next)
    {
        object = mr_container_of(list, struct mr_object, list);
        if (mr_strncmp(object->name, name, MR_CONF_NAME_MAX) == 0)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return object;
        }
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_NULL;
}

/**
 * @brief This function add object to the container.
 *
 * @param object The object to be added.
 * @param name The name of the object.
 * @param type The target container flag.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_container_type type)
{
    mr_container_t container = MR_NULL;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < _MR_CONTAINER_TYPE_MASK);

    /* Check if the object is already registered */
    if (object->flag & _MR_OBJECT_TYPE_REGISTER)
    {
        return -MR_ERR_BUSY;
    }

    /* Check if the object already exists in the container */
    if (mr_object_find(name, type) != MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Copy the specified name to the object name */
    mr_strncpy(object->name, name, MR_CONF_NAME_MAX);

    /* Find the container for the specified flag */
    container = mr_container_find(type);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the object into the container's list */
    mr_list_insert_after(&(container->list), &(object->list));
    object->flag |= _MR_OBJECT_TYPE_REGISTER;

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function remove object from the container.
 *
 * @param object The object to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_object_remove(mr_object_t object)
{
    MR_ASSERT(object != MR_NULL);

    /* Check if the object is registered */
    if ((object->flag & _MR_OBJECT_TYPE_REGISTER) == 0)
    {
        return -MR_ERR_GENERIC;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the object from the container's list */
    mr_list_remove(&(object->list));
    object->flag &= ~_MR_OBJECT_TYPE_REGISTER;

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function move object to specified container.
 *
 * @param object The object to be moved.
 * @param type The target container flag.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_object_move(mr_object_t object, enum mr_container_type type)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(type < _MR_CONTAINER_TYPE_MASK);

    /* Remove the object from its current container */
    ret = mr_object_remove(object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Add the object to the new container */
    ret = mr_object_add(object, object->name, type);

    return ret;
}

/**
 * @brief This function rename object.
 *
 * @param object The object to be renamed.
 * @param name The name of the object.
 */
void mr_object_rename(mr_object_t object, char *name)
{
    MR_ASSERT(object != MR_NULL);

    mr_strncpy(object->name, name, MR_CONF_NAME_MAX);
}

/**
 * @brief This function initialize mutex.
 *
 * @param mutex The mutex to be initialized.
 */
void mr_mutex_init(mr_mutex_t mutex)
{
    MR_ASSERT(mutex != MR_NULL);

    mutex->owner = MR_NULL;
    mutex->lock = MR_UNLOCK;
}

/**
 * @brief This function take the mutex.
 *
 * @param mutex The mutex to be taken.
 * @param owner The object that take the mutex.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner)
{
    MR_ASSERT(mutex != MR_NULL);
    MR_ASSERT(owner != MR_NULL);

    /* Disable interrupt */
    mr_interrupt_disable();

    if (mutex->owner != owner)
    {
        if (mutex->lock == MR_LOCK)
        {
            /* Enable interrupt */
            mr_interrupt_enable();

            return -MR_ERR_BUSY;
        }

        mutex->owner = owner;
    }
    mutex->lock = MR_LOCK;

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function release the mutex.
 *
 * @param mutex The mutex to be released.
 * @param owner The object that release the mutex.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner)
{
    MR_ASSERT(mutex != MR_NULL);
    MR_ASSERT(owner != MR_NULL);

    /* Disable interrupt */
    mr_interrupt_disable();

    if (mutex->owner == owner)
    {
        mutex->lock = MR_UNLOCK;

        /* Enable interrupt */
        mr_interrupt_enable();

        return MR_ERR_OK;
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return -MR_ERR_GENERIC;
}