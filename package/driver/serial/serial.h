/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "stdint.h"
#include "malloc.h"

#define SERIAL_ASSERT(x)

#define SERIAL_FIFO_BUFSZ_MIN           32

#define SERIAL_DATA_BITS_8              0
#define SERIAL_DATA_BITS_9              1

#define SERIAL_STOP_BITS_1              0
#define SERIAL_STOP_BITS_1_5            1
#define SERIAL_STOP_BITS_2              2

#define SERIAL_PARITY_NONE              0
#define SERIAL_PARITY_EVEN              1
#define SERIAL_PARITY_ODD               2

#define SERIAL_BIT_ORDER_LSB            0
#define SERIAL_BIT_ORDER_MSB            1

struct serial_config
{
    size_t baud_rate;

    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t bit_order;
};

typedef struct serial *serial_t;

struct serial_ops
{
    int (*configure)(serial_t serial, struct serial_config *config);
    void (*write)(serial_t serial, uint8_t data);
    uint8_t (*read)(serial_t serial);
    void (*start_tx)(serial_t serial);
    void (*stop_tx)(serial_t serial);
};

struct serial
{
    struct serial_config config;
    size_t fifo_bufsz;
    void *rx_fifo;
    void *tx_fifo;

    int (*rx_callback)(serial_t serial, size_t rx_size);
    void *data;

    struct serial_ops *ops;
};

#define SERIAL_ERR_OK                   0
#define SERIAL_ERR_IO                   1
#define SERIAL_ERR_NO_MEMORY            2

#define SERIAL_EVENT_RX_INT             0x1000
#define SERIAL_EVENT_TX_INT             0x2000

int serial_init(serial_t serial, size_t fifo_bufsz, struct serial_ops *ops, void *data);
int serial_configure(serial_t serial, struct serial_config *config);
int serial_mode(serial_t serial,
                size_t baud_rate,
                uint8_t data_bits,
                uint8_t stop_bits,
                uint8_t parity,
                uint8_t bit_order);
uint8_t serial_get(serial_t serial);
void serial_put(serial_t serial, uint8_t data);
size_t serial_read(serial_t serial, void *buffer, size_t size);
size_t serial_write(serial_t serial, const void *buffer, size_t size);
void serial_hw_isr(serial_t serial, uint16_t event);

#endif