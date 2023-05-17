/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-17     MacRsh       first version
 */

#ifndef _MOTION_H_
#define _MOTION_H_

#include "mrlib.h"

#if (MR_CONF_DEVICE_MOTION == MR_CONF_ENABLE)

#define MR_MOTION_SENSOR_RANGE_ACC_2G                2
#define MR_MOTION_SENSOR_RANGE_ACC_4G                4
#define MR_MOTION_SENSOR_RANGE_ACC_8G                8
#define MR_MOTION_SENSOR_RANGE_ACC_16G               16

#define MR_MOTION_SENSOR_RANGE_GYRO_250DPS           250
#define MR_MOTION_SENSOR_RANGE_GYRO_500DPS           500
#define MR_MOTION_SENSOR_RANGE_GYRO_1000DPS          1000
#define MR_MOTION_SENSOR_RANGE_GYRO_2000DPS          2000

#define MR_MOTION_SENSOR_SAMPLE_RATE_10HZ            10
#define MR_MOTION_SENSOR_SAMPLE_RATE_20HZ            20
#define MR_MOTION_SENSOR_SAMPLE_RATE_50HZ            50
#define MR_MOTION_SENSOR_SAMPLE_RATE_100HZ           100
#define MR_MOTION_SENSOR_SAMPLE_RATE_200HZ           200
#define MR_MOTION_SENSOR_SAMPLE_RATE_500HZ           500
#define MR_MOTION_SENSOR_SAMPLE_RATE_1000HZ          1000

/* Default config for mr_motion_sensor_config structure */
#define MR_MOTION_SENSOR_CONFIG_DEFAULT_ACC          \
{                                                    \
	MR_MOTION_SENSOR_RANGE_ACC_2G,                   \
    MR_MOTION_SENSOR_SAMPLE_RATE_500HZ               \
}

#define MR_MOTION_SENSOR_CONFIG_DEFAULT_GYRO         \
{                                                    \
	MR_MOTION_SENSOR_RANGE_GYRO_250DPS,              \
	MR_MOTION_SENSOR_SAMPLE_RATE_500HZ               \
}

enum mr_motion_sensor_type
{
	MR_MOTION_SENSOR_TYPE_ACCELEROMETER,
	MR_MOTION_SENSOR_TYPE_GYROSCOPE,
	MR_MOTION_SENSOR_TYPE_MAGNETOMETER,
};

struct mr_motion_sensor_config
{
	mr_uint32_t range;
	mr_uint32_t sample_rate;
};

struct mr_motion_sensor_axis
{
	mr_int32_t x;
	mr_int32_t y;
	mr_int32_t z;
};

typedef struct mr_motion_sensor *mr_motion_sensor_t;
struct mr_motion_sensor_ops
{
	mr_err_t (*configure)(mr_motion_sensor_t sensor, struct mr_motion_sensor_config *config);
	struct mr_motion_sensor_axis (*read)(mr_motion_sensor_t sensor);
};

struct mr_motion_sensor
{
	struct mr_device device;

	enum mr_motion_sensor_type type;
	struct mr_motion_sensor_config config;
	struct mr_motion_sensor_axis data;

	const struct mr_motion_sensor_ops *ops;
};

mr_err_t mr_hw_motion_sensor_add(mr_motion_sensor_t sensor,
								 const char *name,
								 enum mr_motion_sensor_type type,
								 struct mr_motion_sensor_ops *ops,
								 void *data);

#endif

#endif