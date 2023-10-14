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

#if (MR_CFG_ICM20602 == MR_CFG_ENABLE)

#define DEBUG_TAG "icm20602"

/**
 * @def ICM20602 register
 */
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

static void icm20602_write_reg(mr_icm20602_t icm20602, mr_uint8_t reg, mr_uint8_t data)
{
    mr_device_write(&icm20602->spi.device, reg, &data, sizeof(data));
}

static mr_uint8_t icm20602_read_reg(mr_icm20602_t icm20602, mr_uint8_t reg)
{
    mr_uint8_t data = 0;

    mr_device_read(&icm20602->spi.device, reg | 0x80, &data, sizeof(data));
    return data;
}

static mr_ssize_t icm20602_read_regs(mr_icm20602_t icm20602, mr_uint8_t reg, mr_uint8_t *buffer, mr_size_t size)
{
    mr_device_read(&icm20602->spi.device, reg | 0x80, buffer, size);
    return (mr_ssize_t)size;
}

static mr_bool_t icm20602_self_check(mr_icm20602_t icm20602)
{
    mr_size_t count = 0;

    for (count = 0; count < 200; count++)
    {
        if (icm20602_read_reg(icm20602, ICM20602_WHO_AM_I) == 0x12)
        {
            return MR_TRUE;
        }
    }

    return MR_FALSE;
}

/**
 * @brief This function finds a icm20602.
 *
 * @param name The name of the icm20602.
 *
 * @return A pointer to the found icm20602, or MR_NULL if not found.
 */
mr_icm20602_t mr_icm20602_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the icm20602 object from the container */
    return (mr_icm20602_t)mr_object_find(name, Mr_Object_Type_Device);
}

/**
 * @brief This function adds a icm20602 to the container.
 *
 * @param icm20602 The icm20602 to be added.
 * @param name The name of the icm20602.
 * @param cs_number The cs number of the icm20602.
 * @param bus_name The bus name of the icm20602.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_icm20602_add(mr_icm20602_t icm20602, const char *name, mr_uint16_t cs_number, const char *bus_name)
{
    struct mr_icm20602_config default_config = ICM20602_CONFIG_DEFAULT;
    struct mr_spi_config spi_config = MR_SPI_CONFIG_DEFAULT;
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = 0;

    MR_ASSERT(icm20602 != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(bus_name != MR_NULL);

    ret = mr_spi_device_add(&icm20602->spi, name, cs_number);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add spi device failed: [%d]\r\n", name, ret);
        return ret;
    }

    ret = mr_device_ioctl((mr_device_t)&icm20602->spi, MR_DEVICE_CTRL_CONNECT, (char *)bus_name);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] connect [%s] failed: [%d]\r\n", name, bus_name, ret);
        return ret;
    }

    /* Configure SPI */
    mr_device_open(&icm20602->spi.device, MR_DEVICE_OFLAG_RDWR);
    spi_config.baud_rate = 10 * 1000 * 1000;
    mr_device_ioctl(&icm20602->spi.device, MR_DEVICE_CTRL_SET_CONFIG, &spi_config);

    /* Configure ICM20602 */
    if (icm20602_self_check(icm20602) == MR_FALSE)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] self check failed: [%d]\r\n", name, MR_ERR_NOT_FOUND);
        return MR_ERR_NOT_FOUND;
    }

    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x80);
    mr_delay_ms(10);
    for (count = 0; count < 200; count++)
    {
        if (icm20602_read_reg(icm20602, ICM20602_PWR_MGMT_1) == 0x41)
        {
            break;
        }
    }
    if (count == 200)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s failed to reset\r\n", name);
        return MR_ERR_NOT_FOUND;
    }

    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x01);
    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_2, 0x00);
    icm20602_write_reg(icm20602, ICM20602_CONFIG, 0x01);
    icm20602_write_reg(icm20602, ICM20602_SMPLRT_DIV, 0x07);

    ret = mr_icm20602_config(icm20602, &default_config);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s failed to configure\r\n", name);
        return ret;
    }
    icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG_2, 0x03);

    return MR_ERR_OK;
}

/**
 * @brief This function configures the icm20602.
 *
 * @param icm20602 The icm20602 to be configured.
 * @param config The configuration of the icm20602.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_icm20602_config(mr_icm20602_t icm20602, mr_icm20602_config_t config)
{
    MR_ASSERT(icm20602 != MR_NULL);
    MR_ASSERT(config != MR_NULL);

    switch (config->acc_range)
    {
        case MR_ICM20602_ACC_RANGE_2G:
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x00);
            break;
        case MR_ICM20602_ACC_RANGE_4G:
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x08);
            break;
        case MR_ICM20602_ACC_RANGE_8G:
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x10);
            break;
        case MR_ICM20602_ACC_RANGE_16G:
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x18);
            break;

        default:
            return MR_ERR_UNSUPPORTED;
    }

    switch (config->gyro_range)
    {
        case MR_ICM20602_GYRO_RANGE_250DPS:
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x00);
            break;
        case MR_ICM20602_GYRO_RANGE_500DPS:
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x08);
            break;
        case MR_ICM20602_GYRO_RANGE_1000DPS:
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x10);
            break;
        case MR_ICM20602_GYRO_RANGE_2000DPS:
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x18);
            break;

        default:
            return MR_ERR_UNSUPPORTED;
    }
    icm20602->config = *config;

    return MR_ERR_OK;
}

/**
 * @brief This function reads the 3-axis acceleration from the icm20602.
 *
 * @param icm20602 The icm20602 to be read.
 *
 * @return The 3-axis acceleration.
 */
struct mr_icm20602_3_axis mr_icm20602_read_acc_3_axis(mr_icm20602_t icm20602)
{
    mr_uint8_t buffer[6] = {0};
    struct mr_icm20602_3_axis axis = {0,
                                      0,
                                      0};

    MR_ASSERT(icm20602 != MR_NULL);

    icm20602_read_regs(icm20602, ICM20602_ACCEL_XOUT_H, buffer, 6);

    axis.x = (mr_int16_t)((buffer[0] << 8) | buffer[1]);
    axis.y = (mr_int16_t)((buffer[2] << 8) | buffer[3]);
    axis.z = (mr_int16_t)((buffer[4] << 8) | buffer[5]);

    return axis;
}

/**
 * @brief This function reads the 3-axis gyroscope from the icm20602.
 *
 * @param icm20602 The icm20602 to be read.
 *
 * @return The 3-axis gyroscope.
 */
struct mr_icm20602_3_axis mr_icm20602_read_gyro_3_axis(mr_icm20602_t icm20602)
{
    mr_uint8_t buffer[6] = {0};
    struct mr_icm20602_3_axis axis = {0,
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