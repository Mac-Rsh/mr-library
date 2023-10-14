/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "drv_uart.h"

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

enum
{
#ifdef MR_BSP_UART_1
    DRV_UART_1_INDEX,
#endif
#ifdef MR_BSP_UART_2
    DRV_UART_2_INDEX,
#endif
#ifdef MR_BSP_UART_3
    DRV_UART_3_INDEX,
#endif
#ifdef MR_BSP_UART_4
    DRV_UART_4_INDEX,
#endif
#ifdef MR_BSP_UART_5
    DRV_UART_5_INDEX,
#endif
#ifdef MR_BSP_UART_6
    DRV_UART_6_INDEX,
#endif
#ifdef MR_BSP_UART_7
    DRV_UART_7_INDEX,
#endif
#ifdef MR_BSP_UART_8
    DRV_UART_8_INDEX,
#endif
};

static struct drv_uart_data drv_uart_data[] =
    {
#ifdef MR_BSP_UART_1
        {"uart1", USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_9, GPIOA, GPIO_Pin_10,
         USART1_IRQn},
#endif
#ifdef MR_BSP_UART_2
        {"uart2", USART2, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3,
         USART2_IRQn},
#endif
#ifdef MR_BSP_UART_3
        {"uart3", USART3, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_10, GPIOB, GPIO_Pin_11,
         USART3_IRQn},
#endif
#ifdef MR_BSP_UART_4
        {"uart4", UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_10, GPIOC, GPIO_Pin_11,
         UART4_IRQn},
#endif
#ifdef MR_BSP_UART_5
        {"uart5", UART5, RCC_APB1Periph_UART5, RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, GPIOC, GPIO_Pin_12, GPIOD,
         GPIO_Pin_2, UART5_IRQn},
#endif
#ifdef MR_BSP_UART_6
        {"uart6", UART6, RCC_APB1Periph_UART6, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_0, GPIOC, GPIO_Pin_1, UART6_IRQn},
#endif
#ifdef MR_BSP_UART_7
        {"uart7", UART7, RCC_APB1Periph_UART7, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_2, GPIOC, GPIO_Pin_3, UART7_IRQn},
#endif
#ifdef MR_BSP_UART_8
        {"uart8", UART8, RCC_APB1Periph_UART8, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_4, GPIOC, GPIO_Pin_5, UART8_IRQn},
#endif
    };

static struct mr_serial serial_device[mr_array_num(drv_uart_data)];

static mr_err_t drv_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    if (config->baud_rate == 0)
    {
        if (uart_data->instance == USART1)
        {
            RCC_APB2PeriphClockCmd(uart_data->uart_periph_clock, DISABLE);
        } else
        {
            RCC_APB1PeriphClockCmd(uart_data->uart_periph_clock, DISABLE);
        }
        return MR_ERR_OK;
    }

    if (uart_data->instance == USART1)
    {
        RCC_APB2PeriphClockCmd(uart_data->uart_periph_clock, ENABLE);
    } else
    {
        RCC_APB1PeriphClockCmd(uart_data->uart_periph_clock, ENABLE);
    }
    RCC_APB2PeriphClockCmd(uart_data->gpio_periph_clock, ENABLE);

    switch (config->data_bits)
    {
        case MR_SERIAL_DATA_BITS_8:
        {
            USART_InitStructure.USART_WordLength = USART_WordLength_8b;
            break;
        }

        case MR_SERIAL_DATA_BITS_9:
        {
            USART_InitStructure.USART_WordLength = USART_WordLength_9b;
            break;
        }

        default:
            return MR_ERR_INVALID;
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
            return MR_ERR_INVALID;
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
            return MR_ERR_INVALID;
    }

    switch (config->bit_order)
    {
        case MR_SERIAL_BIT_ORDER_LSB:
        {
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->invert)
    {
        case MR_SERIAL_NRZ_NORMAL:
        {
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    GPIO_InitStructure.GPIO_Pin = uart_data->tx_gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(uart_data->tx_gpio_port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = uart_data->rx_gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_data->rx_gpio_port, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = uart_data->irqno;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(uart_data->instance, USART_IT_RXNE, ENABLE);

    USART_InitStructure.USART_BaudRate = config->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = 0;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart_data->instance, &USART_InitStructure);
    USART_Cmd(uart_data->instance, ENABLE);

    return MR_ERR_OK;
}

static void drv_serial_write(mr_serial_t serial, mr_uint8_t data)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    mr_size_t i = 0;

    while (USART_GetFlagStatus(uart_data->instance, USART_FLAG_TC) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return;
        }
    }
    uart_data->instance->DATAR = data;
}

static mr_uint8_t drv_serial_read(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    mr_size_t i = 0;

    while (USART_GetFlagStatus(uart_data->instance, USART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }
    return uart_data->instance->DATAR & 0xff;
}

static void drv_serial_start_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    USART_ITConfig(uart_data->instance, USART_IT_TXE, ENABLE);
}

static void drv_serial_stop_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    USART_ITConfig(uart_data->instance, USART_IT_TXE, DISABLE);
}

static void drv_serial_isr(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    if (USART_GetITStatus(uart_data->instance, USART_IT_RXNE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_RX_INT);
        USART_ClearITPendingBit(uart_data->instance, USART_IT_RXNE);
    }

    if (USART_GetITStatus(uart_data->instance, USART_IT_TXE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_TX_INT);
        USART_ClearITPendingBit(uart_data->instance, USART_IT_TXE);
    }
}

#ifdef MR_BSP_UART_1
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_1_INDEX]);
}
#endif

#ifdef MR_BSP_UART_2
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_2_INDEX]);
}
#endif

#ifdef MR_BSP_UART_3
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_3_INDEX]);
}
#endif

#ifdef MR_BSP_UART_4
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_4_INDEX]);
}
#endif

#ifdef MR_BSP_UART_5
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_5_INDEX]);
}
#endif

#ifdef MR_BSP_UART_6
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_6_INDEX]);
}
#endif

#ifdef MR_BSP_UART_7
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_7_INDEX]);
}
#endif

#ifdef MR_BSP_UART_8
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART8_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_8_INDEX]);
}
#endif

mr_err_t drv_uart_init(void)
{
    static struct mr_serial_ops drv_ops =
        {
            drv_serial_configure,
            drv_serial_write,
            drv_serial_read,
            drv_serial_start_tx,
            drv_serial_stop_tx,
        };
    mr_size_t count = mr_array_num(serial_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_serial_device_add(&serial_device[count], drv_uart_data[count].name, &drv_ops, &drv_uart_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_uart_init);

#endif
