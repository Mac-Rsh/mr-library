/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _SPI_H_
#define _SPI_H_

#include "mrlib.h"

#if (MR_CONF_SPI == MR_CONF_ENABLE)

#define MR_SPI_HOST                     0
#define MR_SPI_SLAVE                    1

#define MR_SPI_MODE_0                   0
#define MR_SPI_MODE_1                   1
#define MR_SPI_MODE_2                   2
#define MR_SPI_MODE_3                   3

#define MR_SPI_BIT_ORDER_MSB            0
#define MR_SPI_BIT_ORDER_LSB            1

#define MR_SPI_CS_ACTIVE_LOW            0
#define MR_SPI_CS_ACTIVE_HIGH           1
#define MR_SPI_CS_ACTIVE_NONE           2

/* Default config for mr_spi_config structure */
#define MR_SPI_CONFIG_DEFAULT           \
{                                       \
    3000000,                            \
    MR_SPI_HOST,                        \
    MR_SPI_MODE_0,                      \
    MR_SPI_BIT_ORDER_MSB,               \
    MR_SPI_CS_ACTIVE_LOW,               \
}

struct mr_spi_config
{
    mr_uint32_t baud_rate;

    mr_uint8_t host_slave;
    mr_uint8_t mode;
    mr_uint8_t bit_order;
    mr_uint8_t cs_active;
};

typedef struct mr_spi_bus *mr_spi_bus_t;

struct mr_spi_device
{
    struct mr_device device;

    struct mr_spi_config config;
    struct mr_spi_bus *bus;
    mr_uint16_t cs_pin;
};
typedef struct mr_spi_device *mr_spi_device_t;

struct mr_spi_bus_ops
{
    mr_err_t (*configure)(mr_spi_bus_t spi_bus, struct mr_spi_config *config);
    mr_uint8_t (*transfer)(mr_spi_bus_t spi_bus, mr_uint8_t data);
    void (*cs_ctrl)(mr_spi_bus_t spi_bus, mr_uint16_t cs_pin, mr_uint8_t state);
};

struct mr_spi_bus
{
    struct mr_device device;

    struct mr_spi_config config;
    struct mr_spi_device *owner;
    struct mr_mutex lock;

    const struct mr_spi_bus_ops *ops;
};

mr_err_t mr_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, struct mr_spi_bus_ops *ops, void *data);
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device,
                           const char *name,
                           mr_uint16_t support_flag,
                           mr_uint16_t cs_pin,
                           const char *bus_name);

#endif

#endif