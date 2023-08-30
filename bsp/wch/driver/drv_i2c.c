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

#if (MR_CFG_I2C == MR_CFG_ENABLE)

static struct ch32_soft_i2c_bus_data ch32_soft_i2c_bus_data[] =
    {
#ifdef MR_BSP_I2C_1
        {"i2c1", RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIO_Pin_1},
#endif
#ifdef MR_BSP_I2C_2
        {"i2c2", RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_0, GPIO_Pin_1},
#endif
#ifdef MR_BSP_I2C_3
        {"i2c3", RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_0, GPIO_Pin_1},
#endif
    };

static struct mr_soft_i2c_bus soft_i2c_bus_device[MR_ARRAY_SIZE(ch32_soft_i2c_bus_data)];

static mr_err_t ch32_soft_i2c_bus_configure(mr_soft_i2c_bus_t i2c_bus)
{
    struct ch32_soft_i2c_bus_data *soft_i2c_bus_data = (struct ch32_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(soft_i2c_bus_data->gpio_periph_clock, ENABLE);
    GPIO_InitStructure.GPIO_Pin = soft_i2c_bus_data->scl_gpio_pin | soft_i2c_bus_data->sda_gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(soft_i2c_bus_data->gpio_port, &GPIO_InitStructure);

    return MR_ERR_OK;
}

static void ch32_soft_i2c_bus_scl_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{
    struct ch32_soft_i2c_bus_data *soft_i2c_bus_data = (struct ch32_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    GPIO_WriteBit(soft_i2c_bus_data->gpio_port, soft_i2c_bus_data->scl_gpio_pin, value);
}

static void ch32_soft_i2c_bus_sda_write(mr_soft_i2c_bus_t i2c_bus, mr_uint8_t value)
{
    struct ch32_soft_i2c_bus_data *soft_i2c_bus_data = (struct ch32_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    GPIO_WriteBit(soft_i2c_bus_data->gpio_port, soft_i2c_bus_data->sda_gpio_pin, value);
}

static mr_uint8_t ch32_soft_i2c_bus_sda_read(mr_soft_i2c_bus_t i2c_bus)
{
    struct ch32_soft_i2c_bus_data *soft_i2c_bus_data = (struct ch32_soft_i2c_bus_data *)i2c_bus->i2c_bus.device.data;

    return GPIO_ReadOutputDataBit(soft_i2c_bus_data->gpio_port, soft_i2c_bus_data->sda_gpio_pin);
}

mr_err_t drv_soft_i2c_bus_init(void)
{
    static struct mr_soft_i2c_ops drv_ops =
        {
            ch32_soft_i2c_bus_configure,
            ch32_soft_i2c_bus_scl_write,
            ch32_soft_i2c_bus_sda_write,
            ch32_soft_i2c_bus_sda_read,
        };
    mr_size_t count = MR_ARRAY_SIZE(soft_i2c_bus_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_soft_i2c_bus_add(&soft_i2c_bus_device[count],
                                  ch32_soft_i2c_bus_data[count].name,
                                  &drv_ops,
                                  &ch32_soft_i2c_bus_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
MR_INIT_DRIVER_EXPORT(drv_soft_i2c_bus_init);

#endif