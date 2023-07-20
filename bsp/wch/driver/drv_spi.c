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

#if (MR_CONF_SPI == MR_CONF_ENABLE)

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))

enum
{
#ifdef BSP_SPI_1
    SPI1_INDEX,
#endif
#ifdef BSP_SPI_2
    SPI2_INDEX,
#endif
#ifdef BSP_SPI_3
    SPI3_INDEX,
#endif
};

static struct ch32_spi ch32_spi[] =
        {
#ifdef BSP_SPI_1
                {"spi1",
                 {SPI1,
                  RCC_APB2Periph_SPI1,
                  RCC_APB2Periph_GPIOA,
                  GPIOA,
                  GPIO_Pin_5,
                  GPIO_Pin_6,
                  GPIO_Pin_7,
                  SPI1_IRQn}},
#endif
#ifdef BSP_SPI_2
                {"spi2",
                 {SPI2,
                  RCC_APB1Periph_SPI2,
                  RCC_APB2Periph_GPIOB,
                  GPIOB,
                  GPIO_Pin_13,
                  GPIO_Pin_14,
                  GPIO_Pin_15,
                  SPI2_IRQn}},
#endif
#ifdef BSP_SPI_3
                {"spi3",
                 {SPI3,
                  RCC_APB1Periph_SPI3,
                  RCC_APB2Periph_GPIOB,
                  GPIOB,
                  GPIO_Pin_3,
                  GPIO_Pin_4,
                  GPIO_Pin_5,
                  SPI3_IRQn}},
#endif
        };

static struct mr_spi_bus spi_bus_device[mr_array_get_length(ch32_spi)];

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

static mr_err_t ch32_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStruct = {0};
    mr_uint32_t pclk_freq = 0;

    RCC_GetClocksFreq(&RCC_ClockStruct);

    if ((uint32_t)driver->info.Instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(driver->info.spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK2_Frequency;
    } else
    {
        RCC_APB1PeriphClockCmd(driver->info.spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK1_Frequency;
    }
    RCC_APB2PeriphClockCmd(driver->info.gpio_periph_clock | RCC_APB2Periph_AFIO, ENABLE);

    if (config->cs_active == MR_SPI_CS_ACTIVE_NONE)
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
    } else
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    }

    switch (config->mode)
    {
        case MR_SPI_MODE_0:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        case MR_SPI_MODE_1:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
        case MR_SPI_MODE_2:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        case MR_SPI_MODE_3:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
        default:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    }

    switch (config->bit_order)
    {
        case MR_SPI_BIT_ORDER_LSB:
            SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
            break;
        case MR_SPI_BIT_ORDER_MSB:
            SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
            break;
        default:
            SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    }

    if (config->host_slave == MR_SPI_HOST)
    {
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

        GPIO_InitStructure.GPIO_Pin = driver->info.clk_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = driver->info.miso_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = driver->info.mosi_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);
    } else
    {
        SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

        GPIO_InitStructure.GPIO_Pin = driver->info.clk_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = driver->info.miso_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = driver->info.mosi_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(driver->info.gpio_port, &GPIO_InitStructure);
    }

    NVIC_InitStructure.NVIC_IRQChannel = driver->info.irqno;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    if (config->host_slave == MR_SPI_HOST)
    {
        SPI_I2S_ITConfig(driver->info.Instance, SPI_I2S_IT_RXNE, DISABLE);
    } else
    {
        SPI_I2S_ITConfig(driver->info.Instance, SPI_I2S_IT_RXNE, ENABLE);
    }

    SPI_InitStructure.SPI_BaudRatePrescaler = ch32_spi_baud_rate_prescaler(pclk_freq, config->baud_rate);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_Init(driver->info.Instance, &SPI_InitStructure);
    SPI_Cmd(driver->info.Instance, ENABLE);

    return MR_ERR_OK;
}

static void ch32_spi_write(mr_spi_bus_t spi_bus, mr_uint8_t data)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;
    mr_size_t i = 0;

    while (SPI_I2S_GetFlagStatus(driver->info.Instance, SPI_I2S_FLAG_TXE) == RESET)
    {
        i++;
        if (i > 200)
        {
            return;
        }
    }
    SPI_I2S_SendData(driver->info.Instance, data);
}

static mr_uint8_t ch32_spi_read(mr_spi_bus_t spi_bus)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;
    mr_size_t i = 0;

    while (SPI_I2S_GetFlagStatus(driver->info.Instance, SPI_I2S_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > 200)
        {
            return 0;
        }
    }

    return SPI_I2S_ReceiveData(driver->info.Instance);
}

static void ch32_spi_cs_crtl(mr_spi_bus_t spi_bus, mr_pos_t cs_pin, mr_state_t state)
{
    if (state == MR_ENABLE)
    {
        GPIO_WriteBit(PIN_STPORT(cs_pin), PIN_STPIN(cs_pin), spi_bus->config.cs_active);
    } else
    {
        GPIO_WriteBit(PIN_STPORT(cs_pin), PIN_STPIN(cs_pin), !spi_bus->config.cs_active);
    }
}

static mr_uint8_t ch32_spi_cs_read(mr_spi_bus_t spi_bus, mr_pos_t cs_pin)
{
    return GPIO_ReadOutputDataBit(PIN_STPORT(cs_pin), PIN_STPIN(cs_pin));
}

static void ch32_spi_start_tx(mr_spi_bus_t spi_bus)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;

    SPI_I2S_ITConfig(driver->info.Instance, SPI_I2S_IT_TXE, ENABLE);
}

static void ch32_spi_stop_tx(mr_spi_bus_t spi_bus)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;

    SPI_I2S_ITConfig(driver->info.Instance, SPI_I2S_IT_TXE, DISABLE);
}

static void ch32_spi_isr(mr_spi_bus_t spi_bus)
{
    struct ch32_spi *driver = (struct ch32_spi *)spi_bus->device.data;

    if (SPI_I2S_GetITStatus(driver->info.Instance, SPI_I2S_IT_RXNE) != RESET)
    {
        mr_spi_bus_isr(spi_bus, MR_SPI_BUS_EVENT_RX_INT);
        SPI_I2S_ClearITPendingBit(driver->info.Instance, SPI_I2S_IT_RXNE);
    }

    if (SPI_I2S_GetITStatus(driver->info.Instance, SPI_I2S_IT_TXE) != RESET)
    {
        mr_spi_bus_isr(spi_bus, MR_SPI_BUS_EVENT_TX_INT);
        SPI_I2S_ClearITPendingBit(driver->info.Instance, SPI_I2S_IT_TXE);
    }
}

#ifdef BSP_SPI_1
void SPI1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI1_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[SPI1_INDEX]);
}
#endif

#ifdef BSP_SPI_2
void SPI2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI2_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[SPI2_INDEX]);
}
#endif

#ifdef BSP_SPI_3
void SPI3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SPI3_IRQHandler(void)
{
    ch32_spi_isr(&spi_bus_device[SPI3_INDEX]);
}
#endif

mr_err_t ch32_spi_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(spi_bus_device);
    static struct mr_spi_bus_ops driver =
            {
                    ch32_spi_configure,
                    ch32_spi_write,
                    ch32_spi_read,
                    ch32_spi_cs_crtl,
                    ch32_spi_cs_read,
                    ch32_spi_start_tx,
                    ch32_spi_stop_tx,
            };

    while (count--)
    {
        ret = mr_spi_bus_add(&spi_bus_device[count], ch32_spi[count].name, &ch32_spi[count], &driver);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_spi_init);

#endif /* MR_CONF_SPI */