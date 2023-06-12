/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-19     MacRsh       first version
 */

#ifndef _ICM20602_H_
#define _ICM20602_H_

#include "device/spi/spi.h"

#if (MR_CONF_SPI == MR_CONF_ENABLE)

#define ICM20602_ACC_RANGE_2G           1
#define ICM20602_ACC_RANGE_4G           2
#define ICM20602_ACC_RANGE_8G           3
#define ICM20602_ACC_RANGE_16G          4

#define ICM20602_GYRO_RANGE_250DPS      1
#define ICM20602_GYRO_RANGE_500DPS      2
#define ICM20602_GYRO_RANGE_1000DPS     3
#define ICM20602_GYRO_RANGE_2000DPS     4

/* Default config for icm20602_config structure */
#define ICM20602_CONFIG_DEFAULT         \
{                                       \
    ICM20602_ACC_RANGE_8G,              \
    ICM20602_GYRO_RANGE_2000DPS,        \
}

#define ICM20602_SMPLRT_DIV             ( 0x19 )
#define ICM20602_CONFIG                 ( 0x1A )
#define ICM20602_GYRO_CONFIG            ( 0x1B )
#define ICM20602_ACCEL_CONFIG           ( 0x1C )
#define ICM20602_ACCEL_CONFIG_2         ( 0x1D )
#define ICM20602_ACCEL_XOUT_H           ( 0x3B )
#define ICM20602_GYRO_XOUT_H            ( 0x43 )
#define ICM20602_PWR_MGMT_1             ( 0x6B )
#define ICM20602_PWR_MGMT_2             ( 0x6C )
#define ICM20602_WHO_AM_I               ( 0x75 )

struct icm20602_3_axis
{
    mr_int16_t x;
    mr_int16_t y;
    mr_int16_t z;
};

struct icm20602_config
{
    mr_uint8_t acc_range;
    mr_uint16_t gyro_range;
};

struct icm20602
{
    struct mr_spi_device spi;

    struct icm20602_config config;
};
typedef struct icm20602 *icm20602_t;

mr_err_t icm20602_init(icm20602_t icm20602, const char *name, mr_uint16_t cs_pin, const char *spi_bus_name);
mr_err_t icm20602_config(icm20602_t icm20602, const struct icm20602_config *config);
struct icm20602_3_axis icm20602_read_acc_3_axis(icm20602_t icm20602);
struct icm20602_3_axis icm20602_read_gyro_3_axis(icm20602_t icm20602);

#endif /* MR_CONF_SPI */

#endif /* _ICM20602_H_ */