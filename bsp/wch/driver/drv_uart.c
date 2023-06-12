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

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)

enum
{
#ifdef BSP_UART_1
    UART1_INDEX,
#endif
#ifdef BSP_UART_2
    UART2_INDEX,
#endif
#ifdef BSP_UART_3
    UART3_INDEX,
#endif
#ifdef BSP_UART_4
    UART4_INDEX,
#endif
#ifdef BSP_UART_5
    UART5_INDEX,
#endif
#ifdef BSP_UART_6
    UART6_INDEX,
#endif
#ifdef BSP_UART_7
    UART7_INDEX,
#endif
#ifdef BSP_UART_8
    UART8_INDEX,
#endif
};

static struct ch32_uart ch32_uart[] =
        {
#ifdef BSP_UART_1
                {"uart1",
                 {USART1,
                  RCC_APB2Periph_USART1,
                  RCC_APB2Periph_GPIOA,
                  GPIOA,
                  GPIO_Pin_9,
                  GPIOA,
                  GPIO_Pin_10,
                  CH32_UART_GPIO_REMAP_NONE,
                  USART1_IRQn}},
#endif
#ifdef BSP_UART_2
                {"uart2",
                 {USART2,
                  RCC_APB1Periph_USART2,
                  RCC_APB2Periph_GPIOA,
                  GPIOA,
                  GPIO_Pin_2,
                  GPIOA,
                  GPIO_Pin_3,
                  CH32_UART_GPIO_REMAP_NONE,
                  USART2_IRQn}},
#endif
#ifdef BSP_UART_3
                {"uart3",
                 {USART3,
                  RCC_APB1Periph_USART2,
                  RCC_APB2Periph_GPIOB,
                  GPIOB,
                  GPIO_Pin_10,
                  GPIOB,
                  GPIO_Pin_11,
                  CH32_UART_GPIO_REMAP_NONE,
                  USART3_IRQn}},
#endif
#ifdef BSP_UART_4
                {"uart4",
                 {UART4,
                  RCC_APB1Periph_UART4,
                  RCC_APB2Periph_GPIOC,
                  GPIOC,
                  GPIO_Pin_10,
                  GPIOC,
                  GPIO_Pin_11,
                  CH32_UART_GPIO_REMAP_NONE,
                  UART4_IRQn}},
#endif
#ifdef BSP_UART_5
                {"uart5",
                 {UART5,
                  RCC_APB1Periph_UART5,
                  RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
                  GPIOC,
                  GPIO_Pin_12,
                  GPIOD,
                  GPIO_Pin_2,
                  CH32_UART_GPIO_REMAP_NONE,
                  UART5_IRQn}},
#endif
#ifdef BSP_UART_6
                {"uart6",
                 {UART6,
                  RCC_APB1Periph_UART6,
                  RCC_APB2Periph_GPIOC,
                  GPIOC,
                  GPIO_Pin_0,
                  GPIOC,
                  GPIO_Pin_1,
                  CH32_UART_GPIO_REMAP_NONE,
                  UART6_IRQn}},
#endif
#ifdef BSP_UART_7
                {"uart7",
                 {UART7,
                  RCC_APB1Periph_UART7,
                  RCC_APB2Periph_GPIOC,
                  GPIOC,
                  GPIO_Pin_2,
                  GPIOC,
                  GPIO_Pin_3,
                  CH32_UART_GPIO_REMAP_NONE,
                  UART7_IRQn}},
#endif
#ifdef BSP_UART_8
                {"uart8",
                 {UART8,
                  RCC_APB1Periph_UART8,
                  RCC_APB2Periph_GPIOC,
                  GPIOC,
                  GPIO_Pin_4,
                  GPIOC,
                  GPIO_Pin_5,
                  CH32_UART_GPIO_REMAP_NONE,
                  UART8_IRQn}},
#endif
        };

static struct mr_serial serial_device[mr_array_get_length(ch32_uart)];

static mr_err_t ch32_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    if (config->baud_rate == 0)
    {
        if (driver->info.Instance == USART1)
        {
            RCC_APB2PeriphClockCmd(driver->info.uart_periph_clock, DISABLE);
        }
        else
        {
            RCC_APB1PeriphClockCmd(driver->info.uart_periph_clock, DISABLE);
        }
        RCC_APB2PeriphClockCmd(driver->info.gpio_periph_clock, DISABLE);

        return MR_ERR_OK;
    }

    if (driver->info.Instance == USART1)
    {
        RCC_APB2PeriphClockCmd(driver->info.uart_periph_clock, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(driver->info.uart_periph_clock, ENABLE);
    }
    RCC_APB2PeriphClockCmd(driver->info.gpio_periph_clock, ENABLE);

    switch (config->data_bits)
    {
        case MR_SERIAL_DATA_BITS_8:
            USART_InitStructure.USART_WordLength = USART_WordLength_8b;
            break;
        case MR_SERIAL_DATA_BITS_9:
            USART_InitStructure.USART_WordLength = USART_WordLength_9b;
            break;
        default:
            USART_InitStructure.USART_WordLength = USART_WordLength_8b;
            break;
    }

    switch (config->stop_bits)
    {
        case MR_SERIAL_STOP_BITS_1:
            USART_InitStructure.USART_StopBits = USART_StopBits_1;
            break;
        case MR_SERIAL_STOP_BITS_2:
            USART_InitStructure.USART_StopBits = USART_StopBits_2;
            break;
        default:
            USART_InitStructure.USART_StopBits = USART_StopBits_1;
            break;
    }

    switch (config->parity)
    {
        case MR_SERIAL_PARITY_NONE:
            USART_InitStructure.USART_Parity = USART_Parity_No;
            break;
        case MR_SERIAL_PARITY_ODD:
            USART_InitStructure.USART_Parity = USART_Parity_Odd;
            break;
        case MR_SERIAL_PARITY_EVEN:
            USART_InitStructure.USART_Parity = USART_Parity_Even;
            break;
        default:
            USART_InitStructure.USART_Parity = USART_Parity_No;
            break;
    }

    if (driver->info.remap != CH32_UART_GPIO_REMAP_NONE)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(driver->info.remap, ENABLE);
    }

    GPIO_InitStructure.GPIO_Pin = driver->info.tx_gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(driver->info.tx_gpio_port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = driver->info.rx_gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(driver->info.rx_gpio_port, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = driver->info.irqno;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(driver->info.Instance, USART_IT_RXNE, ENABLE);

    USART_InitStructure.USART_BaudRate = config->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = 0;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(driver->info.Instance, &USART_InitStructure);
    USART_Cmd(driver->info.Instance, ENABLE);

    return MR_ERR_OK;
}

static void ch32_serial_write(mr_serial_t serial, mr_uint8_t data)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;

    while (USART_GetFlagStatus(driver->info.Instance, USART_FLAG_TC) == RESET);
    driver->info.Instance->DATAR = data;
}

static mr_uint8_t ch32_serial_read(mr_serial_t serial)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;

    if (USART_GetFlagStatus(driver->info.Instance, USART_FLAG_RXNE) != RESET)
    {
        return driver->info.Instance->DATAR & 0xff;
    }

    return 0;
}

static void ch32_serial_start_tx(mr_serial_t serial)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;

    USART_ITConfig(driver->info.Instance, USART_IT_TXE, ENABLE);
}

static void ch32_serial_stop_tx(mr_serial_t serial)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;

    USART_ITConfig(driver->info.Instance, USART_IT_TXE, DISABLE);
}

static void ch32_serial_isr(mr_serial_t serial)
{
    struct ch32_uart *driver = (struct ch32_uart *)serial->device.data;

    if (USART_GetITStatus(driver->info.Instance, USART_IT_RXNE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_RX_INT);
        USART_ClearITPendingBit(driver->info.Instance, USART_IT_RXNE);
    }

    if (USART_GetITStatus(driver->info.Instance, USART_IT_TXE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_TX_INT);
        USART_ClearITPendingBit(driver->info.Instance, USART_IT_TXE);
    }
}

#ifdef BSP_UART_1
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USART1_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART1_INDEX]);
}

#endif

#ifdef BSP_UART_2
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USART2_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART2_INDEX]);
}

#endif

#ifdef BSP_UART_3
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USART3_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART3_INDEX]);
}

#endif

#ifdef BSP_UART_4
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UART4_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART4_INDEX]);
}

#endif

#ifdef BSP_UART_5
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UART5_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART5_INDEX]);
}

#endif

#ifdef BSP_UART_6
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UART6_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART6_INDEX]);
}

#endif

#ifdef BSP_UART_7
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UART7_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART7_INDEX]);
}

#endif

#ifdef BSP_UART_8
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UART8_IRQHandler(void)
{
    ch32_serial_isr(&serial_device[UART8_INDEX]);
}

#endif

mr_err_t ch32_uart_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(serial_device);
    static struct mr_serial_ops driver =
            {
                    ch32_serial_configure,
                    ch32_serial_write,
                    ch32_serial_read,
                    ch32_serial_start_tx,
                    ch32_serial_stop_tx,
            };

    while (count--)
    {
        ret = mr_serial_device_add(&serial_device[count], ch32_uart[count].name, &driver, &ch32_uart[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_uart_init);

#endif /* MR_CONF_SERIAL */