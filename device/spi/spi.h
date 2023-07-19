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

#define MR_SPI_BUS_EVENT_RX_INT          0x10000000
#define MR_SPI_BUS_EVENT_TX_INT          0x20000000
#define MR_SPI_BUS_EVENT_RX_DMA          0x40000000
#define MR_SPI_BUS_EVENT_TX_DMA          0x80000000
#define _MR_SPI_BUS_EVENT_MASK           0xf0000000

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

struct mr_spi_fifo
{
    mr_size_t bufsz;
    struct mr_fifo fifo;
};

typedef struct mr_spi_bus *mr_spi_bus_t;

struct mr_spi_device
{
    struct mr_device device;

    struct mr_spi_config config;
    struct mr_spi_bus *bus;
    mr_uint16_t cs_pin;

    struct mr_list tx_list;
    mr_size_t tx_count;
};
typedef struct mr_spi_device *mr_spi_device_t;

struct mr_spi_bus_ops
{
    mr_err_t (*configure)(mr_spi_bus_t spi_bus, struct mr_spi_config *config);
    void (*write)(mr_spi_bus_t spi_bus, mr_uint8_t data);
    mr_uint8_t (*read)(mr_spi_bus_t spi_bus);
    void (*cs_ctrl)(mr_spi_bus_t spi_bus, mr_pos_t cs_pin, mr_state_t state);

    mr_uint8_t (*cs_read)(mr_spi_bus_t spi_bus, mr_pos_t cs_pin);
    void (*start_tx)(mr_spi_bus_t spi_bus);
    void (*stop_tx)(mr_spi_bus_t spi_bus);

    void (*start_dma_tx)(mr_spi_bus_t spi_bus, mr_uint8_t *buffer, mr_size_t size);
    void (*stop_dma_tx)(mr_spi_bus_t spi_bus);
};

struct mr_spi_bus
{
    struct mr_device device;

    struct mr_spi_config config;
    struct mr_spi_device *owner;
    struct mr_mutex lock;
    struct mr_spi_fifo rx_fifo;
    struct mr_spi_fifo tx_fifo;
    struct mr_list tx_list;
    mr_uint8_t rx_dma[MR_CONF_SPI_RX_DMA_BUFS];
    mr_uint8_t tx_dma[MR_CONF_SPI_TX_DMA_BUFS];

    const struct mr_spi_bus_ops *ops;
};

mr_err_t mr_spi_bus_add(mr_spi_bus_t spi_bus, const char *name, void *data, struct mr_spi_bus_ops *ops);
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_uint16_t cs_pin);
void mr_spi_bus_isr(mr_spi_bus_t spi_bus, mr_uint32_t event);

#endif  /* MR_CONF_SPI */

#endif  /* _SPI_H_ */