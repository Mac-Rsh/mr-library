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

#if (MR_CFG_SPI == MR_CFG_ENABLE)

/**
 * @def SPI device host/slave
 */
#define MR_SPI_HOST                     0
#define MR_SPI_SLAVE                    1

/**
 * @def SPI device mode
 */
#define MR_SPI_MODE_0                   0
#define MR_SPI_MODE_1                   1
#define MR_SPI_MODE_2                   2
#define MR_SPI_MODE_3                   3

/**
 * @def SPI device bit order
 */
#define MR_SPI_BIT_ORDER_MSB            0
#define MR_SPI_BIT_ORDER_LSB            1

/**
 * @def SPI device CS active
 */
#define MR_SPI_CS_ACTIVE_LOW            0
#define MR_SPI_CS_ACTIVE_HIGH           1
#define MR_SPI_CS_ACTIVE_NONE           2

/**
 * @def SPI device position bits
 */
#define MR_SPI_POS_BITS_8               0
#define MR_SPI_POS_BITS_16              1
#define MR_SPI_POS_BITS_32              2

/**
 * @def SPI device interrupt event
 */
#define MR_SPI_BUS_EVENT_RX_INT          0x10000000
#define MR_SPI_BUS_EVENT_MASK            0xf0000000

/**
 * @def SPI device default config
 */
#define MR_SPI_CONFIG_DEFAULT           \
{                                       \
    3000000,                            \
    MR_SPI_HOST,                        \
    MR_SPI_MODE_0,                      \
    MR_SPI_BIT_ORDER_MSB,               \
    MR_SPI_CS_ACTIVE_LOW,               \
    MR_SPI_POS_BITS_8,                  \
}

/**
 * @def SPI device config
 */
struct mr_spi_config
{
    mr_uint32_t baud_rate;
    mr_uint8_t host_slave: 1;
    mr_uint8_t mode: 2;
    mr_uint8_t bit_order: 1;
    mr_uint8_t cs_active: 2;
    mr_uint8_t pos_bits: 2;
};
typedef struct mr_spi_config *mr_spi_config_t;

typedef struct mr_spi_bus *mr_spi_bus_t;

/**
 * @struct SPI device
 */
struct mr_spi_device
{
    struct mr_device device;

    struct mr_spi_config config;
    mr_spi_bus_t bus;
    mr_pos_t cs_number;
};
typedef struct mr_spi_device *mr_spi_device_t;

/**
 * @struct SPI bus operations
 */
struct mr_spi_bus_ops
{
    mr_err_t (*configure)(mr_spi_bus_t spi_bus, struct mr_spi_config *config);
    void (*write)(mr_spi_bus_t spi_bus, mr_uint8_t data);
    mr_uint8_t (*read)(mr_spi_bus_t spi_bus);
    void (*cs_ctrl)(mr_spi_bus_t spi_bus, mr_pos_t cs_number, mr_state_t state);
    mr_uint8_t (*cs_read)(mr_spi_bus_t spi_bus, mr_pos_t cs_number);
};

/**
 * @struct SPI bus
 */
struct mr_spi_bus
{
    struct mr_device device;

    struct mr_spi_config config;
    mr_spi_device_t owner;
    struct mr_mutex lock;
    struct mr_rb rx_fifo;

    const struct mr_spi_bus_ops *ops;
};

/**
 * @addtogroup SPI device
 * @{
 */
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_pos_t cs_number);
/** @} */

/**
 * @addtogroup SPI bus
 * @{
 */
mr_err_t mr_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, struct mr_spi_bus_ops *ops, void *data);
void mr_spi_bus_isr(mr_spi_bus_t spi_bus, mr_uint32_t event);
/** @} */

#endif

#endif /* _SPI_H_ */