/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-13    MacRsh       First version
 */

#include "drv_i2c.h"

#ifdef MR_USING_I2C

enum drv_i2c_index
{
#ifdef MR_USING_I2C1
    DRV_INDEX_I2C1,
#endif /* MR_USING_I2C1 */
#ifdef MR_USING_I2C2
    DRV_INDEX_I2C2,
#endif /* MR_USING_I2C2 */
};

static const char *i2c_name[] =
    {
#ifdef MR_USING_I2C1
        "i2c1",
#endif /* MR_USING_I2C1 */
#ifdef MR_USING_I2C2
        "i2c2",
#endif /* MR_USING_I2C2 */
    };

static struct drv_i2c_bus_data i2c_bus_drv_data[] =
    {
#ifdef MR_USING_I2C1
#if (MR_CFG_I2C1_GROUP == 1)
        {
            I2C1,
            RCC_APB1Periph_I2C1,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_6,
            GPIOB,
            GPIO_Pin_7,
            I2C1_IRQn,
            0
        },
#elif (MR_CFG_I2C1_GROUP == 2)
        {
            I2C1,
            RCC_APB1Periph_I2C1,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_8,
            GPIOB,
            GPIO_Pin_9,
            I2C1_IRQn,
            GPIO_Remap_I2C1
        },
#else
#error "MR_CFG_I2C1_GROUP is not defined or defined incorrectly (support values: 1, 2)."
#endif /* MR_CFG_I2C1_GROUP */
#endif /* MR_USING_I2C1 */
#ifdef MR_USING_I2C2
#if (MR_CFG_I2C2_GROUP == 1)
        {
            I2C2,
            RCC_APB1Periph_I2C2,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_10,
            GPIOB,
            GPIO_Pin_11,
            I2C2_IRQn,
            0
        }
#else
#error "MR_CFG_I2C2_GROUP is not defined or defined incorrectly (support values: 1)."
#endif /* MR_CFG_I2C2_GROUP */
#endif /* MR_USING_I2C2 */
    };

static struct mr_i2c_bus i2c_bus_dev[mr_array_num(i2c_bus_drv_data)];

static int drv_i2c_bus_configure(struct mr_i2c_bus *i2c_bus, struct mr_i2c_config *config, int addr, int addr_bits)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int state = (config->baud_rate == 0) ? DISABLE : ENABLE;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitStructure = {0};

    RCC_APB2PeriphClockCmd(i2c_bus_data->gpio_clock, ENABLE);
    RCC_APB2PeriphClockCmd(i2c_bus_data->clock, state);

    /* Configure remap */
    if (i2c_bus_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(i2c_bus_data->remap, state);
    }

    if (state == ENABLE)
    {
        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->scl_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(i2c_bus_data->scl_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->sda_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(i2c_bus_data->sda_port, &GPIO_InitStructure);

        switch (addr_bits)
        {
            case MR_I2C_ADDR_BITS_7:
            {
                I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
                break;
            }

            case MR_I2C_ADDR_BITS_10:
            {
                I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }
    } else
    {
        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->scl_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(i2c_bus_data->scl_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->sda_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(i2c_bus_data->sda_port, &GPIO_InitStructure);
    }
    if (config->host_slave == MR_I2C_HOST)
    {
        I2C_AcknowledgeConfig(i2c_bus_data->instance, state);
    } else
    {
        I2C_AcknowledgeConfig(i2c_bus_data->instance, DISABLE);
    }

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = i2c_bus_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure I2C */
    I2C_InitStructure.I2C_ClockSpeed = config->baud_rate;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 = addr;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(i2c_bus_data->instance, &I2C_InitStructure);
    I2C_Cmd(i2c_bus_data->instance, state);
    return MR_EOK;
}

#endif /* MR_USING_I2C */
