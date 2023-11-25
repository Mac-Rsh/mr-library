/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#ifndef _MR_SPI_H_
#define _MR_SPI_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SPI

/**
 * @brief SPI host/slave.
 */
#define MR_SPI_HOST                     (0)                         /**< SPI host */
#define MR_SPI_SLAVE                    (1)                         /**< SPI slave */

/**
 * @brief SPI mode.
 */
#define MR_SPI_MODE_0                   (0)                         /**< CPOL = 0, CPHA = 0 */
#define MR_SPI_MODE_1                   (1)                         /**< CPOL = 0, CPHA = 1 */
#define MR_SPI_MODE_2                   (2)                         /**< CPOL = 1, CPHA = 0 */
#define MR_SPI_MODE_3                   (3)                         /**< CPOL = 1, CPHA = 1 */

/**
 * @brief SPI data bits.
 */
#define MR_SPI_DATA_BITS_8              (8)                         /**< 8 bits data */
#define MR_SPI_DATA_BITS_16             (16)                        /**< 16 bits data */
#define MR_SPI_DATA_BITS_32             (32)                        /**< 32 bits data */

/**
 * @brief SPI bit order.
 */
#define MR_SPI_BIT_ORDER_MSB            (0)                         /**< MSB first */
#define MR_SPI_BIT_ORDER_LSB            (1)                         /**< LSB first */

/**
 * @brief SPI register bits.
 */
#define MR_SPI_REG_BITS_8               (8)                         /**< 8 bits register */
#define MR_SPI_REG_BITS_16              (16)                        /**< 16 bits register */
#define MR_SPI_REG_BITS_32              (32)                        /**< 32 bits register */

/**
 * @brief SPI default configuration.
 */
#define MR_SPI_CONFIG_DEFAULT           \
{                                       \
    3000000,                            \
    MR_SPI_HOST,                        \
    MR_SPI_MODE_0,                      \
    MR_SPI_DATA_BITS_8,                 \
    MR_SPI_BIT_ORDER_MSB,               \
    MR_SPI_REG_BITS_8,                  \
}

/**
 * @brief SPI configuration structure.
 */
struct mr_spi_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    uint32_t host_slave: 1;                                         /**< Host/slave */
    uint32_t mode: 2;                                               /**< Mode */
    uint32_t data_bits: 6;                                          /**< Data bits */
    uint32_t bit_order: 1;                                          /**< Bit order */
    uint32_t reg_bits: 6;                                           /**< Register bits */
    uint32_t reserved: 16;
};

/**
 * @brief SPI transfer command.
 */
#define MR_CTRL_SPI_TRANSFER            ((0x01|0x80) << 16)         /**< Transfer */

/**
 * @brief SPI transfer structure.
 */
struct mr_spi_transfer
{
    void *rd_buf;                                                   /**< Read buffer */
    const void *wr_buf;                                             /**< Write buffer */
    size_t size;                                                    /**< Transfer size */
};

/**
 * @brief SPI register command.
 */
#define MR_CTRL_SPI_SET_REG             MR_CTRL_SET_OFFSET          /**< Set register */
#define MR_CTRL_SPI_GET_REG             MR_CTRL_GET_OFFSET          /**< Get register */

/**
 * @brief SPI data type.
 */
typedef uint8_t mr_spi_data_t;                                      /**< SPI read/write data type */

/**
 * @brief SPI ISR events.
 */
#define MR_ISR_SPI_RD_INT               (MR_ISR_RD | (0x01 << 16))  /**< Read interrupt */

/**
 * @brief SPI bus structure.
 */
struct mr_spi_bus
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_spi_config config;                                    /**< Configuration */
    volatile void *owner;                                           /**< Owner */
    volatile int hold;                                              /**< Owner hold */
};

/**
 * @brief SPI bus operations structure.
 */
struct mr_spi_bus_ops
{
    int (*configure)(struct mr_spi_bus *spi_bus, struct mr_spi_config *config);
    uint32_t (*read)(struct mr_spi_bus *spi_bus);
    void (*write)(struct mr_spi_bus *spi_bus, uint32_t data);
};

/**
 * @brief SPI CS active level.
 */
#define MR_SPI_CS_ACTIVE_LOW            (0)                         /**< Active low */
#define MR_SPI_CS_ACTIVE_HIGH           (1)                         /**< Active high */
#define MR_SPI_CS_ACTIVE_NONE           (2)                         /**< No active */

/**
 * @brief SPI device structure.
 */
struct mr_spi_dev
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_spi_config config;                                    /**< Config */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    uint32_t cs_pin: 30;                                            /**< CS pin */
    uint32_t cs_active: 2;                                          /**< CS active level */
    int cs_desc;                                                    /**< CS descriptor */
};

/**
 * @addtogroup SPI.
 * @{
 */
int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *name, struct mr_drv *drv);
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *name, int cs_pin, int cs_active);
/** @} */
#endif /* MR_USING_SPI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SPI_H_ */
