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

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)

#define MR_SERIAL_DATA_BITS_8           0
#define MR_SERIAL_DATA_BITS_9           1

#define MR_SERIAL_STOP_BITS_1           0
#define MR_SERIAL_STOP_BITS_1_5         1
#define MR_SERIAL_STOP_BITS_2           2

#define MR_SERIAL_PARITY_NONE           0
#define MR_SERIAL_PARITY_EVEN           1
#define MR_SERIAL_PARITY_ODD            2

#define MR_SERIAL_BIT_ORDER_LSB         0
#define MR_SERIAL_BIT_ORDER_MSB         1

#define MR_SERIAL_NRZ_NORMAL            0
#define MR_SERIAL_NRZ_INVERTED          1

#define MR_SERIAL_EVENT_RX_INT          0x10000000
#define MR_SERIAL_EVENT_TX_INT          0x20000000
#define MR_SERIAL_EVENT_RX_DMA          0x40000000
#define MR_SERIAL_EVENT_TX_DMA          0x80000000
#define _MR_SERIAL_EVENT_MASK           0xf0000000

#define _MR_SERIAL_STATE_IDLE           0x00
#define _MR_SERIAL_STATE_RUNNING        0x10

/* Default config for mr_serial_config structure */
#define MR_SERIAL_CONFIG_DEFAULT        \
{                                       \
    115200,                             \
    MR_SERIAL_DATA_BITS_8,              \
    MR_SERIAL_STOP_BITS_1,              \
    MR_SERIAL_PARITY_NONE,              \
    MR_SERIAL_BIT_ORDER_LSB,            \
    MR_SERIAL_NRZ_NORMAL,               \
}

struct mr_serial_config
{
    mr_uint32_t baud_rate;
    mr_uint8_t data_bits;
    mr_uint8_t stop_bits;
    mr_uint8_t parity;
    mr_uint8_t bit_order;
    mr_uint8_t invert;
};

typedef struct mr_serial *mr_serial_t;

struct mr_serial_ops
{
    mr_err_t (*configure)(mr_serial_t serial, struct mr_serial_config *config);
    void (*write)(mr_serial_t serial, mr_uint8_t data);
    mr_uint8_t (*read)(mr_serial_t serial);

    /* Interrupt */
    void (*start_tx)(mr_serial_t serial);
    void (*stop_tx)(mr_serial_t serial);

    /* DMA */
    void (*start_dma_tx)(mr_serial_t serial, mr_uint8_t *buffer, mr_size_t size);
    void (*stop_dma_tx)(mr_serial_t serial);
};

struct mr_serial
{
    struct mr_device device;

    struct mr_serial_config config;
    mr_size_t rx_bufsz;
    mr_size_t tx_bufsz;
    struct mr_fifo rx_fifo;
    struct mr_fifo tx_fifo;
    mr_uint8_t rx_state;
    mr_uint8_t tx_state;

    mr_uint8_t rx_dma[MR_CONF_SERIAL_RX_DMA_BUFS];
    mr_uint8_t tx_dma[MR_CONF_SERIAL_TX_DMA_BUFS];

    const struct mr_serial_ops *ops;
};

mr_err_t mr_serial_device_add(mr_serial_t serial, const char *name, void *data, struct mr_serial_ops *ops);
void mr_serial_device_isr(mr_serial_t serial, mr_uint32_t event);

#endif /* MR_CONF_SERIAL */

#endif /* _SERIAL_H_ */