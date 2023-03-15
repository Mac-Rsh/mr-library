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

	/* Walk through the container looking for objects */
	for (list = (container->list).next; list != &(container->list); list = list->next)
	{
		object = mr_container_of(list, struct mr_object, list);
		if (mr_strncmp(object->name, name, MR_NAME_MAX) == 0)
			return object;
	}

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

	/* Insert the object into the container's list */
	mr_list_insert_after(&(container->list), &(object->list));
	object->type |= MR_OBJECT_TYPE_REGISTER;

	return MR_ERR_OK;
}

mr_err_t mr_object_remove_from_container(mr_object_t object)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(object != MR_NULL);

	/* Check if the object is registered */
	if ((object->type & MR_OBJECT_TYPE_REGISTER) == 0)
		return - MR_ERR_GENERIC;

	/* Remove the object from the container's list */
	mr_list_remove(&(object->list));
	object->type &= ~ MR_OBJECT_TYPE_REGISTER;

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

	mr_hw_interrupt_disable();

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

	mr_hw_interrupt_enable();
	return MR_ERR_OK;
}

mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner)
{
	MR_ASSERT(mutex != MR_NULL);
	MR_ASSERT(owner != MR_NULL);

	mr_hw_interrupt_disable();

	if (mutex->owner == owner)
	{
		mutex->lock = MR_UNLOCK;

		mr_hw_interrupt_enable();
		return MR_ERR_OK;
	}

	mr_hw_interrupt_enable();
	return - MR_ERR_GENERIC;
}

void mr_ringbuffer_init(mr_ringbuffer_t ringbuffer, mr_uint8_t *pool, mr_size_t pool_size)
{
	MR_ASSERT(ringbuffer != MR_NULL);
	MR_ASSERT(pool != MR_NULL);

	ringbuffer->read_mirror = ringbuffer->read_index = 0;
	ringbuffer->write_mirror = ringbuffer->write_index = 0;

	ringbuffer->buffer = pool;
	ringbuffer->bufsz = pool_size;
}

MR_INLINE enum mr_ringbuffer_state mr_ringbuffer_get_state(mr_ringbuffer_t ringbuffer)
{
	MR_ASSERT(ringbuffer != MR_NULL);

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

	MR_ASSERT(ringbuffer != MR_NULL);

	switch (mr_ringbuffer_get_state(ringbuffer))
	{
		case MR_RINGBUFFER_EMPTY: return 0;
		case MR_RINGBUFFER_FULL: return ringbuffer->bufsz;
		default:
		{
			wi = ringbuffer->write_index;
			ri = ringbuffer->read_index;

			if (wi > ri)
				return wi - ri;
			else
				return (ringbuffer->bufsz - (ri - wi));
		}
	}
}

MR_INLINE mr_size_t mr_ringbuffer_space_length(mr_ringbuffer_t ringbuffer)
{
	MR_ASSERT(ringbuffer != MR_NULL);

	return (ringbuffer->bufsz - mr_ringbuffer_get_data_length(ringbuffer));
}

mr_size_t mr_ringbuffer_write_byte(mr_ringbuffer_t ringbuffer, mr_uint8_t data)
{
	/* Whether there is enough space */
	if (! mr_ringbuffer_space_length(ringbuffer))
		return 0;

	ringbuffer->buffer[ringbuffer->write_index] = data;

	/* flip mirror */
	if (ringbuffer->write_index == ringbuffer->bufsz - 1)
	{
		ringbuffer->write_mirror = ~ ringbuffer->write_mirror;
		ringbuffer->write_index = 0;
	} else
	{
		ringbuffer->write_index ++;
	}

	return 1;
}

mr_size_t mr_ringbuffer_write(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length)
{
	mr_size_t space_length;

	MR_ASSERT(ringbuffer != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	space_length = mr_ringbuffer_space_length(ringbuffer);
	if (space_length == 0)
		return 0;

	if (space_length < length)
		length = space_length;

	if ((ringbuffer->bufsz - ringbuffer->write_index) > length)
	{
		mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index], buffer, length);
		ringbuffer->write_index += length;
		return length;
	} else
	{
		mr_memcpy(&ringbuffer->buffer[ringbuffer->write_index],
				  &buffer[0],
				  (ringbuffer->bufsz - ringbuffer->write_index));
		mr_memcpy(&ringbuffer->buffer[0],
				  &buffer[ringbuffer->bufsz - ringbuffer->write_index],
				  (length - (ringbuffer->bufsz - ringbuffer->write_index)));

		ringbuffer->write_mirror = ~ ringbuffer->write_mirror;
		ringbuffer->write_index = (length - (ringbuffer->bufsz - ringbuffer->write_index));

		return length;
	}
}

mr_size_t mr_ringbuffer_write_data_force(mr_ringbuffer_t ringbuffer, mr_uint8_t data)
{
	enum mr_ringbuffer_state old_state;

	MR_ASSERT(ringbuffer != MR_NULL);

	old_state = mr_ringbuffer_get_state(ringbuffer);

	ringbuffer->buffer[ringbuffer->write_index] = data;

	/* flip mirror */
	if (ringbuffer->write_index == ringbuffer->bufsz - 1)
	{
		ringbuffer->write_mirror = ~ ringbuffer->write_mirror;
		ringbuffer->write_index = 0;
		if (old_state == MR_RINGBUFFER_FULL)
		{
			ringbuffer->read_mirror = ~ ringbuffer->read_mirror;
			ringbuffer->read_index = ringbuffer->write_index;
		}
	} else
	{
		ringbuffer->write_index ++;
		if (old_state == MR_RINGBUFFER_FULL)
			ringbuffer->read_index = ringbuffer->write_index;
	}

	return 1;
}

mr_size_t mr_ringbuffer_write_force(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length)
{
	mr_size_t space_length;

	MR_ASSERT(ringbuffer != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	space_length = mr_ringbuffer_space_length(ringbuffer);
	if (space_length == 0)
		return 0;

	if (ringbuffer->bufsz < length)
	{
		buffer = &buffer[length - ringbuffer->bufsz];
		length = ringbuffer->bufsz;
	}

	if ((ringbuffer->bufsz - ringbuffer->write_index) > length)
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
				  (ringbuffer->bufsz - ringbuffer->write_index));
		mr_memcpy(&ringbuffer->buffer[0],
				  &buffer[ringbuffer->bufsz - ringbuffer->write_index],
				  (length - (ringbuffer->bufsz - ringbuffer->write_index)));

		ringbuffer->write_mirror = ~ ringbuffer->write_mirror;
		ringbuffer->write_index = (length - (ringbuffer->bufsz - ringbuffer->write_index));

		if (length > space_length)
		{
			if (ringbuffer->write_index <= ringbuffer->read_index)
				ringbuffer->read_mirror = ~ ringbuffer->read_mirror;
			ringbuffer->read_index = ringbuffer->write_index;
		}

		return length;
	}
}

mr_size_t mr_ringbuffer_read_data(mr_ringbuffer_t ringbuffer, mr_uint8_t *data)
{
	MR_ASSERT(ringbuffer != MR_NULL);
	MR_ASSERT(data != MR_NULL);

	/* ringbuffer is empty */
	if (! mr_ringbuffer_get_data_length(ringbuffer))
		return 0;

	/* put byte */
	*data = ringbuffer->buffer[ringbuffer->read_index];

	if (ringbuffer->read_index == ringbuffer->bufsz - 1)
	{
		ringbuffer->read_mirror = ~ ringbuffer->read_mirror;
		ringbuffer->read_index = 0;
	} else
	{
		ringbuffer->read_index ++;
	}

	return 1;
}

mr_size_t mr_ringbuffer_read(mr_ringbuffer_t ringbuffer, mr_uint8_t *buffer, mr_size_t length)
{
	mr_size_t count;

	MR_ASSERT(ringbuffer != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	count = mr_ringbuffer_get_data_length(ringbuffer);
	if (count == 0)
		return 0;

	if (count < length)
		length = count;

	if ((ringbuffer->bufsz - ringbuffer->read_index) > length)
	{
		mr_memcpy(&buffer[0], &ringbuffer->buffer[ringbuffer->read_index], length);
		ringbuffer->read_index += length;
		return length;
	}

	/* read index cross mirror */
	mr_memcpy(buffer,
			  &ringbuffer->buffer[ringbuffer->read_index],
			  ringbuffer->bufsz - ringbuffer->read_index);
	mr_memcpy(&buffer[ringbuffer->bufsz - ringbuffer->read_index],
			  ringbuffer->buffer,
			  length - (ringbuffer->bufsz - ringbuffer->read_index));

	ringbuffer->read_mirror = ~ ringbuffer->read_mirror;
	ringbuffer->read_index = length - (ringbuffer->bufsz - ringbuffer->read_index);

	return length;
}

