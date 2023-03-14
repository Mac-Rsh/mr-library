/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-09     MacRsh       first version
 */

#include "serial.h"

static mr_err_t mr_serial_open(mr_device_t device)
{
	mr_serial_t serial = (mr_serial_t)device;
	struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;
	struct mr_serial_fifo *fifo = MR_NULL;

	/* Allocation the fifo memory */
	if (serial->fifo_bufsz != 0)
	{
		fifo = (struct mr_serial_fifo *)mr_malloc(sizeof(struct mr_serial_fifo *) + serial->fifo_bufsz);
		if (fifo == MR_NULL)
			return - MR_ERR_NO_MEMORY;

		mr_ringbuffer_init(&fifo->ringbuffer, fifo->pool, serial->fifo_bufsz);
		serial->fifo_rx = fifo;
		fifo = MR_NULL;

		fifo = (struct mr_serial_fifo *)mr_malloc(sizeof(struct mr_serial_fifo *) + serial->fifo_bufsz);
		if (fifo == MR_NULL)
			return - MR_ERR_NO_MEMORY;

		mr_ringbuffer_init(&fifo->ringbuffer, fifo->pool, serial->fifo_bufsz);
		serial->fifo_tx = fifo;
		fifo = MR_NULL;
	}

	/* Setting serial to default-config */
	if (serial->config.baud_rate == 0)
		serial->config = default_config;

	return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_close(mr_device_t device)
{
	mr_serial_t serial = (mr_serial_t)device;
	struct mr_serial_fifo *fifo = MR_NULL;

	/* Release the fifo memory */
	if (serial->fifo_bufsz != 0)
	{
		//TODO 检测写FIFO是否输出完毕，为输出完毕则在此输出，然后再关闭串口设备

		fifo = (struct mr_serial_fifo *)serial->fifo_rx;
		mr_free(fifo);
		serial->fifo_rx = MR_NULL;

		fifo = (struct mr_serial_fifo *)serial->fifo_tx;
		mr_free(fifo);
		serial->fifo_tx = MR_NULL;
	}

	/* Setting serial to close-config */
	serial->config.baud_rate = 0;

	return serial->ops->configure(serial, &serial->config);
}

static mr_err_t mr_serial_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_err_t ret = MR_ERR_OK;
	mr_serial_t serial = (mr_serial_t)device;

	switch (cmd & _MR_CMD_MASK)
	{
		case MR_CMD_CONFIG:
		{
			if (args)
			{
				ret = serial->ops->configure(serial, (struct mr_serial_config *)args);
				if (ret == MR_ERR_OK)
					serial->config = *(struct mr_serial_config *)args;
			}
			break;
		}

		default: ret = - MR_ERR_UNSUPPORTED;
	}

	return ret;
}

static mr_size_t mr_serial_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_serial_t serial = (mr_serial_t)device;
	struct mr_serial_fifo *fifo = MR_NULL;
	mr_size_t length;

	fifo = (struct mr_serial_fifo *)serial->fifo_rx;

	do
	{
		length = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
	} while (length < size);

	return mr_ringbuffer_read(&fifo->ringbuffer, buffer, size);
}

static mr_size_t mr_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_serial_t serial = (mr_serial_t)device;
	struct mr_serial_fifo *fifo = MR_NULL;
	mr_size_t length = 0;

	fifo = (struct mr_serial_fifo *)serial->fifo_tx;

	do
	{
		length += mr_ringbuffer_write(&fifo->ringbuffer, &buffer[length], size - length);
	} while (length < size);

	return length;
}

static mr_err_t _hw_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
	MR_LOG_D("Serial configure error: -MR_ERR_IO");
	return - MR_ERR_IO;
}

static void _hw_serial_write_byte(mr_serial_t serial, mr_uint8_t data)
{
	MR_LOG_D("Serial write error: -MR_ERR_IO");
}

static int _hw_serial_read_byte(mr_serial_t serial)
{
	MR_LOG_D("Serial read error: -MR_ERR_IO");
	return - 1;
}

mr_err_t mr_hw_serial_add_to_container(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			.open = mr_serial_open,
			.close = mr_serial_close,
			.ioctl = mr_serial_ioctl,
			.read = mr_serial_read,
			.write = mr_serial_write,
		};

	MR_ASSERT(serial != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the serial device to the container */
	ret =
		mr_device_add_to_container(&serial->device, name, MR_DEVICE_TYPE_SERIAL, MR_OPEN_RDWR, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the serial fields */
	serial->config.baud_rate = 0;
	serial->fifo_bufsz = MR_SERIAL_FIFO_BUFSZ;
	serial->fifo_rx = MR_NULL;
	serial->fifo_tx = MR_NULL;

	/* Set serial operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _hw_serial_configure;
	ops->write_byte = ops->write_byte ? ops->write_byte : _hw_serial_write_byte;
	ops->read_byte = ops->read_byte ? ops->read_byte : _hw_serial_read_byte;
	serial->ops = ops;

	return MR_ERR_OK;
}

void mr_hw_serial_isr(mr_serial_t serial, mr_uint16_t event)
{
	switch (event & _MR_SERIAL_EVENT_MASK)
	{
		case MR_SERIAL_EVENT_RX_INT:
		{
			struct mr_serial_fifo *fifo = (struct mr_serial_fifo *)serial->fifo_rx;
			mr_uint8_t data;

			/* Read data into the ring buffer */
			data = serial->ops->read_byte(serial);
			mr_ringbuffer_write_byte_force(&fifo->ringbuffer, data);

			/* Invoke the rx-callback function */
			if (serial->device.rx_callback != MR_NULL)
			{
				mr_size_t rx_length = 0;

				rx_length = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
				serial->device.rx_callback(&serial->device, &rx_length);
			}
			break;
		}

		default:break;
	}
}
