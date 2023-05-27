/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-19     MacRsh       first version
 */

#include "icm20602.h"

#undef LOG_TAG
#define LOG_TAG "icm20602"

#if (MR_CONF_SPI == MR_CONF_ENABLE)

static void icm20602_write_reg(icm20602_t icm20602, mr_uint8_t reg, mr_uint8_t data)
{
	mr_device_write(&icm20602->spi.device, reg, &data, sizeof(data));
}

static mr_uint8_t icm20602_read_reg(icm20602_t icm20602, mr_uint8_t reg)
{
	mr_uint8_t data = 0;

	mr_device_read(&icm20602->spi.device, reg | 0x80, &data, sizeof(data));

	return data;
}

static mr_ssize_t icm20602_read_regs(icm20602_t icm20602, mr_uint8_t reg, mr_uint8_t *buffer, mr_size_t size)
{
	mr_device_read(&icm20602->spi.device, reg | 0x80, buffer, size);

	return (mr_ssize_t)size;
}

static mr_bool_t icm20602_self_check(icm20602_t icm20602)
{
	mr_size_t count = 0;

	for (count = 0; count < 200; count ++)
	{
		if (icm20602_read_reg(icm20602, ICM20602_WHO_AM_I) == 0x12)
			return MR_TRUE;
	}

	return MR_FALSE;
}

mr_err_t icm20602_init(icm20602_t icm20602, const char *name, mr_uint16_t cs_pin, const char *spi_bus_name)
{
	mr_err_t ret = MR_ERR_OK;
	struct icm20602_config default_config = ICM20602_CONFIG_DEFAULT;
	struct mr_spi_config spi_config = MR_SPI_CONFIG_DEFAULT;
#if (MR_CONF_PIN == MR_CONF_ENABLE)
	struct mr_pin_config pin_config = {cs_pin,
									   MR_PIN_MODE_OUTPUT};
	mr_device_t pin = MR_NULL;
#endif
	mr_size_t count = 0;

	MR_ASSERT(icm20602 != MR_NULL);
	MR_ASSERT(name != MR_NULL);
	MR_ASSERT(spi_bus_name != MR_NULL);

	ret = mr_hw_spi_device_add(&icm20602->spi, name, MR_OPEN_RDWR, cs_pin, spi_bus_name);
	if (ret != MR_ERR_OK)
	{
		MR_LOG_E(LOG_TAG, "%s failed to add SPI device\r\n", name);
		return ret;
	}

	/* Configure SPI */
	mr_device_open(&icm20602->spi.device, MR_OPEN_RDWR);
	spi_config.baud_rate = 10 * 1000 * 1000;
	mr_device_ioctl(&icm20602->spi.device, MR_CTRL_CONFIG, &spi_config);

#if (MR_CONF_PIN == MR_CONF_ENABLE)
	/* Configure pin */
	pin = mr_device_find("pin");
	if (pin == MR_NULL)
	{
		MR_LOG_E(LOG_TAG, "%s failed to find pin device\r\n", name);
		return - MR_ERR_NOT_FOUND;
	}
	mr_device_open(pin, MR_OPEN_RDWR);
	mr_device_ioctl(pin, MR_CTRL_CONFIG, &pin_config);
#endif

	/* Configure ICM20602 */
	if (icm20602_self_check(icm20602) == MR_FALSE)
	{
		MR_LOG_E(LOG_TAG, "%s failed to self check\r\n", name);
		return - MR_ERR_NOT_FOUND;
	}

	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x80);    /* reset */
	mr_hw_delay_ms(10);
	for (count = 0; count < 200; count ++)
	{
		if (icm20602_read_reg(icm20602, ICM20602_PWR_MGMT_1) == 0x41)
			break;
	}
	if (count == 200)
	{
		MR_LOG_E(LOG_TAG, "%s failed to reset\r\n", name);
		return - MR_ERR_NOT_FOUND;
	}

	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x01);
	icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_2, 0x00);
	icm20602_write_reg(icm20602, ICM20602_CONFIG, 0x01);
	icm20602_write_reg(icm20602, ICM20602_SMPLRT_DIV, 0x07);

	ret = icm20602_config(icm20602, &default_config);
	if (ret != MR_ERR_OK)
	{
		MR_LOG_E(LOG_TAG, "%s failed to configure\r\n", name);
		return ret;
	}

	icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG_2, 0x03);

	return MR_ERR_OK;
}

mr_err_t icm20602_config(icm20602_t icm20602, const struct icm20602_config *config)
{
	MR_ASSERT(icm20602 != MR_NULL);
	MR_ASSERT(config != MR_NULL);

	switch (config->acc_range)
	{
		case ICM20602_ACC_RANGE_2G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x00);
			break;
		case ICM20602_ACC_RANGE_4G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x08);
			break;
		case ICM20602_ACC_RANGE_8G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x10);
			break;
		case ICM20602_ACC_RANGE_16G: icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x18);
			break;

		default: return - MR_ERR_UNSUPPORTED;
	}

	switch (config->gyro_range)
	{
		case ICM20602_GYRO_RANGE_250DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x00);
			break;
		case ICM20602_GYRO_RANGE_500DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x08);
			break;
		case ICM20602_GYRO_RANGE_1000DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x10);
			break;
		case ICM20602_GYRO_RANGE_2000DPS: icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x18);
			break;

		default: return - MR_ERR_UNSUPPORTED;
	}
	icm20602->config = *config;

	return MR_ERR_OK;
}

struct icm20602_3_axis icm20602_read_acc_3_axis(icm20602_t icm20602)
{
	mr_uint8_t buffer[6] = {0};
	struct icm20602_3_axis axis = {0,
								   0,
								   0};

	MR_ASSERT(icm20602 != MR_NULL);

	icm20602_read_regs(icm20602, ICM20602_ACCEL_XOUT_H, buffer, 6);

	axis.x = (mr_int16_t)((buffer[0] << 8) | buffer[1]);
	axis.y = (mr_int16_t)((buffer[2] << 8) | buffer[3]);
	axis.z = (mr_int16_t)((buffer[4] << 8) | buffer[5]);

	return axis;
}

struct icm20602_3_axis icm20602_read_gyro_3_axis(icm20602_t icm20602)
{
	mr_uint8_t buffer[6] = {0};
	struct icm20602_3_axis axis = {0,
								   0,
								   0};

	MR_ASSERT(icm20602 != MR_NULL);

	icm20602_read_regs(icm20602, ICM20602_GYRO_XOUT_H, buffer, 6);

	axis.x = (mr_int16_t)((buffer[0] << 8) | buffer[1]);
	axis.y = (mr_int16_t)((buffer[2] << 8) | buffer[3]);
	axis.z = (mr_int16_t)((buffer[4] << 8) | buffer[5]);

	return axis;
}

#endif