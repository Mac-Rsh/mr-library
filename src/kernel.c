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

static struct mr_container mr_kernel_container[] =
        {
                {
                        MR_OBJECT_TYPE_NONE,
                        {&mr_kernel_container[MR_OBJECT_TYPE_NONE].list, &mr_kernel_container[MR_OBJECT_TYPE_NONE].list}
                },
#if (MR_CONF_DEVICE == MR_CONF_ENABLE)
                {
                        MR_OBJECT_TYPE_DEVICE,
                        {&mr_kernel_container[MR_OBJECT_TYPE_DEVICE].list,
                         &mr_kernel_container[MR_OBJECT_TYPE_DEVICE].list}
                },
#endif
#if (MR_CONF_EVENT == MR_CONF_ENABLE)
                {
                        MR_OBJECT_TYPE_EVENT,
                        {&mr_kernel_container[MR_OBJECT_TYPE_EVENT].list,
                         &mr_kernel_container[MR_OBJECT_TYPE_EVENT].list}
                },
#endif
#if (MR_CONF_SOFT_TIMER == MR_CONF_ENABLE)
                {
                        MR_OBJECT_TYPE_SOFT_TIMER,
                        {&mr_kernel_container[MR_OBJECT_TYPE_SOFT_TIMER].list,
                         &mr_kernel_container[MR_OBJECT_TYPE_SOFT_TIMER].list}
                },
#endif
        };

/**
 * @brief This function find the container.
 *
 * @param type The type of the container.
 *
 * @return A handle to the find container, or MR_NULL if not find.
 */
mr_container_t mr_container_find(mr_uint8_t type)
{
    for (mr_size_t count = 0; count < mr_array_get_length(mr_kernel_container); count++)
    {
        if (mr_kernel_container[count].type == type)
        {
            return &mr_kernel_container[count];
        }
    }

    return MR_NULL;
}

/**
 * @brief This function find the object.
 *
 * @param name The name of the object.
 * @param type The type of the object.
 *
 * @return A handle to the find object, or MR_NULL if not find.
 */
mr_object_t mr_object_find(const char *name, mr_uint8_t type)
{
    mr_container_t container = MR_NULL;

    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < mr_array_get_length(mr_kernel_container));

    /* Get corresponding container */
    container = mr_container_find(type);
    if (container == MR_NULL)
    {
        return MR_NULL;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Walk through the container looking for objects */
    for (mr_list_t list = container->list.next; list != &container->list; list = list->next)
    {
        mr_object_t object = mr_container_of(list, struct mr_object, list);
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
 * @param type The type of the object.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_object_add(mr_object_t object, const char *name, mr_uint8_t type)
{
    mr_container_t container = MR_NULL;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < mr_array_get_length(mr_kernel_container));

    /* Find the container for the specified type */
    container = mr_container_find(type);
    if (container == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Check if the object already exists in the container */
    if (mr_object_find(name, type) != MR_NULL)
    {
        return -MR_ERR_BUSY;
    }

    /* Initialize the private fields */
    mr_strncpy(object->name, name, MR_CONF_NAME_MAX);
    object->type = type;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the object into the container's list */
    mr_list_insert_before(&(container->list), &(object->list));

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

    /* Check if the object already exists in the container */
    if (mr_object_find(object->name, object->type) == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the object from the container's list */
    mr_list_remove(&(object->list));

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function change the type of the object.
 *
 * @param object The object to be changed.
 * @param type The type of the object.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_object_change_type(mr_object_t object, mr_uint8_t type)
{
    mr_container_t container = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(type < mr_array_get_length(mr_kernel_container));

    /* Find the container for the specified type */
    container = mr_container_find(type);
    if (container == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Check if the object already exists in the container */
    if (mr_object_find(object->name, object->type) == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Change the object type */
    object->type = type;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the object from the old container's list */
    mr_list_remove(&(object->list));

    /* Insert the object into the new container's list */
    mr_list_insert_after(&(container->list), &(object->list));

    /* Enable interrupt */
    mr_interrupt_enable();

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
    MR_ASSERT(name != MR_NULL);

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

    mutex->hold = 0;
    mutex->owner = MR_NULL;
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

    if (mutex->owner == MR_NULL)
    {
        mutex->hold++;
        mutex->owner = owner;

        /* Enable interrupt */
        mr_interrupt_enable();

        return MR_ERR_OK;
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return -MR_ERR_BUSY;
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
        mutex->hold--;

        if (mutex->hold == 0)
        {
            mutex->owner = MR_NULL;
        }

        /* Enable interrupt */
        mr_interrupt_enable();

        return MR_ERR_OK;
    }

    return -MR_ERR_GENERIC;
}

/**
 * @brief This function get the owner of the mutex.
 * @param mutex The mutex to get the owner.
 * @return A handle to the owner of the mutex, or MR_NULL if without owner.
 */
mr_object_t mr_mutex_get_owner(mr_mutex_t mutex)
{
    MR_ASSERT(mutex != MR_NULL);

    return mutex->owner;
}