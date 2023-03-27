/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-27     MacRsh       first version
 */

#include "adc.h"

#if (MR_DEVICE_ADC == MR_CONF_ENABLE)

static mr_err_t mr_adc_open(mr_device_t device)
{
	mr_adc_t adc = (mr_adc_t)device;

	return adc->ops->configure(adc, MR_ENABLE);
}

static mr_err_t mr_adc_close(mr_device_t device)
{
	mr_adc_t adc = (mr_adc_t)device;

	return adc->ops->configure(adc, MR_DISABLE);
}

static mr_err_t mr_adc_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_adc_t adc = (mr_adc_t)device;
	mr_err_t ret = MR_ERR_OK;

	switch (cmd & _MR_CMD_MASK)
	{
		case MR_CMD_CONFIG:
		{
			if (args)
				ret = adc->ops->channel_configure(adc,
												  ((struct mr_adc_config *)args)->channel,
												  ((struct mr_adc_config *)args)->state);

			break;
		}

		default: ret = - MR_ERR_UNSUPPORTED;
	}

	return ret;
}

static mr_size_t mr_adc_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t count)
{
	mr_adc_t adc = (mr_adc_t)device;
	mr_uint16_t *recv_buffer = MR_NULL;
	mr_size_t recv_count = count;

	recv_buffer = (mr_uint16_t *)buffer;
	while (recv_count --)
	{
		*recv_buffer = adc->ops->read(adc, (mr_uint16_t)pos);
		recv_buffer ++;
	}

	return count;
}

static mr_err_t _hw_adc_configure(mr_adc_t adc, mr_state_t state)
{
	MR_LOG_E("Adc configure error: -MR_ERR_IO");
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_err_t _hw_adc_channel_configure(mr_adc_t adc, mr_uint16_t channel, mr_state_t state)
{
	MR_LOG_E("Adc channel-configure error: -MR_ERR_IO");
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_uint16_t _hw_adc_read(mr_adc_t adc, mr_uint16_t channel)
{
	MR_LOG_E("Adc read error: -MR_ERR_IO");
	MR_ASSERT(0);
	return 0;
}

mr_err_t mr_hw_adc_add_to_container(mr_adc_t adc, const char *name, struct mr_adc_ops *ops, void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			.open = mr_adc_open,
			.close = mr_adc_close,
			.ioctl = mr_adc_ioctl,
			.read = mr_adc_read,
			.write = MR_NULL,
		};

	MR_ASSERT(adc != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the adc-device to the container */
	ret = mr_device_add_to_container(&adc->device, name, MR_DEVICE_TYPE_ADC, MR_OPEN_RDONLY, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Set adc operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _hw_adc_configure;
	ops->channel_configure = ops->channel_configure ? ops->channel_configure : _hw_adc_channel_configure;
	ops->read = ops->read ? ops->read : _hw_adc_read;
	adc->ops = ops;

	return MR_ERR_OK;
}

#endif