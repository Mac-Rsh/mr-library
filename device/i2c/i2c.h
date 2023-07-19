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

#if (MR_CONF_I2C == MR_CONF_ENABLE)

#define MR_I2C_HOST                     0
#define MR_I2C_SLAVE                    1

/* Default config for mr_i2c_config structure */
#define MR_I2C_CONFIG_DEFAULT           \
{                                       \
    100000,                             \
    MR_I2C_HOST,                        \
}

struct mr_i2c_config
{
    mr_uint32_t baud_rate;
    mr_uint8_t host_slave;
};

typedef struct mr_i2c_bus *mr_i2c_bus_t;

struct mr_i2c_device
{
    struct mr_device device;

    struct mr_i2c_config config;
    struct mr_i2c_bus *bus;
    mr_uint8_t address;
};
typedef struct mr_i2c_device *mr_i2c_device_t;

struct mr_i2c_bus_ops
{
    mr_err_t (*configure)(mr_i2c_bus_t i2c_bus, struct mr_i2c_config *config);
    void (*start)(mr_i2c_bus_t i2c_bus);
    void (*stop)(mr_i2c_bus_t i2c_bus);
    void (*write)(mr_i2c_bus_t i2c_bus, mr_uint8_t data);
    mr_uint8_t (*read)(mr_i2c_bus_t i2c_bus, mr_state_t ack);
};

struct mr_i2c_bus
{
    struct mr_device device;

    struct mr_i2c_config config;
    struct mr_i2c_device *owner;
    struct mr_mutex lock;

    const struct mr_i2c_bus_ops *ops;
};

typedef struct mr_soft_i2c_bus *mr_soft_i2c_bus_t;

struct mr_soft_i2c_ops
{
    mr_err_t (*configure)(mr_soft_i2c_bus_t i2c_bus);
    void (*scl_write)(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value);
    void (*sda_write)(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value);
    mr_uint8_t (*sda_read)(mr_soft_i2c_bus_t i2c_bus);
};

struct mr_soft_i2c_bus
{
    struct mr_i2c_bus i2c_bus;

    mr_size_t delay;

    const struct mr_soft_i2c_ops *ops;
};

mr_err_t mr_i2c_bus_add(mr_i2c_bus_t i2c_bus, const char *name, void *data, struct mr_i2c_bus_ops *ops);
mr_err_t mr_soft_i2c_bus_add(mr_soft_i2c_bus_t i2c_bus, const char *name, void *data, struct mr_soft_i2c_ops *ops);
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint8_t address);

#endif  /* MR_CONF_I2C */

#endif  /* _I2C_H_ */