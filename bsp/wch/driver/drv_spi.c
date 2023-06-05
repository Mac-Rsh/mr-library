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
                  GPIO_Pin_7}},
#endif
#ifdef BSP_SPI_2
                {"spi2",
                 {SPI2,
                  RCC_APB1Periph_SPI2,
                  RCC_APB2Periph_GPIOB,
                  GPIOB,
                  GPIO_Pin_13,
                  GPIO_Pin_14,
                  GPIO_Pin_15}},
#endif
#ifdef BSP_SPI_3
                {"spi3",
                 {SPI3,
                  RCC_APB1Periph_SPI3,
                  RCC_APB2Periph_GPIOB,
                  GPIOB,
                  GPIO_Pin_3,
                  GPIO_Pin_4,
                  GPIO_Pin_5}},
#endif
        };

static struct mr_spi_bus hw_spi[mr_array_get_length(ch32_spi)];

static mr_uint16_t ch32_spi_baud_rate_prescaler(mr_uint32_t pclk_freq, mr_uint32_t baud_rate)
{
    mr_uint16_t psc = pclk_freq / baud_rate;

    if (psc >= 256)
    { return SPI_BaudRatePrescaler_256; }
    else if (psc >= 128)
    { return SPI_BaudRatePrescaler_128; }
    else if (psc >= 64)
    { return SPI_BaudRatePrescaler_64; }
    else if (psc >= 32)
    { return SPI_BaudRatePrescaler_32; }
    else if (psc >= 16)
    { return SPI_BaudRatePrescaler_16; }
    else if (psc >= 8)
    { return SPI_BaudRatePrescaler_8; }
    else if (psc >= 4)
    { return SPI_BaudRatePrescaler_4; }
    else if (psc >= 2)
    { return SPI_BaudRatePrescaler_2; }
    return SPI_BaudRatePrescaler_2;
}

static mr_err_t ch32_spi_configure(mr_spi_bus_t spi_bus, struct mr_spi_config *config)
{
    struct ch32_spi *hw = (struct ch32_spi *)spi_bus->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStruct = {0};
    mr_uint32_t pclk_freq = 0;

    RCC_GetClocksFreq(&RCC_ClockStruct);

    if ((uint32_t)hw->hw_spi.Instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(hw->hw_spi.spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK2_Frequency;
    }
    else
    {
        RCC_APB1PeriphClockCmd(hw->hw_spi.spi_periph_clock, ENABLE);
        pclk_freq = RCC_ClockStruct.PCLK1_Frequency;
    }
    RCC_APB2PeriphClockCmd(hw->hw_spi.gpio_periph_clock, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    if (config->cs_active == MR_SPI_CS_ACTIVE_NONE)
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
    }
    else
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
            break;
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
            break;
    }

    if (config->host_slave == MR_SPI_HOST)
    {
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.clk_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.miso_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.mosi_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);
    }
    else
    {
        SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.clk_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.miso_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = hw->hw_spi.mosi_gpio_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(hw->hw_spi.gpio_port, &GPIO_InitStructure);
    }

    SPI_InitStructure.SPI_BaudRatePrescaler = ch32_spi_baud_rate_prescaler(pclk_freq, config->baud_rate);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_Init(hw->hw_spi.Instance, &SPI_InitStructure);
    SPI_Cmd(hw->hw_spi.Instance, ENABLE);

    return MR_ERR_OK;
}

static mr_uint8_t ch32_spi_transfer(mr_spi_bus_t spi_bus, mr_uint8_t data)
{
    struct ch32_spi *hw = (struct ch32_spi *)spi_bus->device.data;
    mr_size_t i = 0;

    while (SPI_I2S_GetFlagStatus(hw->hw_spi.Instance, SPI_I2S_FLAG_TXE) == RESET)
    {
        i++;
        if (i > 200)
        { return 0; }
    }
    SPI_I2S_SendData(hw->hw_spi.Instance, data);

    while (SPI_I2S_GetFlagStatus(hw->hw_spi.Instance, SPI_I2S_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > 200)
        { return 0; }
    }

    return SPI_I2S_ReceiveData(hw->hw_spi.Instance);
}

static void ch32_spi_cs_crtl(mr_spi_bus_t spi_bus, mr_uint16_t cs_pin, mr_uint8_t state)
{
    if (state == MR_ENABLE)
    {
        GPIO_WriteBit(PIN_STPORT(cs_pin), PIN_STPIN(cs_pin), spi_bus->config.cs_active);
    }
    else
    {
        GPIO_WriteBit(PIN_STPORT(cs_pin), PIN_STPIN(cs_pin), !spi_bus->config.cs_active);
    }
}

mr_err_t ch32_spi_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(hw_spi);
    static struct mr_spi_bus_ops ops =
            {
                    ch32_spi_configure,
                    ch32_spi_transfer,
                    ch32_spi_cs_crtl,
            };

    while (count--)
    {
        ret = mr_spi_bus_add(&hw_spi[count], ch32_spi[count].name, &ops, &ch32_spi[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_spi_init);

#endif