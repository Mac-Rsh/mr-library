/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _I2C_H_
#define _I2C_H_

#include "mrlib.h"

#if (MR_CFG_I2C == MR_CFG_ENABLE)

/**
 * @def I2C device host/slave
 */
#define MR_I2C_HOST                     0
#define MR_I2C_SLAVE                    1

/**
 * @def I2C device position bits
 */
#define MR_I2C_POS_BITS_8               0
#define MR_I2C_POS_BITS_16              1
#define MR_I2C_POS_BITS_32              2

/**
 * @def I2C device default config
 */
#define MR_I2C_CONFIG_DEFAULT           \
{                                       \
    100000,                             \
    MR_I2C_HOST,                        \
    MR_I2C_POS_BITS_8,                  \
}

/**
 * @struct I2C device config
 */
struct mr_i2c_config
{
    mr_uint32_t baud_rate;
    mr_uint8_t host_slave: 1;
    mr_uint8_t pos_bits: 2;
    mr_uint8_t reserve: 5;
};
typedef struct mr_i2c_config *mr_i2c_config_t;

typedef struct mr_i2c_bus *mr_i2c_bus_t;

/**
 * @struct I2C device
 */
struct mr_i2c_device
{
    struct mr_device device;

    struct mr_i2c_config config;
    mr_i2c_bus_t bus;
    mr_uint8_t address;
};
typedef struct mr_i2c_device *mr_i2c_device_t;

/**
 * @struct I2C bus operations
 */
struct mr_i2c_bus_ops
{
    mr_err_t (*configure)(mr_i2c_bus_t i2c_bus, mr_i2c_config_t config);
    void (*start)(mr_i2c_bus_t i2c_bus);
    void (*stop)(mr_i2c_bus_t i2c_bus);
    void (*write)(mr_i2c_bus_t i2c_bus, mr_uint8_t data);
    mr_uint8_t (*read)(mr_i2c_bus_t i2c_bus, mr_state_t ack);
};

/**
 * @struct I2C bus
 */
struct mr_i2c_bus
{
    struct mr_device device;

    struct mr_i2c_config config;
    mr_i2c_device_t owner;
    struct mr_mutex lock;

    const struct mr_i2c_bus_ops *ops;
};

typedef struct mr_soft_i2c_bus *mr_soft_i2c_bus_t;

/**
 * @struct I2C soft bus operations
 */
struct mr_soft_i2c_ops
{
    mr_err_t (*configure)(mr_soft_i2c_bus_t i2c_bus);
    void (*scl_write)(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value);
    void (*sda_write)(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value);
    mr_uint8_t (*sda_read)(mr_soft_i2c_bus_t i2c_bus);
};

/**
 * @struct I2C soft bus
 */
struct mr_soft_i2c_bus
{
    struct mr_i2c_bus i2c_bus;

    mr_size_t delay;

    const struct mr_soft_i2c_ops *ops;
};

/**
 * @addtogroup I2C device
 * @{
 */
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint8_t address);
/** @} */

/**
 * @addtogroup I2C bus
 * @{
 */
mr_err_t mr_i2c_bus_add(mr_i2c_bus_t i2c_bus, const char *name, struct mr_i2c_bus_ops *ops, void *data);
/** @} */

/**
 * @addtogroup I2C soft bus
 * @{
 */
mr_err_t mr_soft_i2c_bus_add(mr_soft_i2c_bus_t i2c_bus, const char *name, struct mr_soft_i2c_ops *ops, void *data);
/** @} */

#endif

#endif /* _I2C_H_ */