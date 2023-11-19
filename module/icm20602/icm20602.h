/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#ifndef _ICM20602_H_
#define _ICM20602_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ICM20602

#if !defined(MR_USING_SPI)
#undef MR_USING_ICM20602
#error "Please define MR_USING_SPI. Otherwise ICM20602 will not work."
#else

#include "include/device/spi.h"

/**
 * @brief ICM20602 acc Range.
 */
#define MR_ICM20602_ACC_RANGE_2G        (2)                         /**< 2G */
#define MR_ICM20602_ACC_RANGE_4G        (4)                         /**< 4G */
#define MR_ICM20602_ACC_RANGE_8G        (8)                         /**< 8G */
#define MR_ICM20602_ACC_RANGE_16G       (16)                        /**< 16G */

/**
 * @brief ICM20602 gyro Range.
 */
#define MR_ICM20602_GYRO_RANGE_250DPS   (250)                       /**< 250DPS */
#define MR_ICM20602_GYRO_RANGE_500DPS   (500)                       /**< 500DPS */
#define MR_ICM20602_GYRO_RANGE_1000DPS  (1000)                      /**< 1000DPS */
#define MR_ICM20602_GYRO_RANGE_2000DPS  (2000)                      /**< 2000DPS */

/**
 * @def ICM20602 default config.
 */
#define MR_ICM20602_CONFIG_DEFAULT      \
{                                       \
    MR_ICM20602_ACC_RANGE_8G,           \
    MR_ICM20602_GYRO_RANGE_2000DPS,     \
}

/**
 * @brief ICM20602 config structure.
 */
struct mr_icm20602_config
{
    uint16_t acc_range;                                             /**< Acc range */
    uint16_t gyro_range;                                            /**< Gyro range */
};

/**
 * @brief ICM20602 offset.
 */
#define MR_ICM20602_ACC_OFFSET          (0x00)                      /**< Acc offset */
#define MR_ICM20602_GYRO_OFFSET         (0x01)                      /**< Gyro offset */

/**
 * @brief ICM20602 data type.
 */
typedef struct mr_icm20602_data
{
    int16_t x;                                                      /**< X axis */
    int16_t y;                                                      /**< Y axis */
    int16_t z;                                                      /**< Z axis */
} mr_icm20602_data_t;                                               /**< ICM20602 read data type */

/**
 * @brief ICM20602 structure.
 */
struct mr_icm20602
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_spi_dev spi_dev;                                      /**< SPI device */
    struct mr_icm20602_config config;                               /**< Config */
    int desc;                                                       /**< Descriptor */
};

/**
 * @addtogroup ICM20602.
 * @{
 */
int mr_icm20602_register(struct mr_icm20602 *icm20602, const char *name, int cs_pin, const char *bus_name);
/** @} */
#endif /* MR_USING_SPI */

#endif /* MR_USING_ICM20602 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ICM20602_H_ */
