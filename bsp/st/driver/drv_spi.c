/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_spi.h"

#ifdef MR_USING_SPI

#if !defined(MR_USING_SPI1) && !defined(MR_USING_SPI2) && !defined(MR_USING_SPI3) && !defined(MR_USING_SPI4) && \
    !defined(MR_USING_SPI5) && !defined(MR_USING_SPI6)
#warning "Please enable at least one SPI driver"
#endif /* !defined(MR_USING_SPI1) && !defined(MR_USING_SPI2) && !defined(MR_USING_SPI3) && !defined(MR_USING_SPI4) && \
        * !defined(MR_USING_SPI5) && !defined(MR_USING_SPI6) */

enum drv_spi_bus_index
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
#ifdef MR_USING_SPI4
    DRV_INDEX_SPI4,
#endif /* MR_USING_SPI4 */
#ifdef MR_USING_SPI5
    DRV_INDEX_SPI5,
#endif /* MR_USING_SPI5 */
#ifdef MR_USING_SPI6
    DRV_INDEX_SPI6,
#endif /* MR_USING_SPI6 */
    DRV_INDEX_SPI_MAX
};

static const char *spi_bus_path[] =
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
#ifdef MR_USING_SPI4
        "spi4",
#endif /* MR_USING_SPI4 */
#ifdef MR_USING_SPI5
        "spi5",
#endif /* MR_USING_SPI5 */
#ifdef MR_USING_SPI6
        "spi6",
#endif /* MR_USING_SPI6 */
    };

static struct drv_spi_bus_data spi_bus_drv_data[] =
    {
#ifdef MR_USING_SPI1
        {{0}, SPI1, SPI1_IRQn},
#endif /* MR_USING_SPI1 */
#ifdef MR_USING_SPI2
        {{0}, SPI2, SPI2_IRQn},
#endif /* MR_USING_SPI2 */
#ifdef MR_USING_SPI3
        {{0}, SPI3, SPI3_IRQn},
#endif /* MR_USING_SPI3 */
#ifdef MR_USING_SPI4
        {{0}, SPI4, SPI4_IRQn},
#endif /* MR_USING_SPI4 */
#ifdef MR_USING_SPI5
        {{0}, SPI5, SPI5_IRQn},
#endif /* MR_USING_SPI5 */
#ifdef MR_USING_SPI6
        {{0}, SPI6, SPI6_IRQn},
#endif /* MR_USING_SPI6 */
    };

static struct mr_spi_bus spi_bus_dev[MR_ARRAY_NUM(spi_bus_drv_data)];

static int drv_spi_bus_configure(struct mr_spi_bus *spi_bus, struct mr_spi_config *config)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->dev.drv->data;
    int state = (config->baud_rate == 0) ? DISABLE : ENABLE;
    spi_bus_data->handle.Instance = spi_bus_data->instance;
    uint32_t pclk = 0, psc = 0;

    /* Configure clock */
    if ((uint32_t)spi_bus_data->instance > APB2PERIPH_BASE)
    {
        pclk = HAL_RCC_GetPCLK2Freq();
    } else
    {
        pclk = HAL_RCC_GetPCLK1Freq();
    }

    if (state == ENABLE)
    {
        psc = pclk / config->baud_rate;
        if (psc >= 256)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
        } else if (psc >= 128)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
        } else if (psc >= 64)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
        } else if (psc >= 32)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
        } else if (psc >= 16)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
        } else if (psc >= 8)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
        } else if (psc >= 4)
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
        } else
        {
            spi_bus_data->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
        }

        switch (config->host_slave)
        {
            case MR_SPI_HOST:
            {
                spi_bus_data->handle.Init.Mode = SPI_MODE_MASTER;
                spi_bus_data->handle.Init.NSS = SPI_NSS_SOFT;
                break;
            }
            case MR_SPI_SLAVE:
            {
                spi_bus_data->handle.Init.Mode = SPI_MODE_SLAVE;
                spi_bus_data->handle.Init.NSS = SPI_NSS_HARD_INPUT;
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
            {
                return MR_EINVAL;
            }
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
            {
                return MR_EINVAL;
            }
        }

        /* Configure SPI */
        spi_bus_data->handle.Init.DataSize = SPI_DATASIZE_8BIT;
        spi_bus_data->handle.Init.Direction = SPI_DIRECTION_2LINES;
        spi_bus_data->handle.Init.TIMode = SPI_TIMODE_DISABLE;
        spi_bus_data->handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
        spi_bus_data->handle.Init.CRCPolynomial = 7;
        HAL_SPI_Init(&spi_bus_data->handle);
        __HAL_SPI_ENABLE(&spi_bus_data->handle);

        /* Configure NVIC */
        if (config->host_slave == MR_SPI_SLAVE)
        {
            HAL_NVIC_SetPriority(spi_bus_data->irq, 1, 0);
            HAL_NVIC_EnableIRQ(spi_bus_data->irq);
            __HAL_SPI_ENABLE_IT(&spi_bus_data->handle, SPI_IT_RXNE);
        } else
        {
            HAL_NVIC_DisableIRQ(spi_bus_data->irq);
            __HAL_SPI_DISABLE_IT(&spi_bus_data->handle, SPI_IT_RXNE);
        }
    } else
    {
        /* Configure SPI */
        HAL_SPI_DeInit(&spi_bus_data->handle);
        __HAL_SPI_DISABLE(&spi_bus_data->handle);

        /* Configure NVIC */
        HAL_NVIC_DisableIRQ(spi_bus_data->irq);
        __HAL_SPI_DISABLE_IT(&spi_bus_data->handle, SPI_IT_RXNE);
    }
    return MR_EOK;
}

static int drv_spi_bus_read(struct mr_spi_bus *spi_bus, uint8_t *data)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->dev.drv->data;
    size_t i = 0;

    while (__HAL_SPI_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    *data = (uint8_t)(spi_bus_data->handle.Instance->DR & 0xff);
    return MR_EOK;
}

static int drv_spi_bus_write(struct mr_spi_bus *spi_bus, uint8_t data)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->dev.drv->data;
    size_t i = 0;

    spi_bus_data->handle.Instance->DR = data;
    while (__HAL_SPI_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_TXE) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    return MR_EOK;
}

static void drv_spi_bus_isr(struct mr_spi_bus *spi_bus)
{
    struct drv_spi_bus_data *spi_bus_data = (struct drv_spi_bus_data *)spi_bus->dev.drv->data;

    if ((__HAL_SPI_GET_FLAG(&spi_bus_data->handle, SPI_FLAG_RXNE) != RESET) &&
        (__HAL_SPI_GET_IT_SOURCE(&spi_bus_data->handle, SPI_IT_RXNE) != RESET))
    {
        mr_dev_isr(&spi_bus->dev, MR_ISR_SPI_RD_INT, NULL);
    }
}

#ifdef MR_USING_SPI1
void SPI1_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI1]);
}
#endif /* MR_USING_SPI1 */

#ifdef MR_USING_SPI2
void SPI2_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI2]);
}
#endif /* MR_USING_SPI2 */

#ifdef MR_USING_SPI3
void SPI3_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI3]);
}
#endif /* MR_USING_SPI3 */

#ifdef MR_USING_SPI4
void SPI4_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI4]);
}
#endif /* MR_USING_SPI4 */

#ifdef MR_USING_SPI5
void SPI5_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI5]);
}
#endif /* MR_USING_SPI5 */

#ifdef MR_USING_SPI6
void SPI6_IRQHandler(void)
{
    drv_spi_bus_isr(&spi_bus_dev[DRV_INDEX_SPI6]);
}
#endif /* MR_USING_SPI6 */

static struct mr_spi_bus_ops spi_bus_drv_ops =
    {
        drv_spi_bus_configure,
        drv_spi_bus_read,
        drv_spi_bus_write,
    };

static struct mr_drv spi_bus_drv[] =
    {
#ifdef MR_USING_SPI1
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI1],
        },
#endif /* MR_USING_SPI1 */
#ifdef MR_USING_SPI2
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI2],
        },
#endif /* MR_USING_SPI2 */
#ifdef MR_USING_SPI3
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI3],
        },
#endif /* MR_USING_SPI3 */
#ifdef MR_USING_SPI4
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI4],
        },
#endif /* MR_USING_SPI4 */
#ifdef MR_USING_SPI5
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI5],
        },
#endif /* MR_USING_SPI5 */
#ifdef MR_USING_SPI6
        {
            &spi_bus_drv_ops,
            &spi_bus_drv_data[DRV_INDEX_SPI6],
        },
#endif /* MR_USING_SPI6 */
    };

static void drv_spi_bus_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(spi_bus_dev); i++)
    {
        mr_spi_bus_register(&spi_bus_dev[i], spi_bus_path[i], &spi_bus_drv[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_spi_bus_init);

#endif /* MR_USING_SPI */
