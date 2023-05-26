/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#include "serial.h"

struct serial_fifo
{
	uint16_t read_mirror: 1;
	uint16_t read_index: 15;
	uint16_t write_mirror: 1;
	uint16_t write_index: 15;
	uint16_t size;

	uint8_t buffer[];
};

static void *serial_fifo_memcpy(void *dst, const void *src, size_t n)
{
	char *d = (char *)dst;
	const char *s = (const char *)src;

	while (n --)
	{
		*d ++ = *s ++;
	}
	return dst;
}

static size_t serial_fifo_get_data_size(struct serial_fifo *fifo)
{
	SERIAL_ASSERT(fifo != NULL);

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

#define serial_fifo_get_space_size(fifo) ((fifo->size) - serial_fifo_get_data_size(fifo))

static size_t serial_fifo_read(struct serial_fifo *fifo, void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	SERIAL_ASSERT(fifo != NULL);
	SERIAL_ASSERT(buffer != NULL);

	if (size == 0)
		return 0;

	/* Get the data bufsz */
	length = serial_fifo_get_data_size(fifo);

	/* If there is no data to read, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to read if it exceeds the available data */
	if (size > length)
		size = length;

	/* Copy the data from the fifo to the buffer */
	if ((fifo->size - fifo->read_index) > size)
	{
		serial_fifo_memcpy(buf, &fifo->buffer[fifo->read_index], size);
		fifo->read_index += size;

		return size;
	}

	serial_fifo_memcpy(buf, &fifo->buffer[fifo->read_index], fifo->size - fifo->read_index);
	serial_fifo_memcpy(&buf[fifo->size - fifo->read_index], &fifo->buffer[0], size - (fifo->size - fifo->read_index));

	fifo->read_mirror = ~ fifo->read_mirror;
	fifo->read_index = size - (fifo->size - fifo->read_index);

	return size;
}

static size_t serial_fifo_write(struct serial_fifo *fifo, void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	SERIAL_ASSERT(fifo != NULL);
	SERIAL_ASSERT(buffer != NULL);

	if (size == 0)
		return 0;

	/* Get the space bufsz */
	length = serial_fifo_get_space_size(fifo);

	/* If there is no space to write, return 0 */
	if (length == 0)
		return 0;

	/* Adjust the number of bytes to write if it exceeds the available data */
	if (size > length)
		size = length;

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		serial_fifo_memcpy(&fifo->buffer[fifo->write_index], buf, size);
		fifo->write_index += size;

		return size;
	}

	serial_fifo_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
	serial_fifo_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

	fifo->write_mirror = ~ fifo->write_mirror;
	fifo->write_index = size - (fifo->size - fifo->write_index);

	return size;
}

static size_t serial_fifo_write_force(struct serial_fifo *fifo, const void *buffer, size_t size)
{
	uint8_t *buf = (uint8_t *)buffer;
	size_t length = 0;

	SERIAL_ASSERT(fifo != MR_NULL);
	SERIAL_ASSERT(buffer != MR_NULL);

	/* Get the space size */
	length = serial_fifo_get_space_size(fifo);

	/* If the data exceeds the buffer length, the front data is discarded */
	if (size > fifo->size)
	{
		buf = &buf[size - fifo->size];
		size = fifo->size;
	}

	/* Copy the data from the buffer to the fifo */
	if ((fifo->size - fifo->write_index) > size)
	{
		serial_fifo_memcpy(&fifo->buffer[fifo->write_index], buf, size);
		fifo->write_index += size;
		if (size > length)
			fifo->read_index = fifo->write_index;

		return size;
	}

	serial_fifo_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
	serial_fifo_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

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

static int _err_io_serial_configure(serial_t serial, struct serial_config *config)
{
	SERIAL_ASSERT(0);
	return - SERIAL_ERR_IO;
}

static void _err_io_serial_write(serial_t serial, uint8_t data)
{
	SERIAL_ASSERT(0);
}

static uint8_t _err_io_serial_read(serial_t serial)
{
	SERIAL_ASSERT(0);
	return 0;
}

static void _err_io_serial_start_tx(serial_t serial)
{
	SERIAL_ASSERT(0);
}

static void _err_io_serial_stop_tx(serial_t serial)
{
	SERIAL_ASSERT(0);
}

int serial_init(serial_t serial, size_t fifo_bufsz, struct serial_ops *ops, void *data)
{
	struct serial_fifo *fifo = NULL;

	SERIAL_ASSERT(serial != NULL);
	SERIAL_ASSERT(ops != NULL);

	serial->config.baud_rate = 0;
	if (fifo_bufsz < SERIAL_FIFO_BUFSZ_MIN)
		serial->fifo_bufsz = SERIAL_FIFO_BUFSZ_MIN;
	else
		serial->fifo_bufsz = fifo_bufsz;

	free(serial->rx_fifo);
	fifo = malloc(sizeof(struct serial_fifo) + serial->fifo_bufsz);
	if (fifo == NULL)
	{
		return - SERIAL_ERR_NO_MEMORY;
	}
	fifo->read_index = 0;
	fifo->write_index = 0;
	fifo->read_mirror = 0;
	fifo->write_mirror = 0;
	fifo->size = serial->fifo_bufsz;
	serial->rx_fifo = fifo;
	fifo = NULL;

	free(serial->tx_fifo);
	fifo = malloc(sizeof(struct serial_fifo) + serial->fifo_bufsz);
	if (fifo == NULL)
	{
		return - SERIAL_ERR_NO_MEMORY;
	}
	fifo->read_index = 0;
	fifo->write_index = 0;
	fifo->read_mirror = 0;
	fifo->write_mirror = 0;
	fifo->size = serial->fifo_bufsz;
	serial->tx_fifo = fifo;
	fifo = NULL;

	serial->rx_callback = NULL;
	serial->data = data;

	ops->configure = ops->configure ? ops->configure : _err_io_serial_configure;
	ops->write = ops->write ? ops->write : _err_io_serial_write;
	ops->read = ops->read ? ops->read : _err_io_serial_read;
	ops->start_tx = ops->start_tx ? ops->start_tx : _err_io_serial_start_tx;
	ops->stop_tx = ops->stop_tx ? ops->stop_tx : _err_io_serial_stop_tx;
	serial->ops = ops;

	return SERIAL_ERR_OK;
}

int serial_configure(serial_t serial, struct serial_config *config)
{
	SERIAL_ASSERT(serial != NULL);
	SERIAL_ASSERT(config != NULL);

	return serial->ops->configure(serial, config);
}

int serial_mode(serial_t serial,
				size_t baud_rate,
				uint8_t data_bits,
				uint8_t stop_bits,
				uint8_t parity,
				uint8_t bit_order)
{
	struct serial_config config = {baud_rate,
								   data_bits,
								   stop_bits,
								   parity,
								   bit_order};

	SERIAL_ASSERT(serial != NULL);

	return serial->ops->configure(serial, &config);
}

uint8_t serial_get(serial_t serial)
{
	struct serial_fifo *fifo = serial->rx_fifo;
	uint8_t data = 0;
	size_t recv_size = 0;

	SERIAL_ASSERT(serial != NULL);

	while (recv_size < 1)
	{
		recv_size += serial_fifo_read(fifo, &data, 1);
	}

	return data;
}

void serial_put(serial_t serial, uint8_t data)
{
	struct serial_fifo *fifo = serial->tx_fifo;
	size_t send_size = 0;

	SERIAL_ASSERT(serial != NULL);

	while (send_size < 1)
	{
		send_size += serial_fifo_write(fifo, &data, 1);

		serial->ops->start_tx(serial);
	}
}

size_t serial_read(serial_t serial, void *buffer, size_t size)
{
	struct serial_fifo *fifo = serial->rx_fifo;
	uint8_t *recv_buffer = (uint8_t *)buffer;
	size_t recv_size = 0;

	SERIAL_ASSERT(serial != NULL);
	SERIAL_ASSERT(buffer != NULL);

	while (recv_size < size)
	{
		/* Read if the fifo has data */
		recv_size += serial_fifo_read(fifo, recv_buffer + recv_size, size - recv_size);
	}

	return size;
}

size_t serial_write(serial_t serial, const void *buffer, size_t size)
{
	struct serial_fifo *fifo = serial->tx_fifo;
	uint8_t *send_buffer = (uint8_t *)buffer;
	size_t send_size = 0;

	SERIAL_ASSERT(serial != NULL);
	SERIAL_ASSERT(buffer != NULL);

	while (send_size < size)
	{
		send_size += serial_fifo_write(fifo, send_buffer + send_size, size - send_size);

		serial->ops->start_tx(serial);
	}

	return size;
}

void serial_hw_isr(serial_t serial, uint16_t event)
{
	SERIAL_ASSERT(serial != NULL);

	switch (event & 0xf000)
	{
		case SERIAL_EVENT_RX_INT:
		{
			struct serial_fifo *rx_fifo = NULL;
			uint8_t data = 0;

			rx_fifo = serial->rx_fifo;

			/* Read data into the ring buffer */
			data = serial->ops->read(serial);
			serial_fifo_write_force(rx_fifo, &data, 1);

			/* Invoke the rx-cb function */
			if (serial->rx_callback != NULL)
			{
				size_t length;

				length = serial_fifo_get_data_size(rx_fifo);
				serial->rx_callback(serial, length);
			}
			break;
		}

		case SERIAL_EVENT_TX_INT:
		{
			struct serial_fifo *tx_fifo = NULL;
			size_t length = 0;
			uint8_t data = 0;

			tx_fifo = serial->tx_fifo;

			length = serial_fifo_get_data_size(tx_fifo);
			if (length == 0)
			{
				serial->ops->stop_tx(serial);
				break;
			}

			serial_fifo_read(tx_fifo, &data, 1);
			serial->ops->write(serial, data);
			break;
		}

		default:break;
	}
}