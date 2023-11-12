/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_spi.h"

#ifdef MR_USING_SPI

#if !defined(MR_USING_SPI1) && !defined(MR_USING_SPI2) && !defined(MR_USING_SPI3)
#error "Please define at least one SPI macro like MR_USING_SPI1. Otherwise undefine MR_USING_GPIO."
#endif

enum spi_drv_index
{
#ifdef MR_USING_SPI1
    DRV_INDEX_SPI1,
#endif /* MR_USING_SPI1 */
#ifdef MR_USING_SPI2
    DRV_INDEX_SPI2,
#endif /* MR_USING_SPI2 */
#ifdef MR_USING_SPI3
    DRV_INDEX_SPI3,
#endif /* MR_USING_SPI3 */
};

static const char *spi_name[] =
    {
#ifdef MR_USING_SPI1
        "spi1",
#endif /* MR_USING_SPI1 */
#ifdef MR_USING_SPI2
        "spi2",
#endif /* MR_USING_SPI2 */
#ifdef MR_USING_SPI3
        "spi3",
#endif /* MR_USING_SPI3 */
    };

static struct drv_spi_bus_data spi_drv_data[] =
    {
#ifdef MR_USING_SPI1
        #if (MR_CFG_SPI1_GROUP == 1)
        {
            SPI1,
            RCC_APB2Periph_SPI1,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_5,
            GPIOA,
            GPIO_Pin_6,
            GPIOA,
            GPIO_Pin_7,
            SPI1_IRQn,
            0
        },
#elif (MR_CFG_SPI1_GROUP == 2)
        {
            SPI1,
            RCC_APB2Periph_SPI1,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_3,
            GPIOB,
            GPIO_Pin_4,
            GPIOB,
            GPIO_Pin_5,
            SPI1_IRQn,
            GPIO_Remap_SPI1
        },
#else
#error "MR_CFG_SPI1_GROUP is not defined or defined incorrectly (support values: 1, 2)."
#endif /* MR_CFG_SPI1_GROUP */
#endif /* MR_USING_SPI1 */
#ifdef MR_USING_SPI2
        #if (MR_CFG_SPI2_GROUP == 1)
        {
            SPI2,
            RCC_APB1Periph_SPI2,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_13,
            GPIOB,
            GPIO_Pin_14,
            GPIOB,
            GPIO_Pin_15,
            SPI2_IRQn,
            0
        },
#else
#error "MR_CFG_SPI2_GROUP is not defined or defined incorrectly (support values: 1)."
#endif /* MR_CFG_SPI2_GROUP */
#endif /* MR_USING_SPI2 */
#ifdef MR_USING_SPI3
        #if (MR_CFG_SPI3_GROUP == 1)
        {
            SPI3,
            RCC_APB1Periph_SPI3,
            RCC_APB2Periph_GPIOC,
            GPIOB,
            GPIO_Pin_3,
            GPIOB,
            GPIO_Pin_4,
            GPIOB,
            GPIO_Pin_5,
            SPI3_IRQn,
            0
        },
#elif (MR_CFG_SPI3_GROUP == 2)
        {
            SPI3,
            RCC_APB1Periph_SPI3,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_10,
            GPIOC,
            GPIO_Pin_11,
            GPIOC,
            GPIO_Pin_12,
            SPI3_IRQn,
            GPIO_Remap_SPI3
        },
#else
#error "MR_CFG_SPI3_GROUP is not defined or defined incorrectly (support values: 1, 2)."
#endif /* MR_CFG_SPI3_GROUP */
#endif /* MR_USING_SPI3 */
    };

static struct mr_spi_bus spi_bus_dev[mr_array_num(spi_drv_data)];

static int drv_spi_bus_configure(struct mr_spi_bus *spi_bus, struct mr_spi_config *config)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->dev.drv->data;
    int state = (config->baud_rate == 0) ? MR_DISABLE : MR_ENABLE;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStructure = {0};
    uint32_t pclk = 0, psc = 0;

    /* Configure clock */
    RCC_APB2PeriphClockCmd(spi_bus_data->gpio_clock, ENABLE);
    RCC_GetClocksFreq(&RCC_ClockStructure);
    if ((uint32_t)spi_bus_data->instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(spi_bus_data->clock, state);
        pclk = RCC_ClockStructure.PCLK2_Frequency;
    } else
    {
        RCC_APB1PeriphClockCmd(spi_bus_data->clock, state);
        pclk = RCC_ClockStructure.PCLK1_Frequency;
    }

    psc = pclk / config->baud_rate;
    if (psc >= 256)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    } else if (psc >= 128)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    } else if (psc >= 64)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    } else if (psc >= 32)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    } else if (psc >= 16)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    } else if (psc >= 8)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    } else if (psc >= 4)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    } else if (psc >= 2)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    }

    /* Configure remap */
    if (spi_bus_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(spi_bus_data->remap, state);
    }

    if (state == MR_ENABLE)
    {
        switch (config->host_slave)
        {
            case MR_SPI_HOST:
            {
                SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->sck_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(spi_bus_data->sck_port, &GPIO_InitStructure);

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->miso_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
                GPIO_Init(spi_bus_data->miso_port, &GPIO_InitStructure);

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->mosi_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(spi_bus_data->mosi_port, &GPIO_InitStructure);
                break;
            }

            case MR_SPI_SLAVE:
            {
                SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->sck_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(spi_bus_data->sck_port, &GPIO_InitStructure);

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->miso_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(spi_bus_data->miso_port, &GPIO_InitStructure);

                GPIO_InitStructure.GPIO_Pin = spi_bus_data->mosi_pin;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(spi_bus_data->mosi_port, &GPIO_InitStructure);
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
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
            {
                return MR_EINVAL;
            }
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
            {
                return MR_EINVAL;
            }
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
            {
                return MR_EINVAL;
            }
        }
    } else
    {
        GPIO_InitStructure.GPIO_Pin = spi_bus_data->sck_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(spi_bus_data->sck_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = spi_bus_data->miso_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(spi_bus_data->miso_port, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = spi_bus_data->mosi_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(spi_bus_data->mosi_port, &GPIO_InitStructure);
    }

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = spi_bus_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    if (config->host_slave == MR_SPI_HOST)
    {
        SPI_I2S_ITConfig(spi_bus_data->instance, SPI_I2S_IT_RXNE, DISABLE);
    } else
    {
        SPI_I2S_ITConfig(spi_bus_data->instance, SPI_I2S_IT_RXNE, state);
    }

    /* Configure SPI */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(spi_bus_data->instance, &SPI_InitStructure);
    SPI_Cmd(spi_bus_data->instance, state);
    return MR_EOK;
}

#endif /* MR_USING_SPI */
