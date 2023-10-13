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

#include "device/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_SPI != MR_CFG_ENABLE)
#error "Please enable SPI first!"
#else
#define MR_CONF_ICM20602 MR_CFG_ENABLE
#if (MR_CONF_ICM20602 == MR_CFG_ENABLE)

/**
 * @def ICM20602 Acc Range
 */
#define ICM20602_ACC_RANGE_2G           2
#define ICM20602_ACC_RANGE_4G           4
#define ICM20602_ACC_RANGE_8G           8
#define ICM20602_ACC_RANGE_16G          16

/**
 * @def ICM20602 Gyro Range
 */
#define ICM20602_GYRO_RANGE_250DPS      250
#define ICM20602_GYRO_RANGE_500DPS      500
#define ICM20602_GYRO_RANGE_1000DPS     1000
#define ICM20602_GYRO_RANGE_2000DPS     2000

/**
 * @def ICM20602 default config
 */
#define ICM20602_CONFIG_DEFAULT         \
{                                       \
    ICM20602_ACC_RANGE_8G,              \
    ICM20602_GYRO_RANGE_2000DPS,        \
}

/**
 * @def ICM20602 3-axis
 */
struct mr_icm20602_3_axis
{
    mr_int16_t x;
    mr_int16_t y;
    mr_int16_t z;
    mr_int16_t reserved;
};

/**
 * @def ICM20602 config
 */
struct mr_icm20602_config
{
    mr_uint16_t acc_range;
    mr_uint16_t gyro_range;
};
typedef struct mr_icm20602_config *mr_icm20602_config_t;

/**
 * @def ICM20602
 */
struct mr_icm20602
{
    struct mr_spi_device spi;

    struct mr_icm20602_config config;
};
typedef struct mr_icm20602 *mr_icm20602_t;

/**
 * @addtogroup ICM20602
 * @{
 */
mr_err_t mr_icm20602_add(mr_icm20602_t icm20602, const char *name, mr_uint16_t cs_number, const char *bus_name);
mr_err_t mr_icm20602_config(mr_icm20602_t icm20602, mr_icm20602_config_t config);
struct mr_icm20602_3_axis mr_icm20602_read_acc_3_axis(mr_icm20602_t icm20602);
struct mr_icm20602_3_axis mr_icm20602_read_gyro_3_axis(mr_icm20602_t icm20602);
/** @} */

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ICM20602_H_ */