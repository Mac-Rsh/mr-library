/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-19     MacRsh       first version
 */

#include "drv_i2c.h"

#if (MR_CONF_I2C == MR_CONF_ENABLE)

enum
{
#ifdef BSP_I2C_1
    I2C1_INDEX,
#endif
#ifdef BSP_I2C_2
    I2C2_INDEX,
#endif
#ifdef BSP_I2C_3
    I2C3_INDEX,
#endif
};

static struct ch32_soft_i2c ch32_soft_i2c[] =
        {
#ifdef BSP_I2C_1
                {"i2c1", {RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIO_Pin_1}},
#endif
#ifdef BSP_I2C_2
                {"i2c2", {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_0, GPIO_Pin_1}},
#endif
#ifdef BSP_I2C_3
                {"i2c3",{RCC_APB2Periph_GPIOC, GPIOC,GPIO_Pin_0,GPIO_Pin_1}},
#endif
        };

static struct mr_soft_i2c_bus soft_i2c_bus_device[mr_array_get_length(ch32_soft_i2c)];

mr_err_t ch32_soft_i2c_configure(mr_soft_i2c_bus_t i2c_bus)
{
    struct ch32_soft_i2c *driver = (struct ch32_soft_i2c *)i2c_bus->i2c_bus.device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(driver->info.gpio_periph_clock, ENABLE);
    GPIO_InitStructure.GPIO_Pin = driver->info.scl_gpio_pin | driver->info.sda_gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);

    return MR_ERR_OK;
}

void ch32_soft_i2c_scl_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{
    struct ch32_soft_i2c *driver = (struct ch32_soft_i2c *)i2c_bus->i2c_bus.device.data;

    GPIO_WriteBit(driver->info.gpio_port, driver->info.scl_gpio_pin, value);
}

void ch32_soft_i2c_sda_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{
    struct ch32_soft_i2c *driver = (struct ch32_soft_i2c *)i2c_bus->i2c_bus.device.data;

    GPIO_WriteBit(driver->info.gpio_port, driver->info.sda_gpio_pin, value);
}

mr_uint8_t ch32_soft_i2c_sda_read(mr_soft_i2c_bus_t i2c_bus)
{
    struct ch32_soft_i2c *driver = (struct ch32_soft_i2c *)i2c_bus->i2c_bus.device.data;

    return GPIO_ReadOutputDataBit(driver->info.gpio_port, driver->info.sda_gpio_pin);
}

mr_err_t ch32_soft_i2c_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(soft_i2c_bus_device);
    static struct mr_soft_i2c_ops driver =
            {
                    ch32_soft_i2c_configure,
                    ch32_soft_i2c_scl_write,
                    ch32_soft_i2c_sda_write,
                    ch32_soft_i2c_sda_read,
            };

    while (count--)
    {
        ret = mr_soft_i2c_bus_add(&soft_i2c_bus_device[count],
                                  ch32_soft_i2c[count].name,
                                  &ch32_soft_i2c[count],
                                  &driver);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_soft_i2c_init);

#endif  /* MR_CONF_I2C */