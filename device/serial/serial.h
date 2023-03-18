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

#if (MR_DEVICE_SERIAL == MR_CONF_ENABLE)

#define MR_SERIAL_DATA_BITS_8                       0
#define MR_SERIAL_DATA_BITS_9                       1

#define MR_SERIAL_STOP_BITS_1                       0
#define MR_SERIAL_STOP_BITS_1_5                     1
#define MR_SERIAL_STOP_BITS_2                       2

#define MR_SERIAL_PARITY_NONE                       0
#define MR_SERIAL_PARITY_EVEN                       1
#define MR_SERIAL_PARITY_ODD                        2

#define MR_SERIAL_BIT_ORDER_LSB                     0
#define MR_SERIAL_BIT_ORDER_MSB                     1

#define MR_SERIAL_NRZ_NORMAL                        0
#define MR_SERIAL_NRZ_INVERTED                      1

#define MR_SERIAL_FLOW_CONTROL_NONE                 0
#define MR_SERIAL_FLOW_CONTROL_CTSRTS               1

#define MR_SERIAL_FIFO_SIZE_MIN                     32
#if MR_SERIAL_FIFO_SIZE < MR_SERIAL_FIFO_SIZE_MIN
#define MR_SERIAL_FIFO_SIZE         	MR_SERIAL_FIFO_SIZE_MIN
#endif

#define MR_SERIAL_EVENT_RX_INT                      0x1000
#define MR_SERIAL_EVENT_TX_INT                      0x2000
#define _MR_SERIAL_EVENT_MASK                       0xf000

/* Default config for mr_serial_config structure */
#define MR_SERIAL_CONFIG_DEFAULT                    \
{                                                   \
    .baud_rate = 115200,                            \
    .data_bits = MR_SERIAL_DATA_BITS_8,             \
    .stop_bits = MR_SERIAL_STOP_BITS_1,             \
    .parity = MR_SERIAL_PARITY_NONE,                \
    .bit_order = MR_SERIAL_BIT_ORDER_LSB,           \
    .invert = MR_SERIAL_NRZ_NORMAL,                 \
    .flow_control = MR_SERIAL_FLOW_CONTROL_NONE     \
}

struct mr_serial_config
{
	mr_uint32_t baud_rate;

	mr_uint16_t data_bits: 4;
	mr_uint16_t stop_bits: 2;
	mr_uint16_t parity: 2;
	mr_uint16_t bit_order: 1;
	mr_uint16_t invert: 1;
	mr_uint16_t flow_control: 1;
	mr_uint16_t reserved: 5;
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
	void (*write)(mr_serial_t serial, mr_uint8_t data);
	mr_uint8_t (*read)(mr_serial_t serial);
	void (*start_tx)(mr_serial_t serial);
	void (*stop_tx)(mr_serial_t serial);
};

struct mr_serial
{
	struct mr_device device;

	struct mr_serial_config config;
	mr_size_t fifo_bufsz;
	void *fifo_rx;
	void *fifo_tx;

	const struct mr_serial_ops *ops;
};

mr_err_t mr_hw_serial_add_to_container(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data);
void mr_hw_serial_isr(mr_serial_t serial, mr_uint16_t event);

#endif

#endif
