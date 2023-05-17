/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-17     MacRsh       first version
 */

#include "motion.h"

#if (MR_CONF_DEVICE_MOTION == MR_CONF_ENABLE)

#undef LOG_TAG
#define LOG_TAG "motion-sensor"

static mr_err_t mr_motion_sensor_open(mr_device_t device)
{
	mr_motion_sensor_t sensor = (mr_motion_sensor_t)device;
	struct mr_motion_sensor_config default_config_accel = MR_MOTION_SENSOR_CONFIG_DEFAULT_ACC;
	struct mr_motion_sensor_config default_config_gyro = MR_MOTION_SENSOR_CONFIG_DEFAULT_GYRO;

	/* Setting motion-sensor to default-config */
	if (sensor->config.range == 0)
	{
		switch (sensor->type)
		{
			case MR_MOTION_SENSOR_TYPE_ACCELEROMETER:
				sensor->config = default_config_accel;
				break;

			case MR_MOTION_SENSOR_TYPE_GYROSCOPE:
				sensor->config = default_config_gyro;
				break;

			default:break;
		}
	}

	/* Initialize the motion-sensor data */
	mr_memset(&sensor->data, 0, sizeof(sensor->data));

	return sensor->ops->configure(sensor, &sensor->config);
}

static mr_err_t mr_motion_sensor_close(mr_device_t device)
{
	mr_motion_sensor_t sensor = (mr_motion_sensor_t)device;

	/* Empty the motion-sensor data */
	mr_memset(&sensor->data, 0, sizeof(sensor->data));

	/* Setting motion-sensor to close-config */
	sensor->config.range = 0;

	return sensor->ops->configure(sensor, &sensor->config);
}

static mr_err_t mr_motion_sensor_ioctl(mr_device_t device, int cmd, void *args)
{
	mr_motion_sensor_t sensor = (mr_motion_sensor_t)device;
	mr_err_t ret = MR_ERR_OK;

	switch (cmd & _MR_CTRL_FLAG_MASK)
	{
		case MR_CTRL_CONFIG:
		{
			if (args)
			{
				ret = sensor->ops->configure(sensor, (struct mr_motion_sensor_config *)args);
				if (ret == MR_ERR_OK)
					sensor->config = *(struct mr_motion_sensor_config *)args;
				return ret;
			}
			return - MR_ERR_INVALID;
		}

		default: return - MR_ERR_UNSUPPORTED;
	}
}

static mr_ssize_t mr_motion_sensor_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
	mr_motion_sensor_t sensor = (mr_motion_sensor_t)device;
	struct mr_motion_sensor_axis *recv_buffer = (struct mr_motion_sensor_axis *)buffer;

	if (size < sizeof(*recv_buffer))
	{
		MR_LOG_E(LOG_TAG, "Device %s: Invalid read size %d\r\n", device->object.name, size);
		return - MR_ERR_INVALID;
	}

	*recv_buffer = sensor->ops->read(sensor);

	return sizeof(*recv_buffer);
}

static mr_err_t _err_io_motion_sensor_configure(mr_motion_sensor_t sensor, struct mr_motion_sensor_config *config)
{
	MR_ASSERT(0);
	return -MR_ERR_IO;
}

static struct mr_motion_sensor_axis _err_io_motion_sensor_read(mr_motion_sensor_t sensor)
{
	struct mr_motion_sensor_axis axis = {0};

	MR_ASSERT(0);
	return axis;
}

mr_err_t mr_hw_motion_sensor_add(mr_motion_sensor_t sensor,
								 const char *name,
								 enum mr_motion_sensor_type type,
								 struct mr_motion_sensor_ops *ops,
								 void *data)
{
	mr_err_t ret = MR_ERR_OK;
	const static struct mr_device_ops device_ops =
		{
			mr_motion_sensor_open,
			mr_motion_sensor_close,
			mr_motion_sensor_ioctl,
			mr_motion_sensor_read,
			MR_NULL,
		};

	MR_ASSERT(sensor != MR_NULL);
	MR_ASSERT(ops != MR_NULL);

	/* Add the motion-sensor to the container */
	ret = mr_device_add(&sensor->device, name, MR_DEVICE_TYPE_SENSOR, MR_OPEN_RDONLY, &device_ops, data);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the motion-sensor fields */
	sensor->type = type;
	sensor->config.range = 0;

	/* Set motion-sensor operations as protect functions if ops is null */
	ops->configure = ops->configure ? ops->configure : _err_io_motion_sensor_configure;
	ops->read = ops->read ? ops->read : _err_io_motion_sensor_read;
	sensor->ops = ops;

	return MR_ERR_OK;
}

#endif