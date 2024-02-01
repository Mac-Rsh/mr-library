/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#ifndef _MR_SPI_H_
#define _MR_SPI_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SPI

/**
 * @addtogroup SPI
 * @{
 */

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
 * @brief SPI bit order.
 */
#define MR_SPI_BIT_ORDER_LSB            (0)                         /**< LSB first */
#define MR_SPI_BIT_ORDER_MSB            (1)                         /**< MSB first */

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
    MR_SPI_BIT_ORDER_MSB,               \
    MR_SPI_REG_BITS_8,                  \
}

/**
 * @brief SPI configuration structure.
 */
struct mr_spi_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    int host_slave;                                                 /**< Host/slave */
    int mode;                                                       /**< Mode */
    int bit_order;                                                  /**< Bit order */
    int reg_bits;                                                   /**< Register bits */
};

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
 * @brief SPI control command.
 */
#define MR_IOC_SPI_SET_CONFIG           MR_IOC_SCFG                 /**< Set configuration */
#define MR_IOC_SPI_SET_REG              MR_IOC_SPOS                 /**< Set register */
#define MR_IOC_SPI_SET_RD_BUFSZ         MR_IOC_SRBSZ                /**< Set read buffer size */
#define MR_IOC_SPI_CLR_RD_BUF           MR_IOC_CRBD                 /**< Clear read buffer */
#define MR_IOC_SPI_SET_RD_CALL          MR_IOC_SRCB                 /**< Set read callback */
#define MR_IOC_SPI_TRANSFER             (0x01)                      /**< Transfer */

#define MR_IOC_SPI_GET_CONFIG           MR_IOC_GCFG                 /**< Get configuration */
#define MR_IOC_SPI_GET_REG              MR_IOC_GPOS                 /**< Get register */
#define MR_IOC_SPI_GET_RD_BUFSZ         MR_IOC_GRBSZ                /**< Get read buffer size */
#define MR_IOC_SPI_GET_RD_DATASZ        MR_IOC_GRBDSZ               /**< Get read data size */
#define MR_IOC_SPI_GET_RD_CALL          MR_IOC_GRCB                 /**< Get read callback */

/**
 * @brief SPI data type.
 */
typedef uint8_t mr_spi_data_t;                                      /**< SPI read/write data type */

/**
 * @brief SPI ISR events.
 */
#define MR_ISR_SPI_RD_INT               (MR_ISR_RD | (0x01))        /**< Read interrupt */

/**
 * @brief SPI bus structure.
 */
struct mr_spi_bus
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_spi_config config;                                    /**< Configuration */
    volatile void *owner;                                           /**< Owner */
    volatile int hold;                                              /**< Owner hold */
    int cs_desc;                                                    /**< CS descriptor */
};

/**
 * @brief SPI bus operations structure.
 */
struct mr_spi_bus_ops
{
    int (*configure)(struct mr_spi_bus *spi_bus, struct mr_spi_config *config);
    int (*read)(struct mr_spi_bus *spi_bus, uint8_t *data);
    int (*write)(struct mr_spi_bus *spi_bus, uint8_t data);
};

/**
 * @brief SPI CS active level.
 */
#define MR_SPI_CS_ACTIVE_LOW            (0)                         /**< Active low */
#define MR_SPI_CS_ACTIVE_HIGH           (1)                         /**< Active high */
#define MR_SPI_CS_ACTIVE_HARDWARE       (2)                         /**< Hardware */

/**
 * @brief SPI device structure.
 */
struct mr_spi_dev
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_spi_config config;                                    /**< Config */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    int cs_pin;                                                     /**< CS pin */
    int cs_active;                                                  /**< CS active level */
};

int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *path, struct mr_drv *drv);
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *path, int cs_pin, int cs_active);
/** @} */
#endif /* MR_USING_SPI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SPI_H_ */
