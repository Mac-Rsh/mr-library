/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_serial.h"

#ifdef MR_USING_SERIAL

#if !defined(MR_USING_UART1) && !defined(MR_USING_UART2) && !defined(MR_USING_UART3) && !defined(MR_USING_UART4) && \
    !defined(MR_USING_UART5) && !defined(MR_USING_UART6) && !defined(MR_USING_UART7) && !defined(MR_USING_UART8)
#warning "Please enable at least one Serial driver"
#endif /* !defined(MR_USING_UART1) && !defined(MR_USING_UART2) && !defined(MR_USING_UART3) && !defined(MR_USING_UART4) &&  \
        * !defined(MR_USING_UART5) && !defined(MR_USING_UART6) && !defined(MR_USING_UART7) && !defined(MR_USING_UART8) */

enum drv_serial_index
{
#ifdef MR_USING_UART1
    DRV_INDEX_UART1,
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
    DRV_INDEX_UART2,
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
    DRV_INDEX_UART3,
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
    DRV_INDEX_UART4,
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
    DRV_INDEX_UART5,
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
    DRV_INDEX_UART6,
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
    DRV_INDEX_UART7,
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
    DRV_INDEX_UART8,
#endif /* MR_USING_UART8 */
    DRV_INDEX_UART_MAX
};

static const char *serial_path[] =
    {
#ifdef MR_USING_UART1
        "serial1",
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        "serial2",
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        "serial3",
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        "serial4",
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        "serial5",
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        "serial6",
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        "serial7",
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        "serial8",
#endif /* MR_USING_UART8 */
    };

static struct drv_serial_data serial_drv_data[] =
    {
#ifdef MR_USING_UART1
        {{0}, USART1, USART1_IRQn},
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        {{0}, USART2, USART2_IRQn},
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        {{0}, USART3, USART3_IRQn},
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        {{0}, UART4, UART4_IRQn},
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        {{0}, UART5, UART5_IRQn},
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        {{0}, USART6, USART6_IRQn},
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        {{0}, UART7, UART7_IRQn},
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        {{0}, UART8, UART8_IRQn},
#endif /* MR_USING_UART8 */
    };

static struct mr_serial serial_dev[MR_ARRAY_NUM(serial_drv_data)];

static int drv_serial_configure(struct mr_serial *serial, struct mr_serial_config *config)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    int state = (config->baud_rate == 0) ? DISABLE : ENABLE;
    serial_data->handle.Instance = serial_data->instance;

    if (state == ENABLE)
    {
        switch (config->data_bits)
        {
            case MR_SERIAL_DATA_BITS_8:
            {
                serial_data->handle.Init.WordLength = UART_WORDLENGTH_8B;
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->stop_bits)
        {
            case MR_SERIAL_STOP_BITS_1:
            {
                serial_data->handle.Init.StopBits = UART_STOPBITS_1;
                break;
            }
            case MR_SERIAL_STOP_BITS_2:
            {
                serial_data->handle.Init.StopBits = UART_STOPBITS_2;
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->parity)
        {
            case MR_SERIAL_PARITY_NONE:
            {
                serial_data->handle.Init.Parity = UART_PARITY_NONE;
                break;
            }
            case MR_SERIAL_PARITY_ODD:
            {
                serial_data->handle.Init.Parity = UART_PARITY_ODD;
                break;
            }
            case MR_SERIAL_PARITY_EVEN:
            {
                serial_data->handle.Init.Parity = UART_PARITY_EVEN;
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->bit_order)
        {
            case MR_SERIAL_BIT_ORDER_LSB:
            {
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->polarity)
        {
            case MR_SERIAL_POLARITY_NORMAL:
            {
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        /* Configure UART */
        serial_data->handle.Init.BaudRate = config->baud_rate;
        serial_data->handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        serial_data->handle.Init.Mode = UART_MODE_TX_RX;
        serial_data->handle.Init.OverSampling = UART_OVERSAMPLING_16;
        HAL_UART_Init(&serial_data->handle);
        __HAL_UART_CLEAR_FLAG(&serial_data->handle, UART_FLAG_RXNE);
        __HAL_UART_CLEAR_FLAG(&serial_data->handle, UART_FLAG_TC);

        /* Configure NVIC */
        HAL_NVIC_SetPriority(serial_data->irq, 1, 0);
        HAL_NVIC_EnableIRQ(serial_data->irq);
        __HAL_UART_ENABLE_IT(&serial_data->handle, UART_IT_RXNE);
    } else
    {
        /* Configure UART */
        HAL_UART_DeInit(&serial_data->handle);

        /* Configure NVIC */
        HAL_NVIC_DisableIRQ(serial_data->irq);
        __HAL_UART_DISABLE_IT(&serial_data->handle, UART_IT_RXNE);
        __HAL_UART_DISABLE_IT(&serial_data->handle, UART_IT_TXE);
    }
    return MR_EOK;
}

static int drv_serial_read(struct mr_serial *serial, uint8_t *data)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    size_t i = 0;

    /* Read data */
    while (__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
#if defined(STM32L4) || defined(STM32WL) || defined(STM32F7) || defined(STM32F0) \
 || defined(STM32L0) || defined(STM32G0) || defined(STM32H7) || defined(STM32L5) \
 || defined(STM32G4) || defined(STM32MP1) || defined(STM32WB) || defined(STM32F3)\
 || defined(STM32U5) || defined(STM32H5)
    *data = (uint8_t)serial_data->handle.Instance->RDR & 0xff;
#else
    *data = (uint8_t)serial_data->handle.Instance->DR & 0xff;
#endif
    return MR_EOK;
}

static int drv_serial_write(struct mr_serial *serial, uint8_t data)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    size_t i = 0;

    /* Write data */
#if defined(STM32L4) || defined(STM32WL) || defined(STM32F7) || defined(STM32F0) \
 || defined(STM32L0) || defined(STM32G0) || defined(STM32H7) || defined(STM32L5) \
 || defined(STM32G4) || defined(STM32MP1) || defined(STM32WB) || defined(STM32F3)\
 || defined(STM32U5) || defined(STM32H5)
    serial_data->handle.Instance->TDR = data;
#else
    serial_data->handle.Instance->DR = data;
#endif
    while (__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_TC) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    return MR_EOK;
}

static void drv_serial_start_tx(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    /* Enable TX interrupt */
    __HAL_UART_ENABLE_IT(&serial_data->handle, UART_IT_TXE);
}

static void drv_serial_stop_tx(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    /* Disable TX interrupt */
    __HAL_UART_DISABLE_IT(&serial_data->handle, UART_IT_TXE);
}

static void drv_serial_isr(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    if (__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_ORE) != RESET)
    {
        __HAL_UART_CLEAR_OREFLAG(&serial_data->handle);
    }
    if ((__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_TXE) != RESET)
        && (__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_TC) != RESET))
    {
        mr_dev_isr(&serial->dev, MR_ISR_SERIAL_WR_INT, NULL);
    }
    if ((__HAL_UART_GET_FLAG(&serial_data->handle, UART_FLAG_RXNE) != RESET)
        && (__HAL_UART_GET_IT_SOURCE(&serial_data->handle, UART_IT_RXNE) != RESET))
    {
        mr_dev_isr(&serial->dev, MR_ISR_SERIAL_RD_INT, NULL);
    }
}

#ifdef MR_USING_UART1
void USART1_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART1]);
}
#endif /* MR_USING_UART1 */

#ifdef MR_USING_UART2
void USART2_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART2]);
}
#endif /* MR_USING_UART2 */

#ifdef MR_USING_UART3
void USART3_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART3]);
}
#endif /* MR_USING_UART3 */

#ifdef MR_USING_UART4
void UART4_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART4]);
}
#endif /* MR_USING_UART4 */

#ifdef MR_USING_UART5
void UART5_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART5]);
}
#endif /* MR_USING_UART5 */

#ifdef MR_USING_UART6
void USART6_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART6]);
}
#endif /* MR_USING_UART6 */

#ifdef MR_USING_UART7
void UART7_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART7]);
}
#endif /* MR_USING_UART7 */

#ifdef MR_USING_UART8
void UART8_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART8]);
}
#endif /* MR_USING_UART8 */

static struct mr_serial_ops serial_drv_ops =
    {
        drv_serial_configure,
        drv_serial_read,
        drv_serial_write,
        drv_serial_start_tx,
        drv_serial_stop_tx
    };

static struct mr_drv serial_drv[] =
    {
#ifdef MR_USING_UART1
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART1]
        },
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART2]
        },
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART3]
        },
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART4]
        },
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART5]
        },
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART6]
        },
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART7]
        },
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        {
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART8]
        },
#endif /* MR_USING_UART8 */
    };

static void drv_serial_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(serial_dev); i++)
    {
        mr_serial_register(&serial_dev[i], serial_path[i], &serial_drv[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_serial_init);

#endif /* MR_USING_SERIAL */
