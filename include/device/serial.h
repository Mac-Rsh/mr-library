/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_SERIAL_H_
#define _MR_SERIAL_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SERIAL

/**
 * @brief SERIAL data bits.
 */
#define MR_SERIAL_DATA_BITS_5           (5)                         /**< 5 bits data */
#define MR_SERIAL_DATA_BITS_6           (6)                         /**< 6 bits data */
#define MR_SERIAL_DATA_BITS_7           (7)                         /**< 7 bits data */
#define MR_SERIAL_DATA_BITS_8           (8)                         /**< 8 bits data */

/**
 * @brief SERIAL stop bits.
 */
#define MR_SERIAL_STOP_BITS_1           (1)                         /**< 1 bit stop */
#define MR_SERIAL_STOP_BITS_2           (2)                         /**< 2 bit stop */
#define MR_SERIAL_STOP_BITS_3           (3)                         /**< 3 bit stop */
#define MR_SERIAL_STOP_BITS_4           (4)                         /**< 4 bit stop */

/**
 * @brief SERIAL parity.
 */
#define MR_SERIAL_PARITY_NONE           (0)                         /**< No parity */
#define MR_SERIAL_PARITY_EVEN           (1)                         /**< Even parity */
#define MR_SERIAL_PARITY_ODD            (2)                         /**< Odd parity */

/**
 * @brief SERIAL bit order.
 */
#define MR_SERIAL_BIT_ORDER_LSB         (0)                         /**< LSB first */
#define MR_SERIAL_BIT_ORDER_MSB         (1)                         /**< MSB first */

/**
 * @brief SERIAL polarity.
 */
#define MR_SERIAL_NRZ_NORMAL            (0)                         /**< Normal polarity */
#define MR_SERIAL_NRZ_INVERTED          (1)                         /**< Inverted polarity */

/**
 * @brief SERIAL default configuration.
 */
#define MR_SERIAL_CONFIG_DEFAULT        \
{                                       \
    115200,                             \
    MR_SERIAL_DATA_BITS_8,              \
    MR_SERIAL_STOP_BITS_1,              \
    MR_SERIAL_PARITY_NONE,              \
    MR_SERIAL_BIT_ORDER_LSB,            \
    MR_SERIAL_NRZ_NORMAL,               \
}

/**
 * @brief SERIAL configuration structure.
 */
struct mr_serial_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    uint32_t data_bits: 4;                                          /**< Data bits */
    uint32_t stop_bits: 3;                                          /**< Stop bits */
    uint32_t parity: 2;                                             /**< Parity */
    uint32_t bit_order: 1;                                          /**< Bit order */
    uint32_t invert: 1;                                             /**< Invert */
    uint32_t reserved: 21;
};

/**
 * @brief SERIAL control command.
 */
#define MR_CTL_SERIAL_SET_CONFIG        MR_CTL_SET_CONFIG           /**< Set configuration */
#define MR_CTL_SERIAL_GET_CONFIG        MR_CTL_GET_CONFIG           /**< Get configuration */
#define MR_CTL_SERIAL_SET_RD_BUFSZ      MR_CTL_SET_RD_BUFSZ         /**< Set read buffer size */
#define MR_CTL_SERIAL_GET_RD_BUFSZ      MR_CTL_GET_RD_BUFSZ         /**< Get read buffer size */
#define MR_CTL_SERIAL_SET_WR_BUFSZ      MR_CTL_SET_WR_BUFSZ         /**< Set write buffer size */
#define MR_CTL_SERIAL_GET_WR_BUFSZ      MR_CTL_GET_WR_BUFSZ         /**< Get write buffer size */
#define MR_CTL_SERIAL_SET_RD_CALL       MR_CTL_SET_RD_CALL          /**< Set read callback */
#define MR_CTL_SERIAL_GET_RD_CALL       MR_CTL_GET_RD_CALL          /**< Get read callback */
#define MR_CTL_SERIAL_SET_WR_CALL       MR_CTL_SET_WR_CALL          /**< Set write complete callback */
#define MR_CTL_SERIAL_GET_WR_CALL       MR_CTL_GET_WR_CALL          /**< Get write complete callback */

/**
 * @brief SERIAL data type.
 */
typedef uint8_t mr_serial_data_t;                                   /**< SERIAL read/write data type */

/**
 * @brief SERIAL ISR events.
 */
#define MR_ISR_SERIAL_RD_INT            (MR_ISR_RD | (0x01 << 8))   /**< Read interrupt */
#define MR_ISR_SERIAL_WR_INT            (MR_ISR_WR | (0x02 << 8))   /**< Write interrupt */

/**
 * @brief SERIAL structure.
 */
struct mr_serial
{
    struct mr_dev dev;                                              /**< Device structure */

    struct mr_serial_config config;                                 /**< Configuration */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    struct mr_ringbuf wr_fifo;                                      /**< Write FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    size_t wr_bufsz;                                                /**< Write buffer size */
};

/**
 * @brief SERIAL operations structure.
 */
struct mr_serial_ops
{
    int (*configure)(struct mr_serial *serial, struct mr_serial_config *config);
    uint8_t (*read)(struct mr_serial *serial);
    void (*write)(struct mr_serial *serial, uint8_t data);
    void (*start_tx)(struct mr_serial *serial);
    void (*stop_tx)(struct mr_serial *serial);
};

/**
 * @addtogroup SERIAL.
 * @{
 */
int mr_serial_register(struct mr_serial *serial, const char *name, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERIAL_H_ */
