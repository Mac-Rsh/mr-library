/*
 * @copyright (c) 2023-2024, MR Development Team
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
 * @addtogroup Serial
 * @{
 */

/**
 * @brief Serial data bits.
 */
#define MR_SERIAL_DATA_BITS_5           (5)                         /**< 5 bits data */
#define MR_SERIAL_DATA_BITS_6           (6)                         /**< 6 bits data */
#define MR_SERIAL_DATA_BITS_7           (7)                         /**< 7 bits data */
#define MR_SERIAL_DATA_BITS_8           (8)                         /**< 8 bits data */

/**
 * @brief Serial stop bits.
 */
#define MR_SERIAL_STOP_BITS_1           (1)                         /**< 1 bit stop */
#define MR_SERIAL_STOP_BITS_2           (2)                         /**< 2 bit stop */
#define MR_SERIAL_STOP_BITS_3           (3)                         /**< 3 bit stop */
#define MR_SERIAL_STOP_BITS_4           (4)                         /**< 4 bit stop */

/**
 * @brief Serial parity.
 */
#define MR_SERIAL_PARITY_NONE           (0)                         /**< No parity */
#define MR_SERIAL_PARITY_EVEN           (1)                         /**< Even parity */
#define MR_SERIAL_PARITY_ODD            (2)                         /**< Odd parity */

/**
 * @brief Serial bit order.
 */
#define MR_SERIAL_BIT_ORDER_LSB         (0)                         /**< LSB first */
#define MR_SERIAL_BIT_ORDER_MSB         (1)                         /**< MSB first */

/**
 * @brief Serial polarity.
 */
#define MR_SERIAL_POLARITY_NORMAL       (0)                         /**< Normal polarity */
#define MR_SERIAL_POLARITY_INVERTED     (1)                         /**< Inverted polarity */

/**
 * @brief Serial default configuration.
 */
#define MR_SERIAL_CONFIG_DEFAULT        \
{                                       \
    115200,                             \
    MR_SERIAL_DATA_BITS_8,              \
    MR_SERIAL_STOP_BITS_1,              \
    MR_SERIAL_PARITY_NONE,              \
    MR_SERIAL_BIT_ORDER_LSB,            \
    MR_SERIAL_POLARITY_NORMAL,          \
}

/**
 * @brief Serial configuration structure.
 */
struct mr_serial_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    int data_bits;                                                  /**< Data bits */
    int stop_bits;                                                  /**< Stop bits */
    int parity;                                                     /**< Parity */
    int bit_order;                                                  /**< Bit order */
    int polarity;                                                   /**< Polarity */
};

/**
 * @brief Serial control command.
 */
#define MR_IOC_SERIAL_SET_CONFIG        MR_IOC_SCFG                 /**< Set configuration command */
#define MR_IOC_SERIAL_SET_RD_BUFSZ      MR_IOC_SRBSZ                /**< Set read buffer size command */
#define MR_IOC_SERIAL_SET_WR_BUFSZ      MR_IOC_SWBSZ                /**< Set write buffer size command */
#define MR_IOC_SERIAL_CLR_RD_BUF        MR_IOC_CRBD                 /**< Clear read buffer command */
#define MR_IOC_SERIAL_CLR_WR_BUF        MR_IOC_CWBD                 /**< Clear write buffer command */
#define MR_IOC_SERIAL_SET_RD_CALL       MR_IOC_SRCB                 /**< Set read callback command */
#define MR_IOC_SERIAL_SET_WR_CALL       MR_IOC_SWCB                 /**< Set write callback command */

#define MR_IOC_SERIAL_GET_CONFIG        MR_IOC_GCFG                 /**< Get configuration command */
#define MR_IOC_SERIAL_GET_RD_BUFSZ      MR_IOC_GRBSZ                /**< Get read buffer size command */
#define MR_IOC_SERIAL_GET_WR_BUFSZ      MR_IOC_GWBSZ                /**< Get write buffer size command */
#define MR_IOC_SERIAL_GET_RD_DATASZ     MR_IOC_GRBDSZ               /**< Get read data size command */
#define MR_IOC_SERIAL_GET_WR_DATASZ     MR_IOC_GWBDSZ               /**< Get write data size command */
#define MR_IOC_SERIAL_GET_RD_CALL       MR_IOC_GRCB                 /**< Get read callback command */
#define MR_IOC_SERIAL_GET_WR_CALL       MR_IOC_GWCB                 /**< Get write callback command */

/**
 * @brief Serial data type.
 */
typedef uint8_t mr_serial_data_t;                                   /**< Serial read/write data type */

/**
 * @brief Serial ISR events.
 */
#define MR_ISR_SERIAL_RD_INT            (MR_ISR_RD | (0x01))        /**< Read interrupt */
#define MR_ISR_SERIAL_WR_INT            (MR_ISR_WR | (0x02))        /**< Write interrupt */

/**
 * @brief Serial structure.
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
 * @brief Serial operations structure.
 */
struct mr_serial_ops
{
    int (*configure)(struct mr_serial *serial, struct mr_serial_config *config);
    uint8_t (*read)(struct mr_serial *serial);
    void (*write)(struct mr_serial *serial, uint8_t data);
    void (*start_tx)(struct mr_serial *serial);
    void (*stop_tx)(struct mr_serial *serial);
};

int mr_serial_register(struct mr_serial *serial, const char *path, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERIAL_H_ */
