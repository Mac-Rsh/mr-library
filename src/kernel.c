/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrapi.h"

static struct mr_object_container mr_object_container_table[] =
    {
        {Mr_Object_Type_None,   {&mr_object_container_table[Mr_Object_Type_None].list,   &mr_object_container_table[Mr_Object_Type_None].list}},
        {Mr_Object_Type_Device, {&mr_object_container_table[Mr_Object_Type_Device].list, &mr_object_container_table[Mr_Object_Type_Device].list}},
        {Mr_Object_Type_Module, {&mr_object_container_table[Mr_Object_Type_Module].list, &mr_object_container_table[Mr_Object_Type_Module].list}},
    };

static mr_size_t mr_allocated_memory_size = 0;

/**
 * @brief This function find the object container.
 *
 * @param type The type of the object container.
 *
 * @return A pointer to the find container, or MR_NULL if not find.
 */
mr_object_container_t mr_object_container_find(mr_uint32_t type)
{
    mr_size_t count = 0;

    for (count = 0; count < mr_array_num(mr_object_container_table); count++)
    {
        if (mr_object_container_table[count].type == type)
        {
            return &mr_object_container_table[count];
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
 * @return A pointer to the find object, or MR_NULL if not find.
 */
mr_object_t mr_object_find(const char *name, mr_uint32_t type)
{
    mr_object_container_t container = MR_NULL;
    mr_list_t list = MR_NULL;

    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < mr_array_num(mr_object_container_table));

    /* Get corresponding container */
    container = mr_object_container_find(type);
    if (container == MR_NULL)
    {
        return MR_NULL;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Walk through the container looking for objects */
    for (list = container->list.next; list != &container->list; list = list->next)
    {
        mr_object_t object = mr_container_of(list, struct mr_object, list);
        if (mr_strncmp(object->name, name, MR_CFG_OBJECT_NAME_SIZE) == 0)
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
mr_err_t mr_object_add(mr_object_t object, const char *name, mr_uint32_t type)
{
    mr_object_container_t container = MR_NULL;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type < mr_array_num(mr_object_container_table));

    /* Get the container for the specified type */
    container = mr_object_container_find(type);
    if (container == MR_NULL)
    {
        return -MR_ERR_UNSUPPORTED;
    }

    /* Check if the object already exists in the container */
    if (mr_object_find(name, type) != MR_NULL)
    {
        return -MR_ERR_BUSY;
    }

    /* Initialize the private fields */
    mr_strncpy(object->name, name, MR_CFG_OBJECT_NAME_SIZE);
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

    /* Reset the object type */
    object->type = Mr_Object_Type_None;

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
mr_err_t mr_object_change_type(mr_object_t object, mr_uint32_t type)
{
    mr_object_container_t container = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(object != MR_NULL);
    MR_ASSERT(type < mr_array_num(mr_object_container_table));

    /* Get the container for the specified type */
    container = mr_object_container_find(type);
    if (container == MR_NULL)
    {
        return -MR_ERR_UNSUPPORTED;
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
    mr_list_insert_before(&(container->list), &(object->list));

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

    mr_strncpy(object->name, name, MR_CFG_OBJECT_NAME_SIZE);
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
 * @param acquirer The acquirer of the mutex.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_mutex_take(mr_mutex_t mutex, void *acquirer)
{
    MR_ASSERT(mutex != MR_NULL);

    /* Check if the acquirer is valid */
    if (acquirer == MR_NULL)
    {
        return -MR_ERR_INVALID;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    if (mutex->owner == MR_NULL)
    {
        mutex->hold++;
        mutex->owner = acquirer;

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
 * @param owner The owner of the mutex.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_mutex_release(mr_mutex_t mutex, void *owner)
{
    MR_ASSERT(mutex != MR_NULL);

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

    return -MR_ERR_INVALID;
}

/**
 * @brief This function get the owner of the mutex.
 *
 * @param mutex The mutex to get the owner.
 *
 * @return A pointer to the owner of the mutex, or MR_NULL if without owner.
 */
volatile void *mr_mutex_get_owner(mr_mutex_t mutex)
{
    MR_ASSERT(mutex != MR_NULL);

    return mutex->owner;
}

/**
 * @brief This function allocate memory.
 *
 * @param size The size of the memory.
 *
 * @return A pointer to the allocated memory, or MR_NULL if failed.
 */
void *mr_malloc(mr_size_t size)
{
    void *memory = MR_NULL;

    /* Disable interrupt */
    mr_interrupt_disable();

    memory = malloc(size);
    if (memory != MR_NULL)
    {
        mr_allocated_memory_size += size;
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return memory;
}

/**
 * @brief This function free memory.
 *
 * @param memory The memory to be freed.
 */
void mr_free(void *memory)
{
    int *size = ((int *)memory) - 5;

    if (memory != MR_NULL)
    {
        mr_allocated_memory_size -= *size;
        free(memory);
    }
}

/**
 * @brief This function get the allocated size.
 *
 * @return The allocated size.
 */
mr_size_t mr_memory_get_allocated_size(void)
{
    return mr_allocated_memory_size;
}