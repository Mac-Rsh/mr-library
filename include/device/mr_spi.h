/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#ifndef _MR_SPI_H_
#define _MR_SPI_H_

#include <include/mr_api.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_SPI

/**
 * @addtogroup SPI
 * @{
 */

#define MR_SPI_ROLE_MASTER              (0)                         /**< SPI master */
#define MR_SPI_ROLE_SLAVE               (1)                         /**< SPI slave */

#define MR_SPI_MODE_0                   (0)                         /**< CPOL = 0, CPHA = 0 */
#define MR_SPI_MODE_1                   (1)                         /**< CPOL = 0, CPHA = 1 */
#define MR_SPI_MODE_2                   (2)                         /**< CPOL = 1, CPHA = 0 */
#define MR_SPI_MODE_3                   (3)                         /**< CPOL = 1, CPHA = 1 */

#define MR_SPI_BIT_ORDER_MSB            (0)                         /**< MSB first */
#define MR_SPI_BIT_ORDER_LSB            (1)                         /**< LSB first */

#define MR_SPI_DATA_BITS_8              (8)                         /**< 8 bits data */
#define MR_SPI_DATA_BITS_16             (16)                        /**< 16 bits data */
#define MR_SPI_DATA_BITS_32             (32)                        /**< 32 bits data */

#define MR_SPI_REG_BITS_0               (0)                         /**< Disable register */
#define MR_SPI_REG_BITS_8               (8)                         /**< 8 bits register */
#define MR_SPI_REG_BITS_16              (16)                        /**< 16 bits register */
#define MR_SPI_REG_BITS_32              (32)                        /**< 32 bits register */

#define MR_CMD_SPI_CONFIG               MR_CMD_CONFIG               /**< Configuration command */
#define MR_CMD_SPI_REG                  MR_CMD_POS                  /**< Register command */
#define MR_CMD_SPI_RD_FIFO_SIZE         (0x01)                      /**< Read FIFO size command */
#define MR_CMD_SPI_WR_FIFO_SIZE         (0x02)                      /**< Write FIFO size command */
#define MR_CMD_SPI_RD_FIFO_DATA         (0x03)                      /**< Read FIFO data command */
#define MR_CMD_SPI_WR_FIFO_DATA         (0x04)                      /**< Write FIFO data command */
#define MR_CMD_SPI_TRANSFER             (0x05)                      /**< Transfer command */

#define MR_EVENT_SPI_RD_DATA_INT                                                                   \
    (MR_EVENT_RD | MR_EVENT_DATA | (0x01))                          /**< Interrupt on read completion event */
#define MR_EVENT_SPI_WR_DATA_INT                                                                   \
    (MR_EVENT_WR | MR_EVENT_DATA | (0x01))                          /**< Interrupt on write completion event */

#define MR_SPI_CS_ACTIVE_LOW            (0)                         /**< CS active low */
#define MR_SPI_CS_ACTIVE_HIGH           (1)                         /**< CS active high */
#define MR_SPI_CS_ACTIVE_NONE           (2)                         /**< CS active none */

typedef uint8_t mr_spi_data_t;                                      /**< SPI read/write data type */

/**
 * @brief SPI default configuration.
 */
#define MR_SPI_CONFIG_DEFAULT                                                                      \
{                                                                                                  \
    .baud_rate = 3000000,                                                                          \
    .role = MR_SPI_ROLE_MASTER,                                                                    \
    .mode = MR_SPI_MODE_0,                                                                         \
    .bit_order = MR_SPI_BIT_ORDER_MSB,                                                             \
    .data_bits = MR_SPI_DATA_BITS_8,                                                               \
    .reg_bits = MR_SPI_REG_BITS_0,                                                                 \
    .cs_delay = 0,                                                                                 \
}

/**
 * @brief SPI configuration structure.
 */
struct mr_spi_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    uint32_t role;                                                  /**< Role(master/slave) */
    uint32_t mode;                                                  /**< Mode */
    uint32_t bit_order;                                             /**< Bit order */
    uint32_t data_bits;                                             /**< Data bits */
    uint32_t reg_bits;                                              /**< Register bits */
    uint32_t cs_delay;                                              /**< CS delay */
};

/**
 * @brief SPI transfer structure.
 */
struct mr_spi_transfer
{
    void *rbuf;                                                     /**< Read buffer */
    const void *wbuf;                                               /**< Write buffer */
    size_t count;                                                   /**< Transfer size */
};

/**
 * @brief SPI bus structure.
 */
struct mr_spi_bus
{
    struct mr_device device;                                        /**< Device */

    struct mr_spi_config config;                                    /**< Configuration */
    volatile void *owner;                                           /**< Owner */
    uint32_t state;                                                 /**< State */
#ifdef MR_USE_PIN
    int pin_descriptor;                                             /**< Pin device descriptor */
#endif /* MR_USE_PIN */
};

/**
 * @brief SPI bus driver operations structure.
 */
struct mr_spi_bus_driver_ops
{
    int (*configure)(struct mr_driver *driver, bool enable, struct mr_spi_config *config);
    int (*receive)(struct mr_driver *driver, uint32_t *data);
    int (*send)(struct mr_driver *driver, uint32_t data);

    /* Optional operations */
    int (*send_int_configure)(struct mr_driver *driver, bool enable);
    int (*cs_configure)(struct mr_driver *driver, uint32_t pin, uint32_t mode);
    int (*cs_set)(struct mr_driver *driver, uint32_t pin, uint8_t level);
};

/**
 * @brief SPI device structure.
 */
struct mr_spi_device
{
    struct mr_device device;                                        /**< Device */

    struct mr_spi_config config;                                    /**< Configuration */
    struct mr_fifo rfifo;                                           /**< Read FIFO */
    struct mr_fifo wfifo;                                           /**< Write FIFO */
    size_t rfifo_size;                                              /**< Read buffer size */
    size_t wfifo_size;                                              /**< Write buffer size */
    uint32_t cs_pin;                                                /**< CS pin */
    uint32_t cs_active;                                             /**< CS active level */
};

int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *path, struct mr_driver *driver);
int mr_spi_device_register(struct mr_spi_device *spi_device, const char *path, int cs_pin,
                           int cs_active, const char *spi_bus_name);
int mr_spi_device_unregister(struct mr_spi_device *spi_device);

/** @} */

#endif /* MR_USE_SPI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SPI_H_ */
