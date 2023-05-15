/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/timer/timer.h"

#if (MR_CONF_DEVICE_TIMER == MR_CONF_ENABLE)

#undef LOG_TAG
#define LOG_TAG "timer"

static mr_uint32_t mr_timer_timeout_calculate(mr_timer_t timer, mr_uint32_t timeout)
{
	mr_uint32_t count = 0, timer_period = 0, reload = 0;
	mr_uint32_t i = 0, reload_min = 0, reload_max = 0, error = 0, error_min = 0, reload_best = 0;

	/* Calculate the timer-period and timeout total count */
	timer_period = 1000000 / timer->config.freq;
	count = timeout / timer_period;

	if (count == 0)
		count = 1;

	if (count < timer->information.max_cut)
	{
		timer->reload = 1;
		timer->cycles = timer->reload;
		timer->timeout = count * timer_period;
		return count;
	}

	if (count % timer->information.max_cut == 0)
	{
		timer->reload = count / timer->information.max_cut;
		timer->cycles = timer->reload;
		timer->timeout = timer->information.max_cut * timer_period;
		return timer->information.max_cut;
	}

	/* Calculate the Least error reload */
	reload_min = count / timer->information.max_cut + 1;
	reload_max = count / 5;
	error_min = reload_min;
	for (i = reload_min; i < reload_max; i ++)
	{
		reload = count / i;
		error = count - reload * i;
		if (error <= 1)
		{
			reload_best = i;
			break;
		} else if (error < error_min)
		{
			error_min = error;
			reload_best = i;
		}
	}

	timer->reload = reload_best;
	timer->cycles = timer->reload;
	timer->timeout = count / reload_best * timer_period;

	return count / reload_best;
}

static mr_err_t mr_timer_open(mr_device_t device)
{
	mr_timer_t timer = (mr_timer_t)device;
	struct mr_timer_config config = MR_TIMER_CONFIG_DEFAULT;

	if (timer->config.freq == 0)
		timer->config = config;

	if (timer->config.freq > timer->information.max_freq)
		return - MR_ERR_GENERIC;

	return timer->ops->configure(timer, &timer->config);
}

static mr_err_t mr_timer_close(mr_device_t device)
{
	mr_timer_t timer = (mr_timer_t)device;

	/* Setting timer to close-config */
	timer->config.freq = 0;

	return timer->ops->configure(timer, &timer->config);
}

static mr_err_t mr_timer_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_timer_t timer = (mr_timer_t)device;
	mr_err_t ret = MR_ERR_OK;

	switch (cmd & _MR_CTRL_FLAG_MASK)
	{
		case MR_CTRL_CONFIG:
		{
			if (args)
			{
				/* Check the frequency */
				if (((struct mr_timer_config *)args)->freq > timer->information.max_freq)
					return - MR_ERR_GENERIC;

				ret = timer->ops->configure(timer, (struct mr_timer_config *)args);
				if (ret == MR_ERR_OK)
					timer->config = *(struct mr_timer_config *)args;

				return ret;
			}
			return - MR_ERR_INVALID;
		}

		case MR_CTRL_SET_RX_CB:
		{
			device->rx_cb = args;
			return MR_ERR_OK;
		}

		case MR_CTRL_REBOOT:
		{
			timer->overflow = 0;
			timer->cycles = timer->reload;
			timer->ops->start(timer, timer->timeout / (1000000 / timer->config.freq));
			return MR_ERR_OK;
		}

		default: return - MR_ERR_UNSUPPORTED;
	}
}

static mr_ssize_t mr_timer_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_timer_t timer = (mr_timer_t)device;
	mr_uint32_t *recv_buffer = (mr_uint32_t *)buffer;
	mr_uint32_t cut = 0;

	if (size < sizeof(*recv_buffer))
	{
		MR_LOG_E(LOG_TAG, "Device %s: Invalid read size %d\r\n", device->object.name, size);
		return - MR_ERR_INVALID;
	}

	cut = timer->ops->get_count(timer);
	if (timer->information.cut_mode == _MR_TIMER_CUT_MODE_DOWN)
		cut = timer->timeout / (1000000 / timer->config.freq) - cut;

	*recv_buffer = timer->overflow * timer->timeout + cut * (1000000 / timer->config.freq);

	return sizeof(*recv_buffer);
}

static mr_ssize_t mr_timer_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
	mr_timer_t timer = (mr_timer_t)device;
	mr_uint32_t *send_buffer = (mr_uint32_t *)buffer;
	mr_uint32_t period_reload = 0;

	if (size < sizeof(*send_buffer))
	{
		MR_LOG_E(LOG_TAG, "Device %s: Invalid write size %d\r\n", device->object.name, size);
		return - MR_ERR_INVALID;
	}

	if (timer->config.freq == 0)
	{
		MR_LOG_E(LOG_TAG, "Device %s: Invalid frequency %d\r\n", device->object.name, timer->config.freq);
		return - MR_ERR_GENERIC;
	}

	timer->ops->stop(timer);
	period_reload = mr_timer_timeout_calculate(timer, *send_buffer);

	/* When the time is not less than one time, the timer is started */
	if (timer->cycles != 0)
		timer->ops->start(timer, period_reload);

	return sizeof(*send_buffer);
}

static mr_err_t _err_io_timer_configure(mr_timer_t timer, struct mr_timer_config *config)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_err_t _err_io_timer_start(mr_timer_t timer, mr_uint32_t period_reload)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_err_t _err_io_timer_stop(mr_timer_t timer)
{
	MR_ASSERT(0);
	return - MR_ERR_IO;
}

static mr_uint32_t _err_io_timer_get_count(mr_timer_t timer)
{
	MR_ASSERT(0);
	return 0;
}

mr_err_t mr_hw_timer_add(mr_timer_t timer,
						 const char *name,
						 struct mr_timer_ops *ops,
						 struct mr_timer_information *information,
						 void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			mr_timer_open,
			mr_timer_close,
			mr_timer_ioctl,
			mr_timer_read,
			mr_timer_write,
		};

	MR_ASSERT(timer != MR_NULL);
	MR_ASSERT(ops != MR_NULL);
	MR_ASSERT(information != MR_NULL);
	MR_ASSERT(information->max_freq != 0);
	MR_ASSERT(information->max_cut != 0);

	/* Add the timer-device to the container */
	ret = mr_device_add(&timer->device, name, MR_DEVICE_TYPE_TIMER, MR_OPEN_RDWR, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the timer fields */
	timer->config.freq = 0;
	timer->information = *information;
	timer->reload = 0;
	timer->cycles = 0;
	timer->overflow = 0;
	timer->timeout = 0;

	/* Set timer operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_timer_configure;
	ops->start = ops->start ? ops->start : _err_io_timer_start;
	ops->stop = ops->stop ? ops->stop : _err_io_timer_stop;
	ops->get_count = ops->get_count ? ops->get_count : _err_io_timer_get_count;
	timer->ops = ops;

	return MR_ERR_OK;
}

void mr_hw_timer_isr(mr_timer_t timer, mr_uint16_t event)
{
	switch (event & _MR_TIMER_EVENT_MASK)
	{
		case MR_TIMER_EVENT_PIT_INT:
		{
			timer->overflow ++;

			if (timer->cycles != 0)
			{
				timer->cycles --;
			}

			/* Timeout */
			if (timer->cycles == 0)
			{
				timer->cycles = timer->reload;

				if (timer->config.mode == MR_TIMER_MODE_ONE_SHOT)
					timer->ops->stop(timer);

				/* Invoke the rx-cb function */
				if (timer->device.rx_cb != MR_NULL)
					timer->device.rx_cb(&timer->device, MR_NULL);
			}
		}

		default:break;
	}
}

#endif
