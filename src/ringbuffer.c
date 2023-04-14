/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-16     MacRsh       first version
 */

#include <mrlib.h>

void mr_ringbuffer_init(mr_ringbuffer_t rb, mr_uint8_t *pool, mr_size_t pool_size)
{
	MR_ASSERT(rb != MR_NULL);
	MR_ASSERT(pool != MR_NULL);

	rb->read_index = 0;
	rb->write_index = 0;
	rb->read_mirror = 0;
	rb->write_mirror = 0;

	rb->buffer = pool;
	rb->size = pool_size;
}

void mr_ringbuffer_reset(mr_ringbuffer_t rb)
{
	MR_ASSERT(rb != MR_NULL);

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	rb->read_index = 0;
	rb->write_index = 0;

	rb->read_mirror = 0;
	rb->write_mirror = 0;

	/* Enable interrupt */
	mr_hw_interrupt_enable();
}

mr_size_t mr_ringbuffer_get_data_length(mr_ringbuffer_t rb)
{
	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Empty or full according to the mirror flag */
	if (rb->read_index == rb->write_index)
	{
		if (rb->read_mirror == rb->write_mirror)
		{
			/* Enable interrupt */
			mr_hw_interrupt_enable();

			return 0;
		} else
		{
			/* Enable interrupt */
			mr_hw_interrupt_enable();

			return rb->size;
		}
	}

	if (rb->write_index > rb->read_index)
	{
		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return rb->write_index - rb->read_index;
	} else
	{
		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return rb->size - rb->read_index + rb->write_index;
	}
}

mr_size_t mr_ringbuffer_read(mr_ringbuffer_t rb, mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(rb != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Get the number of bytes that can be read */
	length = mr_ringbuffer_get_data_length(rb);

	/* If there is no data to read, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to read if it exceeds the available data */
	if (size > length)
		size = length;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the ringbuffer to the buffer */
	if ((rb->size - rb->read_index) > size)
	{
		mr_memcpy(buffer, &rb->buffer[rb->read_index], size);
		rb->read_index += size;

		/* Enable interrupt */
		mr_hw_interrupt_enable();
		return size;
	}

	mr_memcpy(buffer, &rb->buffer[rb->read_index], rb->size - rb->read_index);
	mr_memcpy(&buffer[rb->size - rb->read_index], &rb->buffer[0], size - (rb->size - rb->read_index));

	rb->read_mirror = ~ rb->read_mirror;
	rb->read_index = size - (rb->size - rb->read_index);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}

mr_size_t mr_ringbuffer_write(mr_ringbuffer_t rb, const mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(rb != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Calculate the number of bytes that can be written */
	length = rb->size - mr_ringbuffer_get_data_length(rb);

	/* If there is no space to write, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to write if it exceeds the available data */
	if (size > length)
		size = length;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the buffer to the ringbuffer */
	if ((rb->size - rb->write_index) > size)
	{
		mr_memcpy(&rb->buffer[rb->write_index], buffer, size);
		rb->write_index += size;

		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return size;
	}

	mr_memcpy(&rb->buffer[rb->write_index], buffer, rb->size - rb->write_index);
	mr_memcpy(&rb->buffer[0], &buffer[rb->size - rb->write_index], size - (rb->size - rb->write_index));

	rb->write_mirror = ~ rb->write_mirror;
	rb->write_index = size - (rb->size - rb->write_index);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}

mr_size_t mr_ringbuffer_write_force(mr_ringbuffer_t rb, const mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(rb != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Calculate the number of bytes that can be written */
	length = rb->size - mr_ringbuffer_get_data_length(rb);

	/* If the data exceeds the buffer length, the front data is discarded */
	if (size > rb->size)
	{
		buffer = &buffer[size - rb->size];
		size = rb->size;
	}

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the buffer to the ringbuffer */
	if ((rb->size - rb->write_index) > size)
	{
		mr_memcpy(&rb->buffer[rb->write_index], buffer, size);
		rb->write_index += size;
		if (size > length)
			rb->read_index = rb->write_index;

		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return size;
	}

	mr_memcpy(&rb->buffer[rb->write_index], buffer, rb->size - rb->write_index);
	mr_memcpy(&rb->buffer[0], &buffer[rb->size - rb->write_index], size - (rb->size - rb->write_index));

	rb->write_mirror = ~ rb->write_mirror;
	rb->write_index = size - (rb->size - rb->write_index);

	if (size > length)
	{
		if (rb->write_index <= rb->read_index)
			rb->read_mirror = ~ rb->read_mirror;

		rb->read_index = rb->write_index;
	}

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}