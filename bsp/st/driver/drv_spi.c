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

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))

enum drv_spi_index
{
#ifdef MR_BSP_SPI_1
    DRV_SPI_1_INDEX,
#endif
#ifdef MR_BSP_SPI_2
    DRV_SPI_2_INDEX,
#endif
#ifdef MR_BSP_SPI_3
    DRV_SPI_3_INDEX,
#endif
#ifdef MR_BSP_SPI_4
    DRV_SPI_4_INDEX,
#endif
#ifdef MR_BSP_SPI_5
    DRV_SPI_5_INDEX,
#endif
#ifdef MR_BSP_SPI_6
    DRV_SPI_6_INDEX,
#endif
};

static struct drv_spi_bus_data drv_spi_bus_data[] =
    {
#ifdef MR_BSP_SPI_1
        {"spi1", {0}, SPI1, SPI1_IRQn},
#endif
#ifdef MR_BSP_SPI_2
        {"spi2", {0}, SPI2, SPI2_IRQn},
#endif
#ifdef MR_BSP_SPI_3
        {"spi3", {0}, SPI3, SPI3_IRQn},
#endif
#ifdef MR_BSP_SPI_4
        {"spi4", {0}, SPI4, SPI4_IRQn},
#endif
#ifdef MR_BSP_SPI_5
        {"spi5", {0}, SPI5, SPI5_IRQn},
#endif
#ifdef MR_BSP_SPI_6
        {"spi6", {0}, SPI6, SPI6_IRQn},
#endif
    };

static struct mr_spi_bus spi_bus_device[mr_array_num(drv_spi_bus_data)];

static mr_uint16_t stm32_spi_baud_rate_prescaler(mr_uint32_t pclk_freq, mr_uint32_t baud_rate)
{
    mr_uint32_t psc = pclk_freq / baud_rate;

    if (psc >= 256)
    {
        return SPI_BAUDRATEPRESCALER_256;
    } else if (psc >= 128)
    {
        return SPI_BAUDRATEPRESCALER_128;
    } else if (psc >= 64)
    {
        return SPI_BAUDRATEPRESCALER_64;
    } else if (psc >= 32)
    {
        return SPI_BAUDRATEPRESCALER_32;
    } else if (psc >= 16)
    {
        return SPI_BAUDRATEPRESCALER_16;
    } else if (psc >= 8)
    {
        return SPI_BAUDRATEPRESCALER_8;
    } else if (psc >= 4)
    {
        return SPI_BAUDRATEPRESCALER_4;
    } else if (psc >= 2)
    {
        return SPI_BAUDRATEPRESCALER_2;
    }
    return SPI_BAUDRATEPRESCALER_2;
}

static mr_err_t drv_spi_configure(mr_spi_bus_t spi_bus, mr_spi_config_t config)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;
    mr_uint32_t pclk_freq = 0;

    if ((uint32_t)spi_bus_data->instance > APB2PERIPH_BASE)
    {
        pclk_freq = HAL_RCC_GetPCLK2Freq();
    } else
    {
        pclk_freq = HAL_RCC_GetPCLK1Freq();
    }

    spi_bus_data->handle.Instance = spi_bus_data->instance;

    switch (config->host_slave)
    {
        case MR_SPI_HOST:
        {
            spi_bus_data->handle.Init.Mode = SPI_MODE_MASTER;
            break;
        }

        case MR_SPI_SLAVE:
        {
            spi_bus_data->handle.Init.Mode = SPI_MODE_SLAVE;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->mode)
    {
        case MR_SPI_MODE_0:
        {
            spi_bus_data->handle.Init.CLKPolarity = SPI_POLARITY_LOW;
            spi_bus_data->handle.Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        }

        case MR_SPI_MODE_1:
        {
            spi_bus_data->handle.Init.CLKPolarity = SPI_POLARITY_LOW;
            spi_bus_data->handle.Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        }

        case MR_SPI_MODE_2:
        {
            spi_bus_data->handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
            spi_bus_data->handle.Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        }

        case MR_SPI_MODE_3:
        {
            spi_bus_data->handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
            spi_bus_data->handle.Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->data_bits)
    {
        case MR_SPI_DATA_BITS_8:
        {
            spi_bus_data->handle.Init.DataSize = SPI_DATASIZE_8BIT;
            break;
        }

        case MR_SPI_DATA_BITS_16:
        {
            spi_bus_data->handle.Init.DataSize = SPI_DATASIZE_16BIT;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->bit_order)
    {
        case MR_SPI_BIT_ORDER_LSB:
        {
            spi_bus_data->handle.Init.FirstBit = SPI_FIRSTBIT_LSB;
            break;
        }

        case MR_SPI_BIT_ORDER_MSB:
        {
            spi_bus_data->handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->cs_active)
    {
        case MR_SPI_CS_ACTIVE_LOW:
        case MR_SPI_CS_ACTIVE_HIGH:
        {
            spi_bus_data->handle.Init.NSS = SPI_NSS_SOFT;
            break;
        }

        case MR_SPI_CS_ACTIVE_HARDWARE:
        {
            if (config->host_slave == MR_SPI_HOST)
            {
                spi_bus_data->handle.Init.NSS = SPI_NSS_HARD_OUTPUT;
            } else
            {
                spi_bus_data->handle.Init.NSS = SPI_NSS_HARD_INPUT;
            }
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    spi_bus_data->handle.Init.BaudRatePrescaler = stm32_spi_baud_rate_prescaler(pclk_freq, config->baud_rate);
    spi_bus_data->handle.Init.Direction = SPI_DIRECTION_2LINES;
    spi_bus_data->handle.Init.TIMode = SPI_TIMODE_DISABLE;
    spi_bus_data->handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_bus_data->handle.Init.CRCPolynomial = 7;
    if (HAL_SPI_Init(&spi_bus_data->handle) != HAL_OK)
    {
        return MR_ERR_GENERIC;
    }

    if (config->baud_rate != 0)
    {
        __HAL_SPI_ENABLE(&spi_bus_data->handle);
    } else
    {
        __HAL_SPI_DISABLE(&spi_bus_data->handle);
    }

    if (config->host_slave == MR_SPI_HOST)
    {
        HAL_NVIC_SetPriority(spi_bus_data->irq_type, 1, 0);
        HAL_NVIC_DisableIRQ(spi_bus_data->irq_type);
        __HAL_UART_DISABLE_IT(&spi_bus_data->handle, SPI_IT_RXNE);
    } else
    {
        HAL_NVIC_SetPriority(spi_bus_data->irq_type, 1, 0);
        HAL_NVIC_EnableIRQ(spi_bus_data->irq_type);
        __HAL_UART_ENABLE_IT(&spi_bus_data->handle, SPI_IT_RXNE);
    }

    return MR_ERR_OK;
}

static void drv_spi_write(mr_spi_bus_t spi_bus, mr_uint32_t data)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;
    mr_size_t i = 0;

    while (__HAL_SPI_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_TXE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return;
        }
    }
    spi_bus_data->instance->DR = data;
}

static mr_uint32_t drv_spi_read(mr_spi_bus_t spi_bus)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;
    mr_size_t i = 0;

    while (__HAL_SPI_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }

    return (mr_uint32_t)spi_bus_data->instance->DR;
}

static void drv_spi_cs_write(mr_spi_bus_t spi_bus, mr_off_t cs_number, mr_level_t level)
{
    if (cs_number > MR_BSP_PIN_NUMBER)
    {
        return;
    }

    HAL_GPIO_WritePin(PIN_STPORT(cs_number), PIN_STPIN(cs_number), (GPIO_PinState)level);
}

static mr_level_t drv_spi_cs_read(mr_spi_bus_t spi_bus, mr_off_t cs_number)
{
    if (cs_number > MR_BSP_PIN_NUMBER)
    {
        return 0;
    }

    return (mr_level_t)HAL_GPIO_ReadPin(PIN_STPORT(cs_number), PIN_STPIN(cs_number));
}

static void drv_spi_isr(mr_spi_bus_t spi_bus)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->device.data;

    if (__HAL_UART_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_RXNE) != RESET &&
        __HAL_UART_GET_IT_SOURCE(&spi_bus_data->handle, SPI_IT_RXNE) != RESET)
    {
        mr_spi_bus_isr(spi_bus, MR_SPI_BUS_EVENT_RX_INT);
    }
}

#ifdef MR_BSP_SPI_1
void SPI1_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_1_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_2
void SPI2_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_2_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_3
void SPI3_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_3_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_4
void SPI4_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_4_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_5
void SPI5_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_5_INDEX]);
}
#endif

#ifdef MR_BSP_SPI_6
void SPI6_IRQHandler(void)
{
    drv_spi_isr(&spi_bus_device[DRV_SPI_6_INDEX]);
}
#endif

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
