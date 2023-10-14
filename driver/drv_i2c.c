/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#include "drv_i2c.h"

#if (MR_CFG_I2C == MR_CFG_ENABLE)

static struct drv_soft_i2c_bus_data drv_soft_i2c_bus_data[] =
    {
#ifdef MR_BSP_I2C_1
        {"i2c1", /* ... */},
#endif
#ifdef MR_BSP_I2C_2
        {"i2c2", /* ... */},
#endif
#ifdef MR_BSP_I2C_3
        {"i2c3", /* ... */},
#endif
        /* ... */
    };

static struct mr_soft_i2c_bus soft_i2c_bus_device[mr_array_num(drv_soft_i2c_bus_data)];

static mr_err_t drv_soft_i2c_bus_configure(mr_soft_i2c_bus_t i2c_bus, mr_state_t state)
{
    struct drv_soft_i2c_bus_data *soft_i2c_bus_data = (struct drv_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    /* ... */

    return MR_ERR_OK;
}

static void drv_soft_i2c_bus_scl_write(mr_soft_i2c_bus_t i2c_bus, mr_level_t level)
{
    struct drv_soft_i2c_bus_data *soft_i2c_bus_data = (struct drv_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    /* ... */
}

static void drv_soft_i2c_bus_sda_write(mr_soft_i2c_bus_t i2c_bus, mr_level_t level)
{
    struct drv_soft_i2c_bus_data *soft_i2c_bus_data = (struct drv_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    /* ... */
}

static mr_level_t drv_soft_i2c_bus_sda_read(mr_soft_i2c_bus_t i2c_bus)
{
    struct drv_soft_i2c_bus_data *soft_i2c_bus_data = (struct drv_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;
    mr_level_t level = MR_LOW;

    /* ... */

    return level;
}

mr_err_t drv_soft_i2c_bus_init(void)
{
    static struct mr_soft_i2c_ops drv_ops =
        {
            drv_soft_i2c_bus_configure,
            drv_soft_i2c_bus_scl_write,
            drv_soft_i2c_bus_sda_write,
            drv_soft_i2c_bus_sda_read,
        };
    mr_size_t count = mr_array_num(soft_i2c_bus_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_soft_i2c_bus_add(&soft_i2c_bus_device[count],
                                  drv_soft_i2c_bus_data[count].name,
                                  &drv_ops,
                                  &drv_soft_i2c_bus_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_soft_i2c_bus_init);

#endif
