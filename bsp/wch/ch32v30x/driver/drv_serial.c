/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_serial.h"

#ifdef MR_USING_SERIAL

#if !defined(MR_USING_UART1) && !defined(MR_USING_UART2) && !defined(MR_USING_UART3) && !defined(MR_USING_UART4) && !defined(MR_USING_UART5) && !defined(MR_USING_UART6) && !defined(MR_USING_UART7) && !defined(MR_USING_UART8)
#error "Please define at least one UART macro like MR_USING_UART1. Otherwise undefine MR_USING_SERIAL."
#else

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
};

static const char *serial_name[] =
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

static struct mr_serial serial_dev[mr_array_num(serial_drv_data)];

static int drv_serial_configure(struct mr_serial *serial, struct mr_serial_config *config)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    int state = (config->baud_rate == 0) ? DISABLE : ENABLE;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(serial_data->gpio_clock, ENABLE);
    if ((uint32_t)serial_data->instance < APB2PERIPH_BASE)
    {
        RCC_APB1PeriphClockCmd(serial_data->clock, state);
    } else
    {
        RCC_APB2PeriphClockCmd(serial_data->clock, state);
    }

    /* Configure remap */
    if (serial_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(serial_data->remap, state);
    }
#if (MR_USING_UART1_GROUP >= 3)
    if(serial_data->instance == USART1)
    {
        GPIO_PinRemapConfig(GPIO_Remap_USART1_HighBit, state);
    }
#endif /* MR_USING_UART1_GROUP >= 3 */

    if (state == ENABLE)
    {
        switch (config->data_bits)
        {
            case MR_SERIAL_DATA_BITS_8:
            {
                USART_InitStructure.USART_WordLength = USART_WordLength_8b;
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
                USART_InitStructure.USART_StopBits = USART_StopBits_1;
                break;
            }

            case MR_SERIAL_STOP_BITS_2:
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
            case MR_SERIAL_PARITY_NONE:
            {
                USART_InitStructure.USART_Parity = USART_Parity_No;
                break;
            }

            case MR_SERIAL_PARITY_ODD:
            {
                USART_InitStructure.USART_Parity = USART_Parity_Odd;
                break;
            }

            case MR_SERIAL_PARITY_EVEN:
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
            case MR_SERIAL_BIT_ORDER_LSB:
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
            case MR_SERIAL_NRZ_NORMAL:
            {
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }

        /* Configure TX/RX GPIO */
        GPIO_InitStructure.GPIO_Pin = serial_data->tx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(serial_data->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = serial_data->rx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(serial_data->rx_port, &GPIO_InitStructure);
    } else
    {
        /* Reset TX/RX GPIO */
        GPIO_InitStructure.GPIO_Pin = serial_data->tx_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(serial_data->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = serial_data->rx_pin;
        GPIO_Init(serial_data->rx_port, &GPIO_InitStructure);
    }

    /* Configure UART */
    USART_InitStructure.USART_BaudRate = config->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = 0;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(serial_data->instance, &USART_InitStructure);
    USART_Cmd(serial_data->instance, state);
    USART_ClearFlag(serial_data->instance, USART_FLAG_RXNE);
    USART_ClearFlag(serial_data->instance, USART_FLAG_TC);

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = serial_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(serial_data->instance, USART_IT_RXNE, state);
    if (state == DISABLE)
    {
        USART_ITConfig(serial_data->instance, USART_IT_TXE, DISABLE);
    }
    USART_ClearITPendingBit(serial_data->instance, USART_IT_RXNE);
    USART_ClearITPendingBit(serial_data->instance, USART_IT_TXE);
    return MR_EOK;
}

static uint8_t drv_serial_read(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    int i = 0;

    /* Read data */
    while (USART_GetFlagStatus(serial_data->instance, USART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return 0;
        }
    }
    return (uint8_t)USART_ReceiveData(serial_data->instance);
}

static void drv_serial_write(struct mr_serial *serial, uint8_t data)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    int i = 0;

    /* Write data */
    USART_SendData(serial_data->instance, data);
    while (USART_GetFlagStatus(serial_data->instance, USART_FLAG_TC) == RESET)
    {
        i++;
        if (i > INT16_MAX)
        {
            return;
        }
    }
}

static void drv_serial_start_tx(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    /* Enable TX interrupt */
    USART_ITConfig(serial_data->instance, USART_IT_TXE, ENABLE);
}

static void drv_serial_stop_tx(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    /* Disable TX interrupt */
    USART_ITConfig(serial_data->instance, USART_IT_TXE, DISABLE);
}

static void drv_serial_isr(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    if (USART_GetITStatus(serial_data->instance, USART_IT_RXNE) != RESET)
    {
        mr_dev_isr(&serial->dev, MR_ISR_SERIAL_RD_INT, NULL);
        USART_ClearITPendingBit(serial_data->instance, USART_IT_RXNE);
    }

    if (USART_GetITStatus(serial_data->instance, USART_IT_TXE) != RESET)
    {
        mr_dev_isr(&serial->dev, MR_ISR_SERIAL_WR_INT, NULL);
        USART_ClearITPendingBit(serial_data->instance, USART_IT_TXE);
    }
}

#ifdef MR_USING_UART1
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART1]);
}
#endif /* MR_USING_UART1 */

#ifdef MR_USING_UART2
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART2]);
}
#endif /* MR_USING_UART2 */

#ifdef MR_USING_UART3
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART3]);
}
#endif /* MR_USING_UART3 */

#ifdef MR_USING_UART4
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART4]);
}
#endif /* MR_USING_UART4 */

#ifdef MR_USING_UART5
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART5]);
}
#endif /* MR_USING_UART5 */

#ifdef MR_USING_UART6
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART6]);
}
#endif /* MR_USING_UART6 */

#ifdef MR_USING_UART7
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void)
{
    drv_serial_isr(&serial_dev[DRV_INDEX_UART7]);
}
#endif /* MR_USING_UART7 */

#ifdef MR_USING_UART8
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
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
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART1]
        },
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART2]
        },
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART3]
        },
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART4]
        },
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART5]
        },
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART6]
        },
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART7]
        },
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        {
            Mr_Drv_Type_Serial,
            &serial_drv_ops,
            &serial_drv_data[DRV_INDEX_UART8]
        },
#endif /* MR_USING_UART8 */
    };

int drv_serial_init(void)
{
    int index = 0;

    for (index = 0; index < mr_array_num(serial_dev); index++)
    {
        mr_serial_register(&serial_dev[index], serial_name[index], &serial_drv[index]);
    }
    return MR_EOK;
}
MR_DRV_EXPORT(drv_serial_init);

#endif /* !defined(MR_USING_UART1) && !defined(MR_USING_UART2) && !defined(MR_USING_UART3) && !defined(MR_USING_UART4) && !defined(MR_USING_UART5) && !defined(MR_USING_UART6) && !defined(MR_USING_UART7) && !defined(MR_USING_UART8) */

#endif /* MR_USING_SERIAL */
