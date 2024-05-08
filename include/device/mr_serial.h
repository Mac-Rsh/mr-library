/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 * @date 2024-05-08    MacRsh       Added support for DMA
 */

#ifndef _MR_SERIAL_H_
#define _MR_SERIAL_H_

#include "../mr-library/include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_SERIAL

/**
 * @addtogroup Serial
 * @{
 */

#define MR_SERIAL_DATA_BITS_5           (5)                 /**< 5 bits data */
#define MR_SERIAL_DATA_BITS_6           (6)                 /**< 6 bits data */
#define MR_SERIAL_DATA_BITS_7           (7)                 /**< 7 bits data */
#define MR_SERIAL_DATA_BITS_8           (8)                 /**< 8 bits data */

#define MR_SERIAL_STOP_BITS_1           (1)                 /**< 1 bit stop */
#define MR_SERIAL_STOP_BITS_2           (2)                 /**< 2 bit stop */
#define MR_SERIAL_STOP_BITS_3           (3)                 /**< 3 bit stop */
#define MR_SERIAL_STOP_BITS_4           (4)                 /**< 4 bit stop */

#define MR_SERIAL_PARITY_NONE           (0)                 /**< No parity */
#define MR_SERIAL_PARITY_EVEN           (1)                 /**< Even parity */
#define MR_SERIAL_PARITY_ODD            (2)                 /**< Odd parity */

#define MR_SERIAL_BITS_ORDER_LSB        (0)                 /**< LSB first */
#define MR_SERIAL_BITS_ORDER_MSB        (1)                 /**< MSB first */

#define MR_SERIAL_POLARITY_NORMAL       (0)                 /**< Normal polarity */
#define MR_SERIAL_POLARITY_INVERTED     (1)                 /**< Inverted polarity */

/**
 * @brief Serial default configuration.
 */
#define MR_SERIAL_CONFIG_DEFAULT                                               \
{                                                                              \
    .baud_rate = 115200,                                                       \
    .data_bits = MR_SERIAL_DATA_BITS_8,                                        \
    .stop_bits = MR_SERIAL_STOP_BITS_1,                                        \
    .parity = MR_SERIAL_PARITY_NONE,                                           \
    .bits_order = MR_SERIAL_BITS_ORDER_LSB,                                    \
    .polarity = MR_SERIAL_POLARITY_NORMAL,                                     \
}

#define MR_CMD_SERIAL_CONFIG            MR_CMD_CONFIG       /**< Configuration command */
#define MR_CMD_SERIAL_RD_FIFO_SIZE      (0x01)              /**< Read FIFO size command */
#define MR_CMD_SERIAL_WR_FIFO_SIZE      (0x02)              /**< Write FIFO size command */
#define MR_CMD_SERIAL_RD_FIFO_DATA      (0x03)              /**< Read FIFO data command */
#define MR_CMD_SERIAL_WR_FIFO_DATA      (0x04)              /**< Write FIFO data command */

#define MR_EVENT_SERIAL_RD_COMPLETE_INT                                        \
    MR_EVENT_RD_COMPLETE                                    /**< Interrupt on read completion event */
#define MR_EVENT_SERIAL_WR_COMPLETE_INT                                        \
    MR_EVENT_WR_COMPLETE                                    /**< Interrupt on write completion event */
#define MR_EVENT_SERIAL_RD_COMPLETE_DMA                                        \
    (MR_EVENT_RD_COMPLETE | 0x01)                           /**< Interrupt on read DMA completion event */

typedef uint8_t mr_serial_data_t;                           /**< Serial read/write data type */

/**
 * @brief Serial configuration structure.
 */
struct mr_serial_config
{
    uint32_t baud_rate;                                     /**< Baud rate */
    uint32_t data_bits;                                     /**< Data bits */
    uint32_t stop_bits;                                     /**< Stop bits */
    uint32_t parity;                                        /**< Parity */
    uint32_t bits_order;                                    /**< Bits order */
    uint32_t polarity;                                      /**< Polarity */
};

/**
 * @brief Serial structure.
 */
struct mr_serial
{
    struct mr_device device;                                /**< Device */

    struct mr_serial_config config;                         /**< Configuration */
    struct mr_fifo rfifo;                                   /**< Read FIFO */
    struct mr_fifo wfifo;                                   /**< Write FIFO */
    size_t rfifo_size;                                      /**< Read buffer size */
    size_t wfifo_size;                                      /**< Write buffer size */
    uint32_t state;                                         /**< Transmission state */
#ifdef MR_USE_SERIAL_DMA
#ifndef MR_CFG_SERIAL_RD_DMA_FIFO_SIZE
#define MR_CFG_SERIAL_RD_DMA_FIFO_SIZE  (128)
#endif /* MR_CFG_SERIAL_RD_DMA_FIFO_SIZE */
#ifndef MR_CFG_SERIAL_WR_DMA_FIFO_SIZE
#define MR_CFG_SERIAL_WR_DMA_FIFO_SIZE  (128)
#endif /* MR_CFG_SERIAL_WR_DMA_FIFO_SIZE */
    uint8_t rdma[MR_CFG_SERIAL_RD_DMA_FIFO_SIZE];           /**< Read DMA buffer */
    uint8_t wdma[MR_CFG_SERIAL_WR_DMA_FIFO_SIZE];           /**< Write DMA buffer */
#endif /* MR_USE_SERIAL_DMA */
};

/**
 * @brief Serial driver operations structure.
 */
struct mr_serial_driver_ops
{
    int (*configure)(struct mr_driver *driver, bool enable,
                     struct mr_serial_config *config);
    int (*receive)(struct mr_driver *driver, uint8_t *data);
    int (*send)(struct mr_driver *driver, uint8_t data);

    /* Optional operations */
    int (*send_int_configure)(struct mr_driver *driver, bool enable);
    int (*receive_dma)(struct mr_driver *driver, bool enable, void *buf,
                       size_t count);
    int (*send_dma)(struct mr_driver *driver, bool enable, const void *buf,
                    size_t count);
};

int mr_serial_register(struct mr_serial *serial, const char *path,
                       const struct mr_driver *driver);

/** @} */

#endif /* MR_USE_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERIAL_H_ */
