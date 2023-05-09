/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include <device/dac/dac.h>

#if (MR_DEVICE_DAC == MR_CONF_ENABLE)

#undef LOG_TAG
#define LOG_TAG "dac"

static mr_err_t mr_dac_open(mr_device_t device)
{
	mr_dac_t dac = (mr_dac_t)device;

	return dac->ops->configure(dac, MR_ENABLE);
}

static mr_err_t mr_dac_close(mr_device_t device)
{
	mr_dac_t dac = (mr_dac_t)device;

	return dac->ops->configure(dac, MR_DISABLE);
}

static mr_err_t mr_dac_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_dac_t dac = (mr_dac_t)device;

	switch (cmd & _MR_CTRL_FLAG_MASK)
	{
		case MR_CTRL_CONFIG:
		{
			if (args)
			{
				return dac->ops->channel_configure(dac,
												   ((struct mr_dac_config *)args)->channel,
												   ((struct mr_dac_config *)args)->state);
			}
			return - MR_ERR_INVALID;
		}

		default: return - MR_ERR_UNSUPPORTED;
	}
}

static mr_ssize_t mr_dac_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_dac_t dac = (mr_dac_t)device;
	mr_uint32_t *send_buffer = (mr_uint32_t *)buffer;
	mr_size_t send_size = 0;

	if (size < sizeof(*send_buffer))
	{
		MR_LOG_E(LOG_TAG, "Device %s: Invalid write size %d\r\n", device->object.name, size);
		return - MR_ERR_INVALID;
	}

	for (send_size = 0; send_size < size; send_size += sizeof(*send_buffer))
	{
		dac->ops->write(dac, (mr_uint16_t)pos, *send_buffer);
		send_buffer ++;
	}

	return (mr_ssize_t)size;
}

static mr_err_t _err_io_dac_configure(mr_dac_t dac, mr_state_t state)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_err_t _err_io_dac_channel_configure(mr_dac_t dac, mr_uint16_t channel, mr_state_t state)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static void _err_io_dac_write(mr_dac_t dac, mr_uint16_t channel, mr_uint32_t value)
{
	MR_ASSERT(0);
}

mr_err_t mr_hw_dac_add(mr_dac_t dac, const char *name, struct mr_dac_ops *ops, void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			mr_dac_open,
			mr_dac_close,
			mr_dac_ioctl,
			MR_NULL,
			mr_dac_write,
		};

	MR_ASSERT(dac != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the dac-device to the container */
	ret = mr_device_add(&dac->device, name, MR_DEVICE_TYPE_DAC, MR_OPEN_WRONLY, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Set dac operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_dac_configure;
	ops->channel_configure = ops->channel_configure ? ops->channel_configure : _err_io_dac_channel_configure;
	ops->write = ops->write ? ops->write : _err_io_dac_write;
	dac->ops = ops;

	return MR_ERR_OK;
}

#endif
