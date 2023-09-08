/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-16     MacRsh       first version
 */

#include "drv_spi.h"

#if (MR_CFG_SPI == MR_CFG_ENABLE)

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))

enum ch32_spi_index
{
#ifdef MR_BSP_SPI_1
    CH32_SPI_1_INDEX,
#endif
#ifdef MR_BSP_SPI_2
    CH32_SPI_2_INDEX,
#endif
#ifdef MR_BSP_SPI_3
    CH32_SPI_3_INDEX
#endif
};

static struct ch32_spi_bus_data ch32_spi_bus_data[] =
    {
#ifdef MR_BSP_SPI_1
        {"spi1", SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, SPI1_IRQn},
#endif
#ifdef MR_BSP_SPI_2
        {"spi2", SPI2, RCC_APB1Periph_SPI2, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15,
         SPI2_IRQn},
#endif
#ifdef MR_BSP_SPI_3
        {"spi3", SPI3, RCC_APB1Periph_SPI3, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, SPI3_IRQn},
#endif
    };

static struct mr_spi_bus spi_bus_device[mr_array_number_of(ch32_spi_bus_data)];

static mr_uint16_t ch32_spi_baud_rate_prescaler(mr_uint32_t pclk_freq, mr_uint32_t baud_rate)
{
    mr_uint16_t psc = pclk_freq / baud_rate;

    if (psc >= 256)
    {
        return SPI_BaudRatePrescaler_256;
    } else if (psc >= 128)
    {
        return SPI_BaudRatePrescaler_128;
    } else if (psc >= 64)
    {
        return SPI_BaudRatePrescaler_64;
    } else if (psc >= 32)
    {
        return SPI_BaudRatePrescaler_32;
    } else if (psc >= 16)
    {
        return SPI_BaudRatePrescaler_16;
    } else if (psc >= 8)
    {
        return SPI_BaudRatePrescaler_8;
    } else if (psc >= 4)
    {
        return SPI_BaudRatePrescaler_4;
    } else if (psc >= 2)
    {
        return SPI_BaudRatePrescaler_2;
    }
    return SPI_BaudRatePrescaler_2;
}

static mr_err_t ch32_spi_configure(mr_spi_bus_t spi_bus, mr_spi_config_t config)
{
    struct ch32_spi_bus_data *spi_bus_data = (struct ch32_spi_bus_data *)spi_bus->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStruct = {0};
    mr_uint32_t pclk_freq = 0;

    RCC_GetClocksFreq(&RCC_ClockStruct);

    if ((uint32_t)spi_bus_data->instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(spi_bus_data->spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK2_Frequency;
    } else
    {
        RCC_APB1PeriphClockCmd(spi_bus_data->spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK1_Frequency;
    }
    RCC_APB2PeriphClockCmd(spi_bus_data->gpio_periph_clock | RCC_APB2Periph_AFIO, ENABLE);

    switch (config->host_slave)
    {
        case MR_SPI_HOST:
        {
            SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->clk_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->miso_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->mosi_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);
            break;
        }

        case MR_SPI_SLAVE:
        {
            SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->clk_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->miso_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = spi_bus_data->mosi_gpio_pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(spi_bus_data->gpio_port, &GPIO_InitStructure);
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    switch (config->mode)
    {
        case MR_SPI_MODE_0:
        {
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        }

        case MR_SPI_MODE_1:
        {
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
        }

        case MR_SPI_MODE_2:
        {
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        }

        case MR_SPI_MODE_3:
        {
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    switch (config->data_bits)
    {
        case MR_SPI_DATA_BITS_8:
        {
            SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
            break;
        }

        case MR_SPI_DATA_BITS_16:
        {
            SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    switch (config->bit_order)
    {
        case MR_SPI_BIT_ORDER_LSB:
        {
            SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
            break;
        }

        case MR_SPI_BIT_ORDER_MSB:
        {
            SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    switch (config->cs_active)
    {
        case MR_SPI_CS_ACTIVE_LOW:
        case MR_SPI_CS_ACTIVE_HIGH:
        {
            SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
            break;
        }

        case MR_SPI_CS_ACTIVE_HARDWARE:
        {
            SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    NVIC_InitStructure.NVIC_IRQChannel = spi_bus_data->irqno;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    if (config->host_slave == MR_SPI_HOST)
    {
        SPI_I2S_ITConfig(spi_bus_data->instance, SPI_I2S_IT_RXNE, DISABLE);
    } else
    {
        SPI_I2S_ITConfig(spi_bus_data->instance, SPI_I2S_IT_RXNE, ENABLE);
    }

    SPI_InitStructure.SPI_BaudRatePrescaler = ch32_spi_baud_rate_prescaler(pclk_freq, config->baud_rate);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(spi_bus_data->instance, &SPI_InitStructure);
    SPI_Cmd(spi_bus_data->instance, ENABLE);

    return MR_ERR_OK;
}

static void ch32_spi_write(mr_spi_bus_t spi_bus, mr_uint32_t data)
{
    struct ch32_spi_bus_data *spi_bus_data = (struct ch32_spi_bus_data *)spi_bus->device.data;
    mr_size_t i = 0;

    while (SPI_I2S_GetFlagStatus(spi_bus_data->instance, SPI_I2S_FLAG_TXE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return;
        }
    }
    SPI_I2S_SendData(spi_bus_data->instance, data);
}

static mr_uint32_t ch32_spi_read(mr_spi_bus_t spi_bus)
{
    struct ch32_spi_bus_data *spi_bus_data = (struct ch32_spi_bus_data *)spi_bus->device.data;
    mr_size_t i = 0;

    while (SPI_I2S_GetFlagStatus(spi_bus_data->instance, SPI_I2S_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }

    return SPI_I2S_ReceiveData(spi_bus_data->instance);
}

static void ch32_spi_cs_write(mr_spi_bus_t spi_bus, mr_off_t cs_number, mr_level_t level)
{
    if (cs_number < 0 || cs_number > MR_BSP_PIN_NUMBER)
    {
        return;
    }

    GPIO_WriteBit(PIN_STPORT(cs_number), PIN_STPIN(cs_number), (BitAction)level);
}

static mr_level_t ch32_spi_cs_read(mr_spi_bus_t spi_bus, mr_off_t cs_number)
{
    if (cs_number < 0 || cs_number > MR_BSP_PIN_NUMBER)
    {
        return 0;
    }

    return (mr_level_t)GPIO_ReadOutputDataBit(PIN_STPORT(cs_number), PIN_STPIN(cs_number));
}

static void ch32_spi_isr(mr_spi_bus_t spi_bus)
{
    struct ch32_spi_bus_data *spi_bus_data = (struct ch32_spi_bus_data *)spi_bus->device.data;

    if (SPI_I2S_GetITStatus(spi_bus_data->instance, SPI_I2S_IT_RXNE) != RESET)
    {
        mr_spi_bus_isr(spi_bus, MR_SPI_BUS_EVENT_RX_INT);
        SPI_I2S_ClearITPendingBit(spi_bus_data->instance, SPI_I2S_IT_RXNE);
    }
}

#ifdef MR_BSP_SPI_1
void SPI1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI1_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[CH32_SPI_1_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_2
void SPI2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI2_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[CH32_SPI_2_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_3
void SPI3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI3_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[CH32_SPI_3_INDEX]);
}
#endif

mr_err_t drv_spi_bus_init(void)
{
    static struct mr_spi_bus_ops drv_ops =
        {
            ch32_spi_configure,
            ch32_spi_write,
            ch32_spi_read,
            ch32_spi_cs_write,
            ch32_spi_cs_read,
        };
    mr_size_t count = mr_array_number_of(spi_bus_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret =
            mr_spi_bus_add(&spi_bus_device[count], ch32_spi_bus_data[count].name, &drv_ops, &ch32_spi_bus_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
MR_INIT_DRIVER_EXPORT(drv_spi_bus_init);

#endif
