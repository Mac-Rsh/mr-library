/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#include "icm20602.h"

static void icm20602_delay(size_t ms)
{
	volatile size_t count = 0;

	while (count < ms * 1000000)
	{
		count ++;
	}
}

static void icm20602_write_reg(icm20602_t icm20602, uint8_t reg, uint8_t data)
{
	icm20602->io.cs_ctrl(icm20602, 0);
	icm20602->io.write(icm20602, reg);
	icm20602->io.write(icm20602, data);
	icm20602->io.cs_ctrl(icm20602, 1);
}

static uint8_t icm20602_read_reg(icm20602_t icm20602, uint8_t reg)
{
	uint8_t data = 0;

	icm20602->io.cs_ctrl(icm20602, 0);
	icm20602->io.write(icm20602, reg | 0x80);
	data = icm20602->io.read(icm20602);
	icm20602->io.cs_ctrl(icm20602, 1);

	return data;
}

static void icm20602_read_regs(icm20602_t icm20602, uint8_t reg, uint8_t *buffer, size_t size)
{
	icm20602->io.cs_ctrl(icm20602, 0);
	icm20602->io.write(icm20602, reg | 0x80);
	while (size --)
	{
		*buffer ++ = icm20602->io.read(icm20602);
	}
	icm20602->io.cs_ctrl(icm20602, 1);
}

static int icm20602_self_check(icm20602_t icm20602)
{
	size_t count = 0;

	for (count = 0; count < 200; count ++)
	{
		if (icm20602_read_reg(icm20602, ICM20602_WHO_AM_I) == 0x12)
			return 1;
	}

	return 0;
}

/**
 * @brief This function initializes the ICM20602.
 *
 * @param icm20602 The ICM20602 to initialize.
 * @param write The write data function for spi.
 * @param read The read data function for spi.
 * @param cs_ctrl The chip select control function for spi.
 * @param data The data for user.
 *
 * @return ICM20602_ERR_OK on success, otherwise an error code.
 */
int icm20602_init(icm20602_t icm20602,
				  void (*write)(icm20602_t icm20602, uint8_t data),
				  uint8_t (*read)(icm20602_t icm20602),
				  void (*cs_ctrl)(icm20602_t icm20602, uint8_t state),
				  void *data)
{
	ICM20602_ASSERT(icm20602 != NULL);
	ICM20602_ASSERT(write != NULL);
	ICM20602_ASSERT(read != NULL);
	ICM20602_ASSERT(cs_ctrl != NULL);

	icm20602->io.write = write;
	icm20602->io.read = read;
	icm20602->io.cs_ctrl = cs_ctrl;
	icm20602->data = data;

	if (icm20602_self_check(icm20602) == 0)
	{
		return - ICM20602_ERR_SELF_CHECK;
	}
	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x80);
	icm20602_delay(10);
	if (icm20602_read_reg(icm20602, ICM20602_PWR_MGMT_1) != 0x41)
	{
		return - ICM20602_ERR_RESET;
	}
	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x01);
	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_2, 0x00);
	icm20602_write_reg(icm20602, ICM20602_CONFIG, 0x01);
	icm20602_write_reg(icm20602, ICM20602_SMPLRT_DIV, 0x07);
	icm20602_config(icm20602, ICM20602_ACC_RANGE_8G, ICM20602_GYRO_RANGE_2000DPS);
	icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG_2, 0x03);
	
	return ICM20602_ERR_OK;
}

/**
 * @brief This function configures the ICM20602.
 *
 * @param icm20602 The ICM20602 to configure.
 * @param acc_range The accelerometer range.
 * @param gyro_range The gyroscope range.
 *
 * @return ICM20602_ERR_OK on success, otherwise an error code.
 */
int icm20602_config(icm20602_t icm20602, uint8_t acc_range, uint16_t gyro_range)
{
	ICM20602_ASSERT(icm20602 != MR_NULL);

	switch (acc_range)
	{
		case ICM20602_ACC_RANGE_2G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x00);
			break;
		case ICM20602_ACC_RANGE_4G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x08);
			break;
		case ICM20602_ACC_RANGE_8G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x10);
			break;
		case ICM20602_ACC_RANGE_16G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x18);
			break;

		default: return - ICM20602_ERR_UNSUPPORTED;
	}

	switch (gyro_range)
	{
		case ICM20602_GYRO_RANGE_250DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x00);
			break;
		case ICM20602_GYRO_RANGE_500DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x08);
			break;
		case ICM20602_GYRO_RANGE_1000DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x10);
			break;
		case ICM20602_GYRO_RANGE_2000DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x18);
			break;

		default: return - ICM20602_ERR_UNSUPPORTED;
	}
	icm20602->config.acc_range = acc_range;
	icm20602->config.gyro_range = gyro_range;

	return ICM20602_ERR_OK;
}

/**
 * @brief This function reads the accelerometer data.
 *
 * @param icm20602 The ICM20602 to read.
 *
 * @return The acceleration data.
 */
struct icm20602_3_axis icm20602_read_acc_3_axis(icm20602_t icm20602)
{
	uint8_t buffer[6] = {0};
	struct icm20602_3_axis axis = {0,
								   0,
								   0};

	ICM20602_ASSERT(icm20602 != NULL);

	icm20602_read_regs(icm20602, ICM20602_ACCEL_XOUT_H, buffer, 6);

	axis.x = (int16_t)((buffer[0] << 8) | buffer[1]);
	axis.y = (int16_t)((buffer[2] << 8) | buffer[3]);
	axis.z = (int16_t)((buffer[4] << 8) | buffer[5]);

	return axis;
}

/**
 * @brief This function reads the gyroscope data.
 *
 * @param icm20602 The ICM20602 to read.
 *
 * @return The gyroscope data.
 */
struct icm20602_3_axis icm20602_read_gyro_3_axis(icm20602_t icm20602)
{
	uint8_t buffer[6] = {0};
	struct icm20602_3_axis axis = {0,
								   0,
								   0};

	ICM20602_ASSERT(icm20602 != NULL);

	icm20602_read_regs(icm20602, ICM20602_GYRO_XOUT_H, buffer, 6);

	axis.x = (int16_t)((buffer[0] << 8) | buffer[1]);
	axis.y = (int16_t)((buffer[2] << 8) | buffer[3]);
	axis.z = (int16_t)((buffer[4] << 8) | buffer[5]);

	return axis;
}