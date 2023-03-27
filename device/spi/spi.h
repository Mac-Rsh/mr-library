/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-18     MacRsh       first version
 */

#ifndef _SPI_H_
#define _SPI_H_

#include <mrlib.h>

#if (MR_DEVICE_SPI == MR_CONF_ENABLE)

#define MR_SPI_HOST                          0
#define MR_SPI_SLAVE                         1

#define MR_SPI_MODE_0                        0
#define MR_SPI_MODE_1                        1
#define MR_SPI_MODE_2                        2
#define MR_SPI_MODE_3                        3

#define MR_SPI_DATA_BITS_8                   0
#define MR_SPI_DATA_BITS_16                  1
#define MR_SPI_DATA_BITS_32                  2

#define MR_SPI_BIT_ORDER_MSB                 0
#define MR_SPI_BIT_ORDER_LSB                 1

#define MR_SPI_CS_ACTIVE_LOW                 0
#define MR_SPI_CS_ACTIVE_HIGH                1

/* Default config for mr_spi_config structure */
#define MR_SPI_CONFIG_DEFAULT                \
{                                            \
    .baud_rate = 3000000,                    \
	.host_slave = MR_SPI_HOST,               \
	.mode = MR_SPI_MODE_0,                   \
	.data_bits = MR_SPI_DATA_BITS_8,         \
    .bit_order = MR_SPI_BIT_ORDER_MSB,       \
    .cs_active = MR_SPI_CS_ACTIVE_LOW,       \
}

struct mr_spi_config
{
	mr_uint32_t baud_rate;

	mr_uint8_t host_slave: 1;
	mr_uint8_t mode: 2;
	mr_uint8_t data_bits: 2;
	mr_uint8_t bit_order: 1;
	mr_uint8_t cs_active: 1;
};

typedef struct mr_spi_bus *mr_spi_bus_t;
struct mr_spi_device
{
	struct mr_device device;

	struct mr_spi_config config;
	struct mr_spi_bus *bus;
};
typedef struct mr_spi_device *mr_spi_device_t;

struct mr_spi_bus_ops
{
	mr_err_t (*configure)(mr_spi_bus_t spi_bus, struct mr_spi_config *config);
	void (*cs_set)(mr_spi_bus_t spi_bus, void *cs_data, mr_state_t state);
	mr_uint32_t (*transmit)(mr_spi_bus_t spi_bus, mr_uint32_t send_data);
};

struct mr_spi_bus
{
	struct mr_device device;

	struct mr_spi_config config;
	struct mr_spi_device *owner;
	struct mr_mutex lock;

	const struct mr_spi_bus_ops *ops;
};

mr_err_t mr_hw_spi_bus_add_to_container(mr_spi_bus_t spi_bus, const char *name, struct mr_spi_bus_ops *ops, void *data);
mr_err_t mr_hw_spi_device_add_to_container(mr_spi_device_t spi_device,
										   const char *name,
										   mr_uint16_t support_flag,
										   void *cs_data);

#endif

#endif
