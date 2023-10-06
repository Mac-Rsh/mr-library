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

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_I2C == MR_CFG_ENABLE)

/**
 * @def I2C device host/slave
 */
#define MR_I2C_HOST                     0
#define MR_I2C_SLAVE                    1

/**
 * @def I2C device address bits
 */
#define MR_I2C_ADDR_BITS_7              7
#define MR_I2C_ADDR_BITS_10             10

/**
 * @def I2C device position bits
 */
#define MR_I2C_POS_BITS_8               8
#define MR_I2C_POS_BITS_16              16
#define MR_I2C_POS_BITS_32              32

/**
 * @def I2C device interrupt event
 */
#define MR_I2C_BUS_EVENT_RX_INT         0x10000000
#define MR_I2C_BUS_EVENT_MASK           0xf0000000

/**
 * @def I2C device default config
 */
#define MR_I2C_CONFIG_DEFAULT           \
{                                       \
    100000,                             \
    MR_I2C_HOST,                        \
    MR_I2C_ADDR_BITS_7,                 \
    MR_I2C_POS_BITS_8,                  \
}

/**
 * @struct I2C device config
 */
struct mr_i2c_config
{
    mr_uint32_t baud_rate;
    mr_uint32_t host_slave: 1;
    mr_uint32_t addr_bits: 4;
    mr_uint32_t pos_bits: 6;
    mr_uint32_t reserved: 21;
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
    struct mr_rb rx_fifo;
    struct mr_rb tx_fifo;
    mr_uint32_t address;
    mr_i2c_bus_t bus;
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
    struct mr_mutex lock;
    mr_i2c_device_t owner;

    const struct mr_i2c_bus_ops *ops;
};

typedef struct mr_soft_i2c_bus *mr_soft_i2c_bus_t;

/**
 * @struct I2C soft bus operations
 */
struct mr_soft_i2c_ops
{
    mr_err_t (*configure)(mr_soft_i2c_bus_t i2c_bus, mr_state_t state);
    void (*scl_write)(mr_soft_i2c_bus_t i2c_bus, mr_level_t level);
    void (*sda_write)(mr_soft_i2c_bus_t i2c_bus, mr_level_t level);
    mr_level_t (*sda_read)(mr_soft_i2c_bus_t i2c_bus);
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
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint32_t address);
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

#ifdef __cplusplus
}
#endif

#endif /* _I2C_H_ */