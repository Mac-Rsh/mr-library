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
    /* Check if the kernel has this container type */
    if (type >= _MR_CONTAINER_TYPE_MASK)
        return MR_NULL;

    return &mr_kernel_container[type];
}

mr_object_t mr_object_find(const char *name, enum mr_container_type type)
{
    mr_list_t list = MR_NULL;
    mr_container_t container = MR_NULL;

    /* Get corresponding container */
    container = mr_container_find(type);
    if (container == MR_NULL)
        return MR_NULL;

    /* Walk through the container looking for objects */
    for (list = (container->list).next; list != &(container->list); list = list->next)
    {
        mr_object_t object;
        object = mr_container_of(list, struct mr_object, list);
        if (object == MR_NULL)
            continue;
        if (mr_strncmp(object->name, name, MR_NAME_MAX) == 0)
            return object;
    }

    return MR_NULL;
}

void mr_object_init(mr_object_t object, const char *name)
{
    /* Set the object list to its initial state */
    mr_list_init(&(object->list));

    /* Copy the specified name to the object name */
    mr_strncpy(object->name, name, MR_NAME_MAX);

    /* Set the object type to initialized */
    object->type = MR_OBJECT_TYPE_NULL;
    object->type |= MR_OBJECT_TYPE_STATIC;
}

mr_object_t mr_object_create(const char *name)
{
    mr_object_t object = MR_NULL;

    /* Allocate memory for the object */
    object = (mr_object_t) mr_malloc(sizeof(struct mr_object));
    if (object == MR_NULL)
        return MR_NULL;

    /* Initialize the memory */
    mr_memset(object, 0x0, sizeof(struct mr_object));

    /* Set the object list to its initial state */
    mr_list_init(&(object->list));

    /* Copy the specified name to the object name */
    mr_strncpy(object->name, name, MR_NAME_MAX);

    return object;
}

mr_err_t mr_object_add_to_container(mr_object_t object, enum mr_container_type container_type)
{
    mr_container_t container = MR_NULL;

    /* Check if the object is already registered */
    if (object->type & MR_OBJECT_TYPE_REGISTER)
        return -MR_ERR_BUSY;

    /* Find the container for the specified type */
    container = mr_container_find(container_type);
    if (container == MR_NULL)
        return -MR_ERR_NOT_FOUND;

    /* Check if the object already exists in the container */
    if (mr_object_find(object->name, container_type) != MR_NULL)
        return -MR_ERR_GENERIC;

    /* Insert the object into the container's list */
    mr_list_insert_after(&(container->list), &(object->list));
    object->type |= MR_OBJECT_TYPE_REGISTER;

    return MR_ERR_OK;
}

mr_err_t mr_object_remove_from_container(mr_object_t object)
{
    mr_err_t error_code = MR_ERR_OK;

    /* Check if the object is registered */
    if ((object->type & MR_OBJECT_TYPE_REGISTER) == 0)
        return -MR_ERR_GENERIC;

    /* Remove the object from the container's list */
    mr_list_remove(&(object->list));
    object->type &= ~MR_OBJECT_TYPE_REGISTER;

    return error_code;
}

mr_err_t mr_object_move(mr_object_t object, enum mr_container_type dest_type)
{
    mr_err_t error_code = MR_ERR_OK;

    /* Remove the object from its current container */
    error_code = mr_object_remove_from_container(object);
    if (error_code != MR_ERR_OK)
        return error_code;

    /* Add the object to the new container */
    error_code = mr_object_add_to_container(object, dest_type);

    return error_code;
}

void mr_object_rename(mr_object_t object, char *name)
{
    mr_strncpy(object->name, name, MR_NAME_MAX);
}

mr_bool_t mr_object_is_static(mr_object_t object)
{
    if (object->type & MR_OBJECT_TYPE_STATIC)
        return MR_TRUE;
    else
        return MR_FALSE;
}

void mr_mutex_init(mr_mutex_t mutex)
{
    mutex->owner = MR_NULL;
    mutex->lock = MR_UNLOCK;
}

mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner)
{
    mr_hw_interrupt_disable();

    if (mutex->owner != owner)
    {
        if (mutex->lock == MR_LOCK)
        {
            mr_hw_interrupt_enable();

            return -MR_ERR_BUSY;
        }

        mutex->owner = owner;
    }

    mutex->lock = MR_LOCK;

    mr_hw_interrupt_enable();
    return MR_ERR_OK;
}

mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner)
{
    mr_hw_interrupt_disable();

    if (mutex->owner == owner)
    {
        mutex->lock = MR_UNLOCK;

        mr_hw_interrupt_enable();
        return MR_ERR_OK;
    }

    mr_hw_interrupt_enable();
    return -MR_ERR_GENERIC;
}

void mr_ringbuffer_init(mr_ringbuffer_t ringbuffer, mr_uint8_t *pool, mr_size_t pool_size)
{
    ringbuffer->read_mirror = ringbuffer->read_index = 0;
    ringbuffer->write_mirror = ringbuffer->write_index = 0;

    ringbuffer->buffer = pool;
    ringbuffer->buffer_size = pool_size;
}

MR_INLINE enum mr_ringbuffer_state mr_ringbuffer_get_state(mr_ringbuffer_t ringbuffer)
{
    if (ringbuffer->read_index == ringbuffer->write_index)
    {
        if (ringbuffer->read_mirror == ringbuffer->write_mirror)
            return MR_RINGBUFFER_EMPTY;
        else
            return MR_RINGBUFFER_FULL;
    }

    return MR_RINGBUFFER_HALF_FULL;
}

mr_size_t mr_ringbuffer_get_data_length(mr_ringbuffer_t ringbuffer)
{
    mr_size_t wi, ri;

    switch (mr_ringbuffer_get_state(ringbuffer))
    {
        case MR_RINGBUFFER_EMPTY:return 0;
        case MR_RINGBUFFER_FULL:return ringbuffer->buffer_size;
        default:
        {
            wi = ringbuffer->write_index;
            ri = ringbuffer->read_index;

            if (wi > ri)
                return wi - ri;
            else
                return (ringbuffer->buffer_size - (ri - wi));
        }
    }
}

MR_INLINE mr_size_t mr_ringbuffer_space_length(mr_ringbuffer_t ringbuffer)
{
    return (ringbuffer->buffer_size - mr_ringbuffer_get_data_length(ringbuffer));
}

mr_size_t mr_ringbuffer_write(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length)
{
    mr_size_t space_length;

    space_length = mr_ringbuffer_space_length(ringbuffer);
    if (space_length == 0)
        return 0;

    if (space_length < length)
        length = space_length;

    if ((ringbuffer->buffer_size - ringbuffer->write_index) > length)
    {
        mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index], buffer, length);
        ringbuffer->write_index += length;
        return length;
    } else
    {
        mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index],
                  &buffer[0],
                  (ringbuffer->buffer_size - ringbuffer->write_index));
        mr_memcpy(&ringbuffer->buffer[0],
                  &buffer[ringbuffer->buffer_size - ringbuffer->write_index],
                  (length - (ringbuffer->buffer_size - ringbuffer->write_index)));

        ringbuffer->write_mirror = ~ringbuffer->write_mirror;
        ringbuffer->write_index = (length - (ringbuffer->buffer_size - ringbuffer->write_index));

        return length;
    }
}

mr_size_t mr_ringbuffer_write_force(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length)
{
    mr_size_t space_length;

    space_length = mr_ringbuffer_space_length(ringbuffer);
    if (space_length == 0)
        return 0;

    if (ringbuffer->buffer_size < length)
    {
        buffer = &buffer[length - ringbuffer->buffer_size];
        length = ringbuffer->buffer_size;
    }

    if ((ringbuffer->buffer_size - ringbuffer->write_index) > length)
    {
        mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index], buffer, length);
        ringbuffer->write_index += length;

        if (length > space_length)
            ringbuffer->read_index = ringbuffer->write_index;

        return length;
    } else
    {
        mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index],
                  &buffer[0],
                  (ringbuffer->buffer_size - ringbuffer->write_index));
        mr_memcpy(&ringbuffer->buffer[0],
                  &buffer[ringbuffer->buffer_size - ringbuffer->write_index],
                  (length - (ringbuffer->buffer_size - ringbuffer->write_index)));

        ringbuffer->write_mirror = ~ringbuffer->write_mirror;
        ringbuffer->write_index = (length - (ringbuffer->buffer_size - ringbuffer->write_index));

        if (length > space_length)
        {
            if (ringbuffer->write_index <= ringbuffer->read_index)
                ringbuffer->read_mirror = ~ringbuffer->read_mirror;
            ringbuffer->read_index = ringbuffer->write_index;
        }

        return length;
    }
}

mr_size_t mr_ringbuffer_read(mr_ringbuffer_t ringbuffer, mr_uint8_t *buffer, mr_size_t length)
{
    mr_size_t count;

    count = mr_ringbuffer_get_data_length(ringbuffer);
    if (count == 0)
        return 0;

    if (count < length)
        length = count;

    if ((ringbuffer->buffer_size - ringbuffer->read_index) > length)
    {
        mr_memcpy(&buffer[0], &ringbuffer->buffer[ringbuffer->read_index], length);
        ringbuffer->read_index += length;
        return length;
    }

    /* read index cross mirror */
    mr_memcpy(buffer,
              &ringbuffer->buffer[ringbuffer->read_index],
              ringbuffer->buffer_size - ringbuffer->read_index);
    mr_memcpy(&buffer[ringbuffer->buffer_size - ringbuffer->read_index],
              ringbuffer->buffer,
              length - (ringbuffer->buffer_size - ringbuffer->read_index));

    ringbuffer->read_mirror = ~ringbuffer->read_mirror;
    ringbuffer->read_index = length - (ringbuffer->buffer_size - ringbuffer->read_index);

    return length;
}

mr_size_t mr_ringbuffer_write_byte(mr_ringbuffer_t ringbuffer, mr_uint8_t data)
{
    /* Whether there is enough space */
    if (!mr_ringbuffer_space_length(ringbuffer))
        return 0;

    ringbuffer->buffer[ringbuffer->write_index] = data;

    /* flip mirror */
    if (ringbuffer->write_index == ringbuffer->buffer_size - 1)
    {
        ringbuffer->write_mirror = ~ringbuffer->write_mirror;
        ringbuffer->write_index = 0;
    } else
    {
        ringbuffer->write_index++;
    }

    return 1;
}

mr_size_t mr_ringbuffer_write_byte_force(mr_ringbuffer_t ringbuffer, mr_uint8_t data)
{
    enum mr_ringbuffer_state old_state;

    old_state = mr_ringbuffer_get_state(ringbuffer);

    ringbuffer->buffer[ringbuffer->write_index] = data;

    /* flip mirror */
    if (ringbuffer->write_index == ringbuffer->buffer_size - 1)
    {
        ringbuffer->write_mirror = ~ringbuffer->write_mirror;
        ringbuffer->write_index = 0;
        if (old_state == MR_RINGBUFFER_FULL)
        {
            ringbuffer->read_mirror = ~ringbuffer->read_mirror;
            ringbuffer->read_index = ringbuffer->write_index;
        }
    } else
    {
        ringbuffer->write_index++;
        if (old_state == MR_RINGBUFFER_FULL)
            ringbuffer->read_index = ringbuffer->write_index;
    }

    return 1;
}

mr_size_t mr_ringbuffer_read_byte(mr_ringbuffer_t ringbuffer, mr_uint8_t *data)
{
    /* ringbuffer is empty */
    if (!mr_ringbuffer_get_data_length(ringbuffer))
        return 0;

    /* put byte */
    *data = ringbuffer->buffer[ringbuffer->read_index];

    if (ringbuffer->read_index == ringbuffer->buffer_size - 1)
    {
        ringbuffer->read_mirror = ~ringbuffer->read_mirror;
        ringbuffer->read_index = 0;
    } else
    {
        ringbuffer->read_index++;
    }

    return 1;
}