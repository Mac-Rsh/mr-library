/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include <mrlib.h>

void mr_fifo_init(mr_fifo_t fifo, mr_uint8_t *pool, mr_size_t pool_size)
{
	MR_ASSERT(fifo != MR_NULL);
	MR_ASSERT(pool != MR_NULL);

	fifo->read_index = 0;
	fifo->write_index = 0;
	fifo->read_mirror = 0;
	fifo->write_mirror = 0;

	fifo->buffer = pool;
	fifo->size = pool_size;
}

void mr_fifo_reset(mr_fifo_t fifo)
{
	MR_ASSERT(fifo != MR_NULL);

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	fifo->read_index = 0;
	fifo->write_index = 0;

	fifo->read_mirror = 0;
	fifo->write_mirror = 0;

	/* Enable interrupt */
	mr_hw_interrupt_enable();
}

mr_size_t mr_fifo_get_length(mr_fifo_t fifo)
{
	MR_ASSERT(fifo != MR_NULL);

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Empty or full according to the mirror flag */
	if (fifo->read_index == fifo->write_index)
	{
		if (fifo->read_mirror == fifo->write_mirror)
		{
			/* Enable interrupt */
			mr_hw_interrupt_enable();

			return 0;
		} else
		{
			/* Enable interrupt */
			mr_hw_interrupt_enable();

			return fifo->size;
		}
	}

	if (fifo->write_index > fifo->read_index)
	{
		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return fifo->write_index - fifo->read_index;
	} else
	{
		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return fifo->size - fifo->read_index + fifo->write_index;
	}
}

mr_size_t mr_fifo_read(mr_fifo_t fifo, mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(fifo != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Get the number of bytes that can be read */
	length = mr_fifo_get_length(fifo);

	/* If there is no data to read, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to read if it exceeds the available data */
	if (size > length)
		size = length;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the fifo to the buffer */
	if ((fifo->size - fifo->read_index) > size)
	{
		mr_memcpy(buffer, &fifo->buffer[fifo->read_index], size);
		fifo->read_index += size;

		/* Enable interrupt */
		mr_hw_interrupt_enable();
		return size;
	}

	mr_memcpy(buffer, &fifo->buffer[fifo->read_index], fifo->size - fifo->read_index);
	mr_memcpy(&buffer[fifo->size - fifo->read_index], &fifo->buffer[0], size - (fifo->size - fifo->read_index));

	fifo->read_mirror = ~ fifo->read_mirror;
	fifo->read_index = size - (fifo->size - fifo->read_index);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}

mr_size_t mr_fifo_write(mr_fifo_t fifo, const mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(fifo != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Calculate the number of bytes that can be written */
	length = fifo->size - mr_fifo_get_length(fifo);

	/* If there is no space to write, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to write if it exceeds the available data */
	if (size > length)
		size = length;

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		mr_memcpy(&fifo->buffer[fifo->write_index], buffer, size);
		fifo->write_index += size;

		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return size;
	}

	mr_memcpy(&fifo->buffer[fifo->write_index], buffer, fifo->size - fifo->write_index);
	mr_memcpy(&fifo->buffer[0], &buffer[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

	fifo->write_mirror = ~ fifo->write_mirror;
	fifo->write_index = size - (fifo->size - fifo->write_index);

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}

mr_size_t mr_fifo_write_force(mr_fifo_t fifo, const mr_uint8_t *buffer, mr_size_t size)
{
	mr_size_t length = 0;

	MR_ASSERT(fifo != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Calculate the number of bytes that can be written */
	length = fifo->size - mr_fifo_get_length(fifo);

	/* If the data exceeds the buffer length, the front data is discarded */
	if (size > fifo->size)
	{
		buffer = &buffer[size - fifo->size];
		size = fifo->size;
	}

	/* Disable interrupt */
	mr_hw_interrupt_disable();

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		mr_memcpy(&fifo->buffer[fifo->write_index], buffer, size);
		fifo->write_index += size;
		if (size > length)
			fifo->read_index = fifo->write_index;

		/* Enable interrupt */
		mr_hw_interrupt_enable();

		return size;
	}

	mr_memcpy(&fifo->buffer[fifo->write_index], buffer, fifo->size - fifo->write_index);
	mr_memcpy(&fifo->buffer[0], &buffer[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

	fifo->write_mirror = ~ fifo->write_mirror;
	fifo->write_index = size - (fifo->size - fifo->write_index);

	if (size > length)
	{
		if (fifo->write_index <= fifo->read_index)
			fifo->read_mirror = ~ fifo->read_mirror;

		fifo->read_index = fifo->write_index;
	}

	/* Enable interrupt */
	mr_hw_interrupt_enable();

	return size;
}