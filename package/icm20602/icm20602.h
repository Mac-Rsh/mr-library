/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#ifndef _ICM20602_H_
#define _ICM20602_H_

#include "stdint.h"

#define ICM20602_ASSERT(x)

#define ICM20602_ACC_RANGE_2G           1
#define ICM20602_ACC_RANGE_4G           2
#define ICM20602_ACC_RANGE_8G           3
#define ICM20602_ACC_RANGE_16G          4

#define ICM20602_GYRO_RANGE_250DPS      1
#define ICM20602_GYRO_RANGE_500DPS      2
#define ICM20602_GYRO_RANGE_1000DPS     3
#define ICM20602_GYRO_RANGE_2000DPS     4

struct icm20602_3_axis
{
	int16_t x;
	int16_t y;
	int16_t z;
};

typedef struct icm20602 *icm20602_t;
struct icm20602
{
	struct
	{
		void (*write)(icm20602_t icm20602, uint8_t data);
		uint8_t (*read)(icm20602_t icm20602);
		void (*cs_ctrl)(icm20602_t icm20602, uint8_t state);
	} io;

	struct
	{
		uint8_t acc_range;
		uint16_t gyro_range;
	} config;

	void *data;
};

#define ICM20602_ERR_OK					0
#define ICM20602_ERR_SELF_CHECK         1
#define ICM20602_ERR_RESET				2
#define ICM20602_ERR_UNSUPPORTED		3

#define ICM20602_SMPLRT_DIV             (0x19)
#define ICM20602_CONFIG                 (0x1A)
#define ICM20602_GYRO_CONFIG            (0x1B)
#define ICM20602_ACCEL_CONFIG           (0x1C)
#define ICM20602_ACCEL_CONFIG_2         (0x1D)
#define ICM20602_ACCEL_XOUT_H           (0x3B)
#define ICM20602_GYRO_XOUT_H            (0x43)
#define ICM20602_PWR_MGMT_1             (0x6B)
#define ICM20602_PWR_MGMT_2             (0x6C)
#define ICM20602_WHO_AM_I               (0x75)

int icm20602_init(icm20602_t icm20602,
				  void (*write)(icm20602_t icm20602, uint8_t data),
				  uint8_t (*read)(icm20602_t icm20602),
				  void (*cs_ctrl)(icm20602_t icm20602, uint8_t state),
				  void *data);
int icm20602_config(icm20602_t icm20602, uint8_t acc_range, uint16_t gyro_range);

#endif