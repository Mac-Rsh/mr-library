/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#include "fifo.h"

/**
 * @brief This function memcpy.
 *
 * @param dst The destination.
 * @param src The source.
 * @param n The number of bytes.
 *
 * @return The destination.
 */
void *fifo_memcpy(void *dst, const void *src, size_t n)
{
	char *d = (char *)dst;
	const char *s = (const char *)src;

	while (n --)
	{
		*d ++ = *s ++;
	}
	return dst;
}

/**
 * @brief This function initialize the fifo.
 *
 * @param fifo The fifo to initialize.
 * @param pool The pool of data.
 * @param pool_size The size of the pool.
 */
void fifo_init(fifo_t fifo, void *pool, size_t pool_size)
{
	FIFO_ASSERT(fifo != MR_NULL);
	FIFO_ASSERT(pool != MR_NULL);

	fifo->read_index = 0;
	fifo->write_index = 0;
	fifo->read_mirror = 0;
	fifo->write_mirror = 0;

	fifo->buffer = pool;
	fifo->size = pool_size;
}

/**
 * @brief This function reset the fifo.
 *
 * @param fifo The fifo to reset.
 */
void fifo_reset(fifo_t fifo)
{
	FIFO_ASSERT(fifo != MR_NULL);

	fifo->read_index = 0;
	fifo->write_index = 0;

	fifo->read_mirror = 0;
	fifo->write_mirror = 0;
}

/**
 * @brief This function get the data size from the fifo.
 *
 * @param fifo The fifo to get the data size.
 *
 * @return The data size.
 */
size_t fifo_get_data_size(fifo_t fifo)
{
	FIFO_ASSERT(fifo != MR_NULL);

	/* Empty or full according to the mirror flag */
	if (fifo->read_index == fifo->write_index)
	{
		if (fifo->read_mirror == fifo->write_mirror)
		{
			return 0;
		} else
		{
			return fifo->size;
		}
	}

	if (fifo->write_index > fifo->read_index)
	{
		return fifo->write_index - fifo->read_index;
	} else
	{
		return fifo->size - fifo->read_index + fifo->write_index;
	}
}

/**
 * @brief This function reads from the fifo.
 *
 * @param fifo The fifo to be read.
 * @param buffer The data buffer to be read from the fifo.
 * @param size The size of the read.
 *
 * @return The size of the actual read.
 */
size_t fifo_read(fifo_t fifo, void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	FIFO_ASSERT(fifo != MR_NULL);
	FIFO_ASSERT(buffer != MR_NULL);

	if (size == 0)
		return 0;

	/* Get the data size */
	length = fifo_get_data_size(fifo);

	/* If there is no data to read, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to read if it exceeds the available data */
	if (size > length)
		size = length;

	/* Copy the data from the fifo to the buffer */
	if ((fifo->size - fifo->read_index) > size)
	{
		fifo_memcpy(buf, &fifo->buffer[fifo->read_index], size);
		fifo->read_index += size;

		return size;
	}

	fifo_memcpy(buf, &fifo->buffer[fifo->read_index], fifo->size - fifo->read_index);
	fifo_memcpy(&buf[fifo->size - fifo->read_index], &fifo->buffer[0], size - (fifo->size - fifo->read_index));

	fifo->read_mirror = ~ fifo->read_mirror;
	fifo->read_index = size - (fifo->size - fifo->read_index);

	return size;
}

/**
 * @brief This function write the fifo.
 *
 * @param fifo The fifo to be written.
 * @param buffer The data buffer to be written to fifo.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t fifo_write(fifo_t fifo, const void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	FIFO_ASSERT(fifo != MR_NULL);
	FIFO_ASSERT(buf != MR_NULL);

	if (size == 0)
		return 0;

	/* Get the space size */
	length = fifo_get_space_size(fifo);

	/* If there is no space to write, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to write if it exceeds the available data */
	if (size > length)
		size = length;

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		fifo_memcpy(&fifo->buffer[fifo->write_index], buf, size);
		fifo->write_index += size;

		return size;
	}

	fifo_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
	fifo_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

	fifo->write_mirror = ~ fifo->write_mirror;
	fifo->write_index = size - (fifo->size - fifo->write_index);

	return size;
}

/**
 * @brief This function force write the fifo.
 *
 * @param fifo The fifo to be written.
 * @param buffer The data buffer to be written to fifo.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t fifo_write_force(fifo_t fifo, const void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	FIFO_ASSERT(fifo != MR_NULL);
	FIFO_ASSERT(buf != MR_NULL);

	if (size == 0)
		return 0;

	/* Get the space size */
	length = fifo_get_space_size(fifo);

	/* If the data exceeds the buffer length, the front data is discarded */
	if (size > fifo->size)
	{
		buf = &buf[size - fifo->size];
		size = fifo->size;
	}

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		fifo_memcpy(&fifo->buffer[fifo->write_index], buf, size);
		fifo->write_index += size;
		if (size > length)
			fifo->read_index = fifo->write_index;

		return size;
	}

	fifo_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
	fifo_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

	fifo->write_mirror = ~ fifo->write_mirror;
	fifo->write_index = size - (fifo->size - fifo->write_index);

	if (size > length)
	{
		if (fifo->write_index <= fifo->read_index)
			fifo->read_mirror = ~ fifo->read_mirror;

		fifo->read_index = fifo->write_index;
	}

	return size;
}