/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#include "drv_spi.h"

#if (MR_CFG_SPI == MR_CFG_ENABLE)

static struct drv_spi_bus_data drv_spi_bus_data[] =
    {
#ifdef MR_BSP_SPI_1
        {"spi1", /* ... */},
#endif
#ifdef MR_BSP_SPI_2
        {"spi2", /* ... */},
#endif
#ifdef MR_BSP_SPI_3
        {"spi3", /* ... */},
#endif
        /* ... */
    };

static struct mr_spi_bus spi_bus_device[mr_array_num(drv_spi_bus_data)];

static mr_err_t drv_spi_configure(mr_spi_bus_t spi_bus, mr_spi_config_t config)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;

    /* ... */

    return MR_ERR_OK;
}

static void drv_spi_write(mr_spi_bus_t spi_bus, mr_uint32_t data)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;

    /* ... */
}

static mr_uint32_t drv_spi_read(mr_spi_bus_t spi_bus)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;
    mr_uint32_t data = 0;

    /* ... */

    return data;
}

static void drv_spi_cs_write(mr_spi_bus_t spi_bus, mr_off_t cs_number, mr_level_t level)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;

    /* ... */
}

static mr_level_t drv_spi_cs_read(mr_spi_bus_t spi_bus, mr_off_t cs_number)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;
    mr_level_t level = MR_LOW;

    /* ... */

    return level;
}

mr_err_t drv_spi_bus_init(void)
{
    static struct mr_spi_bus_ops drv_ops =
        {
            drv_spi_configure,
            drv_spi_write,
            drv_spi_read,
            drv_spi_cs_write,
            drv_spi_cs_read,
        };
    mr_size_t count = mr_array_num(spi_bus_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret =
            mr_spi_bus_add(&spi_bus_device[count], drv_spi_bus_data[count].name, &drv_ops, &drv_spi_bus_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_spi_bus_init);

#endif
