/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-09     MacRsh       first version
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <mrlib.h>

#define MR_SERIAL_DATA_BITS_8           8
#define MR_SERIAL_DATA_BITS_9           9

#define MR_SERIAL_STOP_BITS_1           1
#define MR_SERIAL_STOP_BITS_1_5         2
#define MR_SERIAL_STOP_BITS_2           3

#define MR_SERIAL_PARITY_NONE           0
#define MR_SERIAL_PARITY_EVEN           1
#define MR_SERIAL_PARITY_ODD            2

#define MR_SERIAL_BIT_ORDER_LSB         0
#define MR_SERIAL_BIT_ORDER_MSB         1

#define MR_SERIAL_NRZ_NORMAL            0
#define MR_SERIAL_NRZ_INVERTED          1

#define MR_SERIAL_FIFO_MIN_SIZE         MR_SERIAL_FIFO_MIN

#define MR_SERIAL_EVENT_RX_INT          0x1000
#define _MR_SERIAL_EVENT_MASK           0xf000

/* Default config for serial_configure structure */
#define MR_SERIAL_CONFIG_DEFAULT        \
{                                       \
    115200,                             \
    MR_SERIAL_DATA_BITS_8,              \
    MR_SERIAL_STOP_BITS_1,              \
    MR_SERIAL_PARITY_NONE,              \
    MR_SERIAL_BIT_ORDER_LSB,            \
    MR_SERIAL_NRZ_NORMAL,               \
    MR_SERIAL_FIFO_MIN_SIZE             \
}

struct mr_serial_config
{
    mr_uint32_t baud_rate;

    mr_uint32_t data_bits: 4;
    mr_uint32_t stop_bits: 2;
    mr_uint32_t parity: 2;
    mr_uint32_t bit_order: 1;
    mr_uint32_t invert: 1;
    mr_uint32_t fifo_size: 16;
    mr_uint32_t reserved: 6;
};

struct mr_serial_fifo
{
    struct mr_ringbuffer ringbuffer;
    mr_uint8_t pool[];
};

typedef struct mr_serial *mr_serial_t;
struct mr_serial_ops
{
    mr_err_t (*configure)(mr_serial_t serial, struct mr_serial_config *config);
    void (*write_byte)(mr_serial_t serial, mr_uint8_t data);
    mr_uint8_t (*read_byte)(mr_serial_t serial);
};

struct mr_serial
{
    struct mr_device device;

    struct mr_serial_config config;
    void *rx_fifo;
    void *tx_fifo;

    const struct mr_serial_ops *ops;
};

mr_err_t mr_serial_init(mr_serial_t serial, struct mr_serial_config *config);
mr_err_t mr_serial_uninit(mr_serial_t serial);
mr_size_t mr_serial_write_byte(mr_serial_t serial, mr_uint8_t data);
mr_size_t mr_serial_write(mr_serial_t serial, const mr_uint8_t *buffer, mr_size_t count);
mr_size_t mr_serial_read(mr_serial_t serial, mr_uint8_t *buffer, mr_size_t count);
mr_size_t mr_serial_read_byte(mr_serial_t serial, mr_uint8_t *data);

mr_err_t mr_serial_add_to_container(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data);
void mr_hw_serial_isr(mr_serial_t serial, mr_uint16_t event);

#endif
