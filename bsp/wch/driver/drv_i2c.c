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

static const char *i2c_bus_name[] =
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
        DRV_I2C1_CONFIG,
#endif /* MR_USING_I2C1 */
#ifdef MR_USING_I2C2
        DRV_I2C2_CONFIG,
#endif /* MR_USING_I2C2 */
    };

static struct mr_i2c_bus i2c_bus_dev[mr_array_num(i2c_bus_drv_data)];

static int drv_i2c_bus_configure(struct mr_i2c_bus *i2c_bus, struct mr_i2c_config *config, int addr, int addr_bits)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int state = (config->baud_rate == 0) ? DISABLE : ENABLE;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    I2C_InitTypeDef I2C_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(i2c_bus_data->gpio_clock, ENABLE);
    RCC_APB1PeriphClockCmd(i2c_bus_data->clock, state);

    /* Configure remap */
    if (i2c_bus_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(i2c_bus_data->remap, state);
    }

    if (state == ENABLE)
    {
        switch (addr_bits)
        {
            case MR_I2C_ADDR_BITS_7:
            {
                I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
                break;
            }
            case MR_I2C_ADDR_BITS_10:
            {
                I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->scl_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(i2c_bus_data->scl_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->sda_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(i2c_bus_data->sda_port, &GPIO_InitStructure);
    } else
    {
        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->scl_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(i2c_bus_data->scl_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = i2c_bus_data->sda_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(i2c_bus_data->sda_port, &GPIO_InitStructure);
    }

    /* Configure I2C */
    I2C_InitStructure.I2C_ClockSpeed = config->baud_rate;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 = addr;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(i2c_bus_data->instance, &I2C_InitStructure);
    I2C_Cmd(i2c_bus_data->instance, state);
    I2C_AcknowledgeConfig(i2c_bus_data->instance, state);

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = i2c_bus_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    if (config->host_slave == MR_I2C_HOST)
    {
        I2C_ITConfig(i2c_bus_data->instance, I2C_IT_EVT, DISABLE);
    } else
    {
        I2C_ITConfig(i2c_bus_data->instance, I2C_IT_EVT, state);
    }
    I2C_ClearITPendingBit(i2c_bus_data->instance, I2C_IT_RXNE);
    return MR_EOK;
}

static void drv_i2c_bus_start(struct mr_i2c_bus *i2c_bus)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int i = 0;

    I2C_GenerateSTART(i2c_bus_data->instance, ENABLE);
    while (I2C_CheckEvent(i2c_bus_data->instance, I2C_EVENT_MASTER_MODE_SELECT) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return;
        }
    }
}

static void drv_i2c_bus_send_addr(struct mr_i2c_bus *i2c_bus, int addr, int addr_bits)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int i = 0;

    I2C_SendData(i2c_bus_data->instance, addr);
    while (I2C_CheckEvent(i2c_bus_data->instance, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return;
        }
    }

    if (addr_bits == MR_I2C_ADDR_BITS_10)
    {
        I2C_SendData(i2c_bus_data->instance, (addr >> 8));
        i = 0;
        while (I2C_CheckEvent(i2c_bus_data->instance, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == RESET)
        {
            i++;
            if (i > INT16_MAX)
            {
                return;
            }
        }
    }
}

static void drv_i2c_bus_stop(struct mr_i2c_bus *i2c_bus)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;

    I2C_GenerateSTOP(i2c_bus_data->instance, ENABLE);
}

static uint8_t drv_i2c_bus_read(struct mr_i2c_bus *i2c_bus, int ack_state)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int i = 0;

    /* Control ack */
    I2C_AcknowledgeConfig(i2c_bus_data->instance, ack_state);

    /* Read data */
    while (I2C_CheckEvent(i2c_bus_data->instance, I2C_EVENT_MASTER_BYTE_RECEIVED) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return 0;
        }
    }
    return (uint8_t)I2C_ReceiveData(i2c_bus_data->instance);
}

static void drv_i2c_bus_write(struct mr_i2c_bus *i2c_bus, uint8_t data)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;
    int i = 0;

    /* Write data */
    I2C_SendData(i2c_bus_data->instance, data);
    while (I2C_CheckEvent(i2c_bus_data->instance, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return;
        }
    }
}

static void drv_i2c_bus_isr(struct mr_i2c_bus *i2c_bus)
{
    struct drv_i2c_bus_data *i2c_bus_data = (struct drv_i2c_bus_data *)i2c_bus->dev.drv->data;

    if (I2C_GetITStatus(i2c_bus_data->instance, I2C_IT_RXNE) != RESET)
    {
        mr_dev_isr(&i2c_bus->dev, MR_ISR_I2C_RD_INT, NULL);
        I2C_ClearITPendingBit(i2c_bus_data->instance, I2C_IT_RXNE);
    }
}

#ifdef MR_USING_I2C1
void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_EV_IRQHandler(void)
{
    drv_i2c_bus_isr(&i2c_bus_dev[DRV_INDEX_I2C1]);
}
#endif /* MR_USING_I2C1 */

#ifdef MR_USING_I2C2
void I2C2_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C2_EV_IRQHandler(void)
{
    drv_i2c_bus_isr(&i2c_bus_dev[DRV_INDEX_I2C2]);
}
#endif /* MR_USING_I2C2 */

static struct mr_i2c_bus_ops i2c_bus_drv_ops =
    {
        drv_i2c_bus_configure,
        drv_i2c_bus_start,
        drv_i2c_bus_send_addr,
        drv_i2c_bus_stop,
        drv_i2c_bus_read,
        drv_i2c_bus_write,
    };

static struct mr_drv i2c_bus_drv[] =
    {
#ifdef MR_USING_I2C1
        {
            Mr_Drv_Type_I2C,
            &i2c_bus_drv_ops,
            &i2c_bus_drv_data[DRV_INDEX_I2C1]
        },
#endif /* MR_USING_I2C1 */
#ifdef MR_USING_I2C2
        {
            Mr_Drv_Type_I2C,
            &i2c_bus_drv_ops,
            &i2c_bus_drv_data[DRV_INDEX_I2C2]
        },
#endif /* MR_USING_I2C2 */
    };

int drv_i2c_bus_init(void)
{
    int index = 0;

    for (index = 0; index < mr_array_num(i2c_bus_dev); index++)
    {
        mr_i2c_bus_register(&i2c_bus_dev[index], i2c_bus_name[index], &i2c_bus_drv[index]);
    }
    return MR_EOK;
}
MR_DRV_EXPORT(drv_i2c_bus_init);

#endif /* MR_USING_I2C */
