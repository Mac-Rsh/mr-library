/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-09    MacRsh       First version
 */

#ifndef _MR_I2C_H_
#define _MR_I2C_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_I2C

/**
* @brief I2C host/slave.
*/
#define MR_I2C_HOST                     (0)                         /**< I2C host */
#define MR_I2C_SLAVE                    (1)                         /**< I2C slave */

/**
 * @brief I2C register bits.
 */
#define MR_I2C_REG_BITS_8               (8)                         /**< 8 bits register */
#define MR_I2C_REG_BITS_16              (16)                        /**< 16 bits register */
#define MR_I2C_REG_BITS_32              (32)                        /**< 32 bits register */

/**
 * @brief I2C default configuration.
 */
#define MR_I2C_CONFIG_DEFAULT           \
{                                       \
    100000,                             \
    MR_I2C_HOST,                        \
    MR_I2C_REG_BITS_8,                  \
}

/**
 * @brief I2C configuration structure.
 */
struct mr_i2c_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    uint32_t host_slave: 1;                                         /**< Host/slave */
    uint32_t reg_bits: 6;                                           /**< Register bits */
    uint32_t reserved: 25;
};

/**
 * @brief I2C control command.
 */
#define MR_CTL_I2C_SET_CONFIG           MR_CTL_SET_CONFIG           /**< Set configuration */
#define MR_CTL_I2C_SET_REG              MR_CTL_SET_OFFSET           /**< Set register */
#define MR_CTL_I2C_SET_RD_BUFSZ         MR_CTL_SET_RD_BUFSZ         /**< Set read buffer size */
#define MR_CTL_I2C_CLR_RD_BUF           MR_CTL_CLR_RD_BUF           /**< Clear read buffer */
#define MR_CTL_I2C_SET_RD_CALL          MR_CTL_SET_RD_CALL          /**< Set read callback */

#define MR_CTL_I2C_GET_CONFIG           MR_CTL_GET_CONFIG           /**< Get configuration */
#define MR_CTL_I2C_GET_REG              MR_CTL_GET_OFFSET           /**< Get register */
#define MR_CTL_I2C_GET_RD_BUFSZ         MR_CTL_GET_RD_BUFSZ         /**< Get read buffer size */
#define MR_CTL_I2C_GET_RD_DATASZ        MR_CTL_GET_RD_DATASZ        /**< Get read data size */
#define MR_CTL_I2C_GET_RD_CALL          MR_CTL_GET_RD_CALL          /**< Get read callback */

/**
 * @brief I2C data type.
 */
typedef uint8_t mr_i2c_data_t;                                      /**< I2C read/write data type */

/**
 * @brief I2C ISR events.
 */
#define MR_ISR_I2C_RD_INT               (MR_ISR_RD | (0x01 << 8))   /**< Read interrupt */

/**
 * @brief I2C bus structure.
 */
struct mr_i2c_bus
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_i2c_config config;                                    /**< Configuration */
    volatile void *owner;                                           /**< Owner */
    volatile int hold;                                              /**< Owner hold */
};

/**
 * @brief I2C bus operations structure.
 */
struct mr_i2c_bus_ops
{
    int (*configure)(struct mr_i2c_bus *i2c_bus, struct mr_i2c_config *config, int addr, int addr_bits);
    void (*start)(struct mr_i2c_bus *i2c_bus);
    void (*send_addr)(struct mr_i2c_bus *i2c_bus, int addr, int addr_bits);
    void (*stop)(struct mr_i2c_bus *i2c_bus);
    uint8_t (*read)(struct mr_i2c_bus *i2c_bus, int ack_state);
    void (*write)(struct mr_i2c_bus *i2c_bus, uint8_t data);
};

/**
 * @brief I2C device address bits.
 */
#define MR_I2C_ADDR_BITS_7              (7)                         /**< 7 bit address */
#define MR_I2C_ADDR_BITS_10             (10)                        /**< 10 bit address */

/**
 * @brief I2C device structure.
 */
struct mr_i2c_dev
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_i2c_config config;                                    /**< Configuration */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    uint32_t addr: 16;                                              /**< Address */
    uint32_t addr_bits: 16;                                         /**< Address bits */
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
