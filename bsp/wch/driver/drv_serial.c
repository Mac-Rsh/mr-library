/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "drv_serial.h"

#ifdef MR_USING_SERIAL

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
        DRV_UART1_CONFIG,
#endif /* MR_USING_UART1 */
#ifdef MR_USING_UART2
        DRV_UART2_CONFIG,
#endif /* MR_USING_UART2 */
#ifdef MR_USING_UART3
        DRV_UART3_CONFIG,
#endif /* MR_USING_UART3 */
#ifdef MR_USING_UART4
        DRV_UART4_CONFIG,
#endif /* MR_USING_UART4 */
#ifdef MR_USING_UART5
        DRV_UART5_CONFIG,
#endif /* MR_USING_UART5 */
#ifdef MR_USING_UART6
        DRV_UART6_CONFIG,
#endif /* MR_USING_UART6 */
#ifdef MR_USING_UART7
        DRV_UART7_CONFIG,
#endif /* MR_USING_UART7 */
#ifdef MR_USING_UART8
        DRV_UART8_CONFIG,
#endif /* MR_USING_UART8 */
    };

static struct mr_serial serial_dev[MR_ARRAY_NUM(serial_drv_data)];

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
#if defined(MR_USING_CH32V30X) && (MR_USING_UART1_GROUP >= 3)
    if(serial_data->instance == USART1)
    {
        GPIO_PinRemapConfig(GPIO_Remap_USART1_HighBit, state);
    }
#endif /* defined(MR_USING_CH32V30X) && (MR_USING_UART1_GROUP >= 3) */

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
    USART_ClearITPendingBit(serial_data->instance, USART_IT_RXNE);
    USART_ITConfig(serial_data->instance, USART_IT_RXNE, state);
    if (state == DISABLE)
    {
        USART_ITConfig(serial_data->instance, USART_IT_TXE, DISABLE);
    }
    return MR_EOK;
}

static uint8_t drv_serial_read(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    size_t i = 0;

    /* Read data */
    while (USART_GetFlagStatus(serial_data->instance, USART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return 0;
        }
    }
    return (uint8_t)USART_ReceiveData(serial_data->instance);
}

static void drv_serial_write(struct mr_serial *serial, uint8_t data)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;
    size_t i = 0;

    /* Write data */
    USART_SendData(serial_data->instance, data);
    while (USART_GetFlagStatus(serial_data->instance, USART_FLAG_TC) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return;
        }
    }
}

static void drv_serial_start_tx(struct mr_serial *serial)
{
    struct drv_serial_data *serial_data = (struct drv_serial_data *)serial->dev.drv->data;

    /* Enable TX interrupt */
    USART_ClearITPendingBit(serial_data->instance, USART_IT_TXE);
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

static int drv_serial_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(serial_dev); i++)
    {
        mr_serial_register(&serial_dev[i], serial_name[i], &serial_drv[i]);
    }
    return MR_EOK;
}
MR_INIT_DRV_EXPORT(drv_serial_init);

#endif /* MR_USING_SERIAL */
