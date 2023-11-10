/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-09    MacRsh       First version
 */

#ifndef _MR_I2C_H_
#define _MR_I2C_H_

#include "inc/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_I2C

/**
* @brief I2C host/slave.
*/
#define MR_I2C_HOST                    (0)                         /**< I2C host */
#define MR_I2C_SLAVE                   (1)                         /**< I2C slave */

/**
 * @brief I2C offset bits.
 */
#define MR_I2C_OFF_BITS_8              (8)                        /**< 8 bits offset */
#define MR_I2C_OFF_BITS_16             (16)                       /**< 16 bits offset */
#define MR_I2C_OFF_BITS_32             (32)                       /**< 32 bits offset */

/**
 * @brief I2C default configuration.
 */
#define MR_I2C_CONFIG_DEFAULT          \
{                                      \
    100000,                            \
    MR_I2C_HOST,                       \
    MR_I2C_OFF_BITS_8,                 \
}

/**
 * @struct I2C configuration structure.
 */
struct mr_i2c_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    uint32_t host_slave: 1;                                         /**< Host/slave */
    uint32_t off_bits: 6;                                           /**< Offset bits */
    uint32_t reserved: 25;
};

/**
 * @brief I2C bus structure.
 */
struct mr_i2c_bus
{
    struct mr_dev dev;                                             /**< Device */

    struct mr_i2c_config config;                                   /**< Configuration */
    volatile uint32_t lock;                                         /**< Lock */
    void *owner;                                                    /**< Owner */
};

/**
 * @struct I2C bus operations structure.
 */
struct mr_i2c_bus_ops
{
    int (*configure)(struct mr_i2c_bus *i2c_bus, struct mr_i2c_config *config);
    void (*start)(struct mr_i2c_bus *i2c_bus);
    void (*stop)(struct mr_i2c_bus *i2c_bus);
    ssize_t (*write)(struct mr_i2c_bus *i2c_bus, const void *buf, size_t size);
    ssize_t (*read)(struct mr_i2c_bus *i2c_bus, void *buf, size_t size);
};

/**
 * @brief I2C device address bits.
 */
#define MR_I2C_ADDR_BITS_7             (7)                         /**< 7 bit address */
#define MR_I2C_ADDR_BITS_10            (10)                        /**< 10 bit address */

/**
 * @struct I2C device structure.
 */
struct mr_i2c_dev
{
    struct mr_dev dev;                                             /**< Device */

    struct mr_i2c_config config;                                   /**< Configuration */
    struct mr_ringbuf rd_fifo;                                     /**< Read FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    uint32_t addr: 10;                                              /**< Address */
    uint32_t addr_bits: 22;                                         /**< Address bits */
};

/**
 * @addtogroup I2C.
 * @{
 */
int mr_i2c_bus_register(struct mr_i2c_bus *i2c_bus, const char *name, struct mr_drv *drv);
int mr_i2c_dev_register(struct mr_i2c_dev *i2c_dev, const char *name, int addr, int addr_bits);
/** @} */
#endif /* MR_USING_I2C */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_I2C_H_ */
