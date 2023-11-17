/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_UART_H_
#define _MR_UART_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_UART

/**
 * @brief UART data bits.
 */
#define MR_UART_DATA_BITS_5             (5)                         /**< 5 bits data */
#define MR_UART_DATA_BITS_6             (6)                         /**< 6 bits data */
#define MR_UART_DATA_BITS_7             (7)                         /**< 7 bits data */
#define MR_UART_DATA_BITS_8             (8)                         /**< 8 bits data */
#define MR_UART_DATA_BITS_9             (9)                         /**< 9 bits data */

/**
 * @brief UART stop bits.
 */
#define MR_UART_STOP_BITS_1             (1)                         /**< 1 bit stop */
#define MR_UART_STOP_BITS_2             (2)                         /**< 2 bit stop */
#define MR_UART_STOP_BITS_3             (3)                         /**< 3 bit stop */
#define MR_UART_STOP_BITS_4             (4)                         /**< 4 bit stop */

/**
 * @brief UART parity.
 */
#define MR_UART_PARITY_NONE             (0)                         /**< No parity */
#define MR_UART_PARITY_EVEN             (1)                         /**< Even parity */
#define MR_UART_PARITY_ODD              (2)                         /**< Odd parity */

/**
 * @brief UART bit order.
 */
#define MR_UART_BIT_ORDER_LSB           (0)                         /**< LSB first */
#define MR_UART_BIT_ORDER_MSB           (1)                         /**< MSB first */

/**
 * @brief UART polarity.
 */
#define MR_UART_NRZ_NORMAL              (0)                         /**< Normal polarity */
#define MR_UART_NRZ_INVERTED            (1)                         /**< Inverted polarity */

/**
 * @brief UART default configuration.
 */
#define MR_UART_CONFIG_DEFAULT          \
{                                       \
    115200,                             \
    MR_UART_DATA_BITS_8,                \
    MR_UART_STOP_BITS_1,                \
    MR_UART_PARITY_NONE,                \
    MR_UART_BIT_ORDER_LSB,              \
    MR_UART_NRZ_NORMAL,                 \
}

/**
 * @brief UART configuration structure.
 */
struct mr_uart_config
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
 * @brief UART structure.
 */
struct mr_uart
{
    struct mr_dev dev;                                              /**< Device structure */

    struct mr_uart_config config;                                   /**< Configuration */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    struct mr_ringbuf wr_fifo;                                      /**< Write FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    size_t wr_bufsz;                                                /**< Write buffer size */
};

/**
 * @brief UART operations structure.
 */
struct mr_uart_ops
{
    int (*configure)(struct mr_uart *uart, struct mr_uart_config *config);
    ssize_t (*read)(struct mr_uart *uart, uint8_t *buf, size_t size);
    ssize_t (*write)(struct mr_uart *uart, const uint8_t *buf, size_t size);
    void (*start_tx)(struct mr_uart *uart);
    void (*stop_tx)(struct mr_uart *uart);
};

/**
 * @addtogroup UART.
 * @{
 */
int mr_uart_register(struct mr_uart *uart, const char *name, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_UART */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_UART_H_ */
