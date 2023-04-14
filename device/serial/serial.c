/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-09     MacRsh       first version
 */

#include <device/serial/serial.h>

#if (MR_DEVICE_SERIAL == MR_CONF_ENABLE)

static mr_err_t mr_serial_open(mr_device_t device)
{
	mr_serial_t serial = (mr_serial_t)device;
	struct mr_serial_config default_config = MR_SERIAL_CONFIG_DEFAULT;
	struct mr_serial_fifo *fifo = MR_NULL;

	/* Allocation the fifo memory */
	if (serial->fifo_bufsz != 0)
	{
		/* Rx-fifo */
		fifo = (struct mr_serial_fifo *)mr_malloc(sizeof(struct mr_serial_fifo) + serial->fifo_bufsz);
		if (fifo == MR_NULL)
			return - MR_ERR_NO_MEMORY;

		mr_ringbuffer_init(&fifo->ringbuffer, fifo->pool, serial->fifo_bufsz);
		serial->fifo_rx = fifo;
		fifo = MR_NULL;

		/* Tx-fifo */
		fifo = (struct mr_serial_fifo *)mr_malloc(sizeof(struct mr_serial_fifo) + serial->fifo_bufsz);
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
		/* Rx-fifo */
		fifo = (struct mr_serial_fifo *)serial->fifo_rx;
		mr_free(fifo);
		serial->fifo_rx = MR_NULL;

		/* Tx-fifo */
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
	mr_serial_t serial = (mr_serial_t)device;
	mr_err_t ret = MR_ERR_OK;

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

		case MR_CMD_SET_RX_CALLBACK:
		{
			if (args)
				device->rx_callback = (mr_err_t (*)(mr_device_t device, void *args))args;
			break;
		}

		case MR_CMD_SET_TX_CALLBACK:
		{
			if (args)
				device->tx_callback = (mr_err_t (*)(mr_device_t device, void *args))args;
			break;
		}

		default: ret = - MR_ERR_UNSUPPORTED;
	}

	return ret;
}

static mr_size_t mr_serial_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_serial_t serial = (mr_serial_t)device;
	mr_uint8_t *recv_buffer = (mr_uint8_t *)buffer;
	struct mr_serial_fifo *fifo = (struct mr_serial_fifo *)serial->fifo_rx;
	mr_size_t recv_size = 0;

	while (recv_size < size)
	{
		/* Read if the ringbuffer has data */
		recv_size += mr_ringbuffer_read(&fifo->ringbuffer, recv_buffer + recv_size, size - recv_size);
	}

	return size;
}

static mr_size_t mr_serial_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_serial_t serial = (mr_serial_t)device;
	mr_uint8_t *send_buffer = (mr_uint8_t *)buffer;
	struct mr_serial_fifo *fifo = (struct mr_serial_fifo *)serial->fifo_tx;
	mr_size_t send_size = 0;

	while (send_size < size)
	{
		/* Write if the ringbuffer has space */
		send_size += mr_ringbuffer_write(&fifo->ringbuffer, send_buffer + send_size, size - send_size);

		/* Start send */
		serial->ops->start_tx(serial);
	}

	return size;
}

static mr_err_t _err_io_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static void _err_io_serial_write(mr_serial_t serial, mr_uint8_t data)
{
	MR_ASSERT(0);
}

static mr_uint8_t _err_io_serial_read(mr_serial_t serial)
{
	MR_ASSERT(0);
	return 0;
}

static void _err_io_serial_start_tx(mr_serial_t serial)
{
	MR_ASSERT(0);
}

static void _err_io_serial_stop_tx(mr_serial_t serial)
{
	MR_ASSERT(0);
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

	/* Add the serial-device to the container */
	ret = mr_device_add_to_container(&serial->device, name, MR_DEVICE_TYPE_SERIAL, MR_OPEN_RDWR, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the serial fields */
	serial->config.baud_rate = 0;
	serial->fifo_bufsz = MR_SERIAL_FIFO_SIZE;
	serial->fifo_rx = MR_NULL;
	serial->fifo_tx = MR_NULL;

	/* Set serial operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_serial_configure;
	ops->write = ops->write ? ops->write : _err_io_serial_write;
	ops->read = ops->read ? ops->read : _err_io_serial_read;
	ops->start_tx = ops->start_tx ? ops->start_tx : _err_io_serial_start_tx;
	ops->stop_tx = ops->stop_tx ? ops->stop_tx : _err_io_serial_stop_tx;
	serial->ops = ops;

	return MR_ERR_OK;
}

void mr_hw_serial_isr(mr_serial_t serial, mr_uint16_t event)
{
	switch (event & _MR_SERIAL_EVENT_MASK)
	{
		case MR_SERIAL_EVENT_RX_INT:
		{
			struct mr_serial_fifo *fifo = MR_NULL;
			mr_uint8_t data;

			fifo = (struct mr_serial_fifo *)serial->fifo_rx;

			/* Read data into the ring buffer */
			data = serial->ops->read(serial);
			mr_ringbuffer_write_force(&fifo->ringbuffer, &data, 1);

			/* Invoke the rx-callback function */
			if (serial->device.rx_callback != MR_NULL)
			{
				mr_size_t length;

				length = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
				serial->device.rx_callback(&serial->device, &length);
			}
			break;
		}

		case MR_SERIAL_EVENT_TX_INT:
		{
			struct mr_serial_fifo *fifo = MR_NULL;
			mr_size_t length;
			mr_uint8_t data;

			fifo = (struct mr_serial_fifo *)serial->fifo_tx;

			length = mr_ringbuffer_get_data_length(&fifo->ringbuffer);
			if (length == 0)
			{
				serial->ops->stop_tx(serial);

				/* Invoke the tx-callback function */
				if (serial->device.tx_callback != MR_NULL)
				{
					serial->device.rx_callback(&serial->device, &length);
				}
				break;
			}

			mr_ringbuffer_read(&fifo->ringbuffer, &data, 1);
			serial->ops->write(serial, data);
			break;
		}

		default:break;
	}
}

#endif
