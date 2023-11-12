/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_uart.h"

#ifdef MR_USING_UART

#if !defined(MR_USING_UART1) && !defined(MR_USING_UART2) && !defined(MR_USING_UART3) && !defined(MR_USING_UART4) && !defined(MR_USING_UART5) && !defined(MR_USING_UART6) && !defined(MR_USING_UART7) && !defined(MR_USING_UART8)
#error "Please define at least one UART macro like MR_USING_UART1. Otherwise undefine MR_USING_UART."
#endif

enum uart_drv_index
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
};

static const char *uart_name[] =
    {
#ifdef MR_USING_UART1
        "uart1",
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        "uart2",
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        "uart3",
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        "uart4",
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        "uart5",
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        "uart6",
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        "uart7",
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        "uart8",
#endif /* MR_USING_UART8 */
    };

static struct drv_uart_data uart_drv_data[] =
    {
#ifdef MR_USING_UART1
#if (MR_CFG_UART1_GROUP == 1)
        {
            USART1,
            RCC_APB2Periph_USART1,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_10,
            GPIOA,
            GPIO_Pin_9,
            USART1_IRQn,
            0
        },
#elif (MR_CFG_UART1_GROUP == 2)
        {
            USART1,
            RCC_APB2Periph_USART1,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_7,
            GPIOA,
            GPIO_Pin_6,
            USART1_IRQn,
            GPIO_Remap_USART1
        },
#elif (MR_CFG_UART1_GROUP == 3)
        {
            USART1,
            RCC_APB2Periph_USART1,
            RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,
            GPIOA,
            GPIO_Pin_8,
            GPIOB,
            GPIO_Pin_15,
            USART1_IRQn,
            0
        },
#elif (MR_CFG_UART1_GROUP == 4)
        {
            USART1,
            RCC_APB2Periph_USART1,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_7,
            GPIOA,
            GPIO_Pin_6,
            USART1_IRQn,
            GPIO_Remap_USART1
        },
#else
#error "MR_CFG_UART1_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3, 4)."
#endif /* MR_CFG_UART1_GROUP */
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
#if (MR_CFG_UART2_GROUP == 1)
        {
            USART2,
            RCC_APB1Periph_USART2,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_3,
            GPIOA,
            GPIO_Pin_2,
            USART2_IRQn,
            0
        },
#elif (MR_CFG_UART2_GROUP == 2)
        {
            USART2,
            RCC_APB1Periph_USART2,
            RCC_APB2Periph_GPIOD,
            GPIOD,
            GPIO_Pin_6,
            GPIOD,
            GPIO_Pin_5,
            USART2_IRQn,
            GPIO_Remap_USART2
        },
#else
#error "MR_CFG_UART2_GROUP is not defined or defined incorrectly (supported values: 1, 2)."
#endif /* MR_CFG_UART2_GROUP */
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
#if (MR_CFG_UART3_GROUP == 1)
        {
            USART3,
            RCC_APB1Periph_USART3,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_11,
            GPIOB,
            GPIO_Pin_10,
            USART3_IRQn,
            0
        },
#elif (MR_CFG_UART3_GROUP == 2)
        {
            USART3,
            RCC_APB1Periph_USART3,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_11,
            GPIOC,
            GPIO_Pin_10,
            USART3_IRQn,
            GPIO_PartialRemap_USART3
        },
#elif (MR_CFG_UART3_GROUP == 3)
        {
            USART3,
            RCC_APB1Periph_USART3,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_14,
            GPIOA,
            GPIO_Pin_13,
            USART3_IRQn,
            GPIO_PartialRemap1_USART3
        },
#elif (MR_CFG_UART3_GROUP == 4)
        {
            USART3,
            RCC_APB1Periph_USART3,
            RCC_APB2Periph_GPIOD,
            GPIOD,
            GPIO_Pin_9,
            GPIOD,
            GPIO_Pin_8,
            USART3_IRQn,
            GPIO_FullRemap_USART3
        },
#else
#error "MR_CFG_UART3_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3, 4)."
#endif /* MR_CFG_UART3_GROUP */
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
#if (MR_CFG_UART4_GROUP == 1)
        {
            UART4,
            RCC_APB1Periph_UART4,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_11,
            GPIOC,
            GPIO_Pin_10,
            UART4_IRQn,
            0
        },
#elif (MR_CFG_UART4_GROUP == 2)
        {
            UART4,
            RCC_APB1Periph_UART4,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_1,
            GPIOB,
            GPIO_Pin_0,
            UART4_IRQn,
            GPIO_PartialRemap_USART4,
        },
#elif (MR_CFG_UART4_GROUP == 3)
        {
            UART4,
            RCC_APB1Periph_UART4,
            RCC_APB2Periph_GPIOE,
            GPIOE,
            GPIO_Pin_1,
            GPIOE,
            GPIO_Pin_0,
            UART4_IRQn,
            GPIO_FullRemap_USART4
        },
#else
#error "MR_CFG_UART4_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3)."
#endif /* MR_USING_UART4_GROUP */
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
#if (MR_CFG_UART5_GROUP == 1)
        {
            UART5,
            RCC_APB1Periph_UART5,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_13,
            GPIOC,
            GPIO_Pin_12,
            UART5_IRQn,
            0
        },
#elif (MR_CFG_UART5_GROUP == 2)
        {
            UART5,
            RCC_APB1Periph_UART5,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_5,
            GPIOB,
            GPIO_Pin_4,
            UART5_IRQn,
            GPIO_PartialRemap_USART5
        },
#elif (MR_CFG_UART5_GROUP == 3)
        {
            UART5,
            RCC_APB1Periph_UART5,
            RCC_APB2Periph_GPIOE,
            GPIOE,
            GPIO_Pin_9,
            GPIOE,
            GPIO_Pin_8,
            UART5_IRQn,
            GPIO_FullRemap_USART5
        },
#else
#error "MR_CFG_UART5_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3)."
#endif /* MR_CFG_UART5_GROUP */
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
#if (MR_CFG_UART6_GROUP == 1)
        {
            UART6,
            RCC_APB1Periph_UART6,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_1,
            GPIOC,
            GPIO_Pin_0,
            UART6_IRQn,
            0
        },
#elif (MR_CFG_UART6_GROUP == 2)
        {
            UART6,
            RCC_APB1Periph_UART6,
            RCC_APB2Periph_GPIOB,
            GPIOB,
            GPIO_Pin_9,
            GPIOB,
            GPIO_Pin_8,
            UART6_IRQn,
            GPIO_PartialRemap_USART6
        },
#elif (MR_CFG_UART6_GROUP == 3)
        {
            UART6,
            RCC_APB1Periph_UART6,
            RCC_APB2Periph_GPIOE,
            GPIOE,
            GPIO_Pin_11,
            GPIOE,
            GPIO_Pin_10,
            UART6_IRQn,
            GPIO_FullRemap_USART6
        },
#else
#error "MR_CFG_UART6_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3)."
#endif /* MR_CFG_UART6_GROUP */
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
#if (MR_CFG_UART7_GROUP == 1)
        {
            UART7,
            RCC_APB1Periph_UART7,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_3,
            GPIOE,
            GPIO_Pin_2,
            UART7_IRQn,
            0
        },
#elif (MR_CFG_UART7_GROUP == 2)
        {
            UART7,
            RCC_APB1Periph_UART7,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_7,
            GPIOA,
            GPIO_Pin_6,
            UART7_IRQn,
            GPIO_PartialRemap_USART7
        },
#elif (MR_CFG_UART7_GROUP == 3)
        {
            UART7,
            RCC_APB1Periph_UART7,
            RCC_APB2Periph_GPIOE,
            GPIOE,
            GPIO_Pin_13,
            GPIOE,
            GPIO_Pin_12,
            UART7_IRQn,
            GPIO_FullRemap_USART7
        },
#else
#error "MR_CFG_UART7_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3)."
#endif /* MR_CFG_UART7_GROUP */
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
#if (MR_CFG_UART8_GROUP == 1)
        {
            UART8,
            RCC_APB1Periph_UART8,
            RCC_APB2Periph_GPIOC,
            GPIOC,
            GPIO_Pin_5,
            GPIOC,
            GPIO_Pin_4,
            UART8_IRQn,
            0
        },
#elif (MR_CFG_UART8_GROUP == 2)
        {
            UART8,
            RCC_APB1Periph_UART8,
            RCC_APB2Periph_GPIOA,
            GPIOA,
            GPIO_Pin_15,
            GPIOA,
            GPIO_Pin_14,
            UART8_IRQn,
            GPIO_PartialRemap_USART8
        },
#elif (MR_CFG_UART8_GROUP == 3)
        {
            UART8,
            RCC_APB1Periph_UART8,
            RCC_APB2Periph_GPIOE,
            GPIOE,
            GPIO_Pin_15,
            GPIOE,
            GPIO_Pin_14,
            UART8_IRQn,
            GPIO_FullRemap_USART8
        },
#else
#error "MR_CFG_UART8_GROUP is not defined or defined incorrectly (supported values: 1, 2, 3)."
#endif /* MR_CFG_UART8_GROUP */
#endif /* MR_USING_UART8 */
    };

static struct mr_uart uart_dev[mr_array_num(uart_drv_data)];

static int drv_uart_configure(struct mr_uart *uart, struct mr_uart_config *config)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;
    int state = (config->baud_rate == 0) ? MR_DISABLE : MR_ENABLE;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(uart_data->gpio_clock, ENABLE);
    if ((uint32_t)uart_data->instance < APB2PERIPH_BASE)
    {
        RCC_APB1PeriphClockCmd(uart_data->clock, state);
    } else
    {
        RCC_APB2PeriphClockCmd(uart_data->clock, state);
    }

    /* Configure remap */
    if (uart_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(uart_data->remap, state);
    }
#if (MR_USING_UART1_GROUP >= 3)
    if(uart_data->instance == USART1)
    {
        GPIO_PinRemapConfig(GPIO_Remap_USART1_HighBit, state);
    }
#endif /* MR_USING_UART1_GROUP >= 3 */

    if (state == MR_ENABLE)
    {
        switch (config->data_bits)
        {
            case MR_UART_DATA_BITS_8:
            {
                USART_InitStructure.USART_WordLength = USART_WordLength_8b;
                break;
            }

            case MR_UART_DATA_BITS_9:
            {
                USART_InitStructure.USART_WordLength = USART_WordLength_9b;
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->stop_bits)
        {
            case MR_UART_STOP_BITS_1:
            {
                USART_InitStructure.USART_StopBits = USART_StopBits_1;
                break;
            }

            case MR_UART_STOP_BITS_2:
            {
                USART_InitStructure.USART_StopBits = USART_StopBits_2;
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->parity)
        {
            case MR_UART_PARITY_NONE:
            {
                USART_InitStructure.USART_Parity = USART_Parity_No;
                break;
            }

            case MR_UART_PARITY_ODD:
            {
                USART_InitStructure.USART_Parity = USART_Parity_Odd;
                break;
            }

            case MR_UART_PARITY_EVEN:
            {
                USART_InitStructure.USART_Parity = USART_Parity_Even;
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->bit_order)
        {
            case MR_UART_BIT_ORDER_LSB:
            {
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        switch (config->invert)
        {
            case MR_UART_NRZ_NORMAL:
            {
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        /* Configure TX/RX GPIO */
        GPIO_InitStructure.GPIO_Pin = uart_data->tx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(uart_data->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = uart_data->rx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(uart_data->rx_port, &GPIO_InitStructure);
    } else
    {
        /* Reset TX/RX GPIO */
        GPIO_InitStructure.GPIO_Pin = uart_data->tx_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(uart_data->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = uart_data->rx_pin;
        GPIO_Init(uart_data->rx_port, &GPIO_InitStructure);
    }

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = uart_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(uart_data->instance, USART_IT_RXNE, state);
    if (state == MR_DISABLE)
    {
        USART_ITConfig(uart_data->instance, USART_IT_TXE, DISABLE);
    }

    /* Configure UART */
    USART_InitStructure.USART_BaudRate = config->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = 0;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart_data->instance, &USART_InitStructure);
    USART_Cmd(uart_data->instance, state);
    return MR_EOK;
}

static ssize_t drv_uart_read(struct mr_uart *uart, uint8_t *buf, size_t size)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;
    ssize_t rd_size = 0;

    for (rd_size = 0; rd_size < size; rd_size++)
    {
        int i = 0;

        /* Read data */
        while (USART_GetFlagStatus(uart_data->instance, USART_FLAG_RXNE) == RESET)
        {
            i++;
            if (i > UINT16_MAX)
            {
                break;
            }
        }
        buf[rd_size] = uart_data->instance->DATAR & 0xff;
    }
    return rd_size;
}

static ssize_t drv_uart_write(struct mr_uart *uart, const uint8_t *buf, size_t size)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;
    ssize_t wr_size = 0;

    for (wr_size = 0; wr_size < size; wr_size++)
    {
        int i = 0;

        /* Write data */
        while (USART_GetFlagStatus(uart_data->instance, USART_FLAG_TC) == RESET)
        {
            i++;
            if (i > UINT16_MAX)
            {
                break;
            }
        }
        uart_data->instance->DATAR = buf[wr_size];
    }
    return wr_size;
}

static void drv_uart_start_tx(struct mr_uart *uart)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;

    /* Enable TXE */
    USART_ITConfig(uart_data->instance, USART_IT_TXE, ENABLE);
}

static void drv_uart_stop_tx(struct mr_uart *uart)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;

    /* Disable TXE */
    USART_ITConfig(uart_data->instance, USART_IT_TXE, DISABLE);
}

static void drv_uart_isr(struct mr_uart *uart)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)uart->dev.drv->data;

    if (USART_GetITStatus(uart_data->instance, USART_IT_RXNE) != RESET)
    {
        mr_dev_isr(&uart->dev, MR_ISR_EVENT_RD_INTER, NULL);
        USART_ClearITPendingBit(uart_data->instance, USART_IT_RXNE);
    }

    if (USART_GetITStatus(uart_data->instance, USART_IT_TXE) != RESET)
    {
        mr_dev_isr(&uart->dev, MR_ISR_EVENT_WR_INTER, NULL);
        USART_ClearITPendingBit(uart_data->instance, USART_IT_TXE);
    }
}

#ifdef MR_USING_UART1
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART1]);
}
#endif /* MR_USING_UART1 */

#ifdef MR_USING_UART2
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART2]);
}
#endif /* MR_USING_UART2 */

#ifdef MR_USING_UART3
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART3]);
}
#endif /* MR_USING_UART3 */

#ifdef MR_USING_UART4
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART4]);
}
#endif /* MR_USING_UART4 */

#ifdef MR_USING_UART5
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART5]);
}
#endif /* MR_USING_UART5 */

#ifdef MR_USING_UART6
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART6]);
}
#endif /* MR_USING_UART6 */

#ifdef MR_USING_UART7
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART7]);
}
#endif /* MR_USING_UART7 */

#ifdef MR_USING_UART8
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART8_IRQHandler(void)
{
    drv_uart_isr(&uart_dev[DRV_INDEX_UART8]);
}
#endif /* MR_USING_UART8 */

static struct mr_uart_ops uart_drv_ops =
    {
        drv_uart_configure,
        drv_uart_read,
        drv_uart_write,
        drv_uart_start_tx,
        drv_uart_stop_tx
    };

static struct mr_drv uart_drv[mr_array_num(uart_drv_data)] =
    {
#ifdef MR_USING_UART1
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART1]
        },
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART2]
        },
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART3]
        },
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART4]
        },
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART5]
        },
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART6]
        },
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART7]
        },
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        {
            Mr_Drv_Type_Uart,
            &uart_drv_ops,
            &uart_drv_data[DRV_INDEX_UART8]
        },
#endif /* MR_USING_UART8 */
    };

int drv_uart_init(void)
{
    int index = 0;

    for (index = 0; index < mr_array_num(uart_dev); index++)
    {
        mr_uart_register(&uart_dev[index], uart_name[index], &uart_drv[index]);
    }
    return MR_EOK;
}
MR_INIT_CONSOLE_EXPORT(drv_uart_init);

#endif /* MR_USING_UART */
