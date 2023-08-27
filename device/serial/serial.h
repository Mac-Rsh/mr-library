/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "mrlib.h"

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

/**
 * @def Serial device data bits
 */
#define MR_SERIAL_DATA_BITS_8           0
#define MR_SERIAL_DATA_BITS_9           1

/**
 * @def Serial device stop bits
 */
#define MR_SERIAL_STOP_BITS_1           0
#define MR_SERIAL_STOP_BITS_1_5         1
#define MR_SERIAL_STOP_BITS_2           2

/**
 * @def Serial device parity
 */
#define MR_SERIAL_PARITY_NONE           0
#define MR_SERIAL_PARITY_EVEN           1
#define MR_SERIAL_PARITY_ODD            2

/**
 * @def Serial device bit order
 */
#define MR_SERIAL_BIT_ORDER_LSB         0
#define MR_SERIAL_BIT_ORDER_MSB         1

/**
 * @def Serial device invert
 */
#define MR_SERIAL_NRZ_NORMAL            0
#define MR_SERIAL_NRZ_INVERTED          1

/**
 * @def Serial device interrupt event
 */
#define MR_SERIAL_EVENT_RX_INT          0x10000000
#define MR_SERIAL_EVENT_TX_INT          0x20000000
#define MR_SERIAL_EVENT_MASK            0xf0000000

/**
 * @def Serial device default config
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
 * @struct Serial device config
 */
struct mr_serial_config
{
    mr_uint32_t baud_rate;
    mr_uint8_t data_bits: 2;
    mr_uint8_t stop_bits: 2;
    mr_uint8_t parity: 2;
    mr_uint8_t bit_order: 1;
    mr_uint8_t invert: 1;
};
typedef struct mr_serial_config *mr_serial_config_t;

typedef struct mr_serial *mr_serial_t;

/**
 * @struct Serial device operations
 */
struct mr_serial_ops
{
    mr_err_t (*configure)(mr_serial_t serial, mr_serial_config_t config);
    void (*write)(mr_serial_t serial, mr_uint8_t data);
    mr_uint8_t (*read)(mr_serial_t serial);

    /* Interrupt send operations */
    void (*start_tx)(mr_serial_t serial);
    void (*stop_tx)(mr_serial_t serial);
};

/**
 * @struct Serial device
 */
struct mr_serial
{
    struct mr_device device;

    struct mr_serial_config config;
    struct mr_rb rx_fifo;
    struct mr_rb tx_fifo;

    const struct mr_serial_ops *ops;
};

/**
 * @addtogroup Serial device
 * @{
 */
mr_err_t mr_serial_device_add(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data);
void mr_serial_device_isr(mr_serial_t serial, mr_uint32_t event);
/** @} */

#endif

#endif /* _SERIAL_H_ */