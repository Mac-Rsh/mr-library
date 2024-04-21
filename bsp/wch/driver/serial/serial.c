/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#include "../mr-library/driver/include/mr_serial.h"

#ifdef MR_USE_SERIAL

#if !defined(MR_USE_UART1) && !defined(MR_USE_UART2) &&                        \
    !defined(MR_USE_UART3) && !defined(MR_USE_UART4) &&                        \
    !defined(MR_USE_UART5) && !defined(MR_USE_UART6) &&                        \
    !defined(MR_USE_UART7) && !defined(MR_USE_UART8)
#warning "Please enable at least one Serial driver"
#endif /* !defined(MR_USE_UART1) && !defined(MR_USE_UART2) &&                  \
        * !defined(MR_USE_UART3) && !defined(MR_USE_UART4) &&                  \
        * !defined(MR_USE_UART5) && !defined(MR_USE_UART6) &&                  \
        * !defined(MR_USE_UART7) && !defined(MR_USE_UART8) */

enum _serial_driver_index
{
#ifdef MR_USE_UART1
    _SERIAL_DRIVER_INDEX_UART1,
#endif /* MR_USE_UART1 */
#ifdef MR_USE_UART2
    _SERIAL_DRIVER_INDEX_UART2,
#endif /* MR_USE_UART2 */
#ifdef MR_USE_UART3
    _SERIAL_DRIVER_INDEX_UART3,
#endif /* MR_USE_UART3 */
#ifdef MR_USE_UART4
    _SERIAL_DRIVER_INDEX_UART4,
#endif /* MR_USE_UART4 */
#ifdef MR_USE_UART5
    _SERIAL_DRIVER_INDEX_UART5,
#endif /* MR_USE_UART5 */
#ifdef MR_USE_UART6
    _SERIAL_DRIVER_INDEX_UART6,
#endif /* MR_USE_UART6 */
#ifdef MR_USE_UART7
    _SERIAL_DRIVER_INDEX_UART7,
#endif /* MR_USE_UART7 */
#ifdef MR_USE_UART8
    _SERIAL_DRIVER_INDEX_UART8,
#endif /* MR_USE_UART8 */
    _SERIAL_DRIVER_INDEX_UART_MAX
};

static struct mr_serial_driver _serial_driver[] = {
#ifdef MR_USE_UART1
    _DRIVER_UART1_CONFIG,
#endif /* MR_USE_UART1 */
#ifdef MR_USE_UART2
    _DRIVER_UART2_CONFIG,
#endif /* MR_USE_UART2 */
#ifdef MR_USE_UART3
    _DRIVER_UART3_CONFIG,
#endif /* MR_USE_UART3 */
#ifdef MR_USE_UART4
    _DRIVER_UART4_CONFIG,
#endif /* MR_USE_UART4 */
#ifdef MR_USE_UART5
    _DRIVER_UART5_CONFIG,
#endif /* MR_USE_UART5 */
#ifdef MR_USE_UART6
    _DRIVER_UART6_CONFIG,
#endif /* MR_USE_UART6 */
#ifdef MR_USE_UART7
    _DRIVER_UART7_CONFIG,
#endif /* MR_USE_UART7 */
#ifdef MR_USE_UART8
    _DRIVER_UART8_CONFIG,
#endif /* MR_USE_UART8 */
};

static const char *_serial_device_path[] = {
#ifdef MR_USE_UART1
    "serial1",
#endif /* MR_USE_UART1 */
#ifdef MR_USE_UART2
    "serial2",
#endif /* MR_USE_UART2 */
#ifdef MR_USE_UART3
    "serial3",
#endif /* MR_USE_UART3 */
#ifdef MR_USE_UART4
    "serial4",
#endif /* MR_USE_UART4 */
#ifdef MR_USE_UART5
    "serial5",
#endif /* MR_USE_UART5 */
#ifdef MR_USE_UART6
    "serial6",
#endif /* MR_USE_UART6 */
#ifdef MR_USE_UART7
    "serial7",
#endif /* MR_USE_UART7 */
#ifdef MR_USE_UART8
    "serial8",
#endif /* MR_USE_UART8 */
};

static struct mr_serial _serial_device[MR_ARRAY_NUM(_serial_device_path)];

static int serial_driver_configure(struct mr_driver *driver, bool enable,
                                   struct mr_serial_config *config)
{
    struct mr_serial_driver *serial = (struct mr_serial_driver *)driver;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    /* Configure clock */
    RCC_APB2PeriphClockCmd(serial->gpio_clock, ENABLE);
    if ((uint32_t)serial->instance < APB2PERIPH_BASE)
    {
        RCC_APB1PeriphClockCmd(serial->clock, enable);
    } else
    {
        RCC_APB2PeriphClockCmd(serial->clock, enable);
    }

    /* Configure remap */
    if (serial->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(serial->remap, enable);
    }
#if defined(MR_USE_CH32V30X) && (MR_USE_UART1_GROUP >= 3)
    if (serial->instance == USART1)
    {
        GPIO_PinRemapConfig(GPIO_Remap_USART1_HighBit, enable);
    }
#endif /* defined(MR_USE_CH32V30X) && (MR_USE_UART1_GROUP >= 3) */

    if (enable == true)
    {
        /* Configure baud rate */
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

        /* Configure stop bits */
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

        /* Configure parity */
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

        /* Configure bits order */
        switch (config->bits_order)
        {
            case MR_SERIAL_BITS_ORDER_LSB:
            {
                break;
            }
            default:
            {
                return MR_EINVAL;
            }
        }

        /* Configure polarity */
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
        GPIO_InitStructure.GPIO_Pin = serial->tx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(serial->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = serial->rx_pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(serial->rx_port, &GPIO_InitStructure);
    } else
    {
        /* Reset TX/RX GPIO */
        GPIO_InitStructure.GPIO_Pin = serial->tx_pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(serial->tx_port, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = serial->rx_pin;
        GPIO_Init(serial->rx_port, &GPIO_InitStructure);
    }

    /* Configure UART */
    USART_InitStructure.USART_BaudRate = config->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = 0;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(serial->instance, &USART_InitStructure);
    USART_Cmd(serial->instance, enable);
    USART_ClearFlag(serial->instance, USART_FLAG_RXNE);
    USART_ClearFlag(serial->instance, USART_FLAG_TC);

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = serial->irq;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enable;
    NVIC_Init(&NVIC_InitStructure);
    USART_ClearITPendingBit(serial->instance, USART_IT_RXNE);
    USART_ITConfig(serial->instance, USART_IT_RXNE, enable);
    if (enable == false)
    {
        USART_ITConfig(serial->instance, USART_IT_TXE, DISABLE);
    }
    return MR_EOK;
}

static int serial_driver_receive(struct mr_driver *driver, uint8_t *data)
{
    struct mr_serial_driver *serial = (struct mr_serial_driver *)driver;
    size_t i = 0;

    /* Read data from register */
    while (USART_GetFlagStatus(serial->instance, USART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    *data = (uint8_t)USART_ReceiveData(serial->instance);
    return MR_EOK;
}

static int serial_driver_send(struct mr_driver *driver, uint8_t data)
{
    struct mr_serial_driver *serial = (struct mr_serial_driver *)driver;
    size_t i = 0;

    /* Write data to register */
    USART_SendData(serial->instance, data);
    while (USART_GetFlagStatus(serial->instance, USART_FLAG_TC) == RESET)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return MR_ETIMEOUT;
        }
    }
    return MR_EOK;
}

static int serial_driver_send_int_configure(struct mr_driver *driver,
                                            bool enable)
{
    struct mr_serial_driver *serial = (struct mr_serial_driver *)driver;

    /* Configure TX interrupt */
    USART_ClearITPendingBit(serial->instance, USART_IT_TXE);
    USART_ITConfig(serial->instance, USART_IT_TXE, enable);
    return MR_EOK;
}

MR_INLINE void serial_device_isr(struct mr_serial *serial)
{
    struct mr_device *device = (struct mr_device *)serial;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_serial_driver *_serial = (struct mr_serial_driver *)driver;

    if (USART_GetITStatus(_serial->instance, USART_IT_RXNE) != RESET)
    {
        mr_device_isr(device, MR_EVENT_SERIAL_RD_COMPLETE_INT, NULL);
        USART_ClearITPendingBit(_serial->instance, USART_IT_RXNE);
    }

    if (USART_GetITStatus(_serial->instance, USART_IT_TXE) != RESET)
    {
        mr_device_isr(device, MR_EVENT_SERIAL_WR_COMPLETE_INT, NULL);
        USART_ClearITPendingBit(_serial->instance, USART_IT_TXE);
    }
}

#ifdef MR_USE_UART1
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART1]);
}
#endif /* MR_USE_UART1 */

#ifdef MR_USE_UART2
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART2]);
}
#endif /* MR_USE_UART2 */

#ifdef MR_USE_UART3
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART3]);
}
#endif /* MR_USE_UART3 */

#ifdef MR_USE_UART4
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART4]);
}
#endif /* MR_USE_UART4 */

#ifdef MR_USE_UART5
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART5]);
}
#endif /* MR_USE_UART5 */

#ifdef MR_USE_UART6
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART6]);
}
#endif /* MR_USE_UART6 */

#ifdef MR_USE_UART7
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART7]);
}
#endif /* MR_USE_UART7 */

#ifdef MR_USE_UART8
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART8_IRQHandler(void)
{
    serial_device_isr(&_serial_device[_SERIAL_DRIVER_INDEX_UART8]);
}
#endif /* MR_USE_UART8 */

static void serial_driver_init(void)
{
    static struct mr_serial_ops ops = {
        serial_driver_configure, serial_driver_receive, serial_driver_send,
        serial_driver_send_int_configure};

    for (size_t i = 0; i < MR_ARRAY_NUM(_serial_device); i++)
    {
        _serial_driver[i].driver.ops = &ops;
        mr_serial_register(&_serial_device[i], _serial_device_path[i],
                           (struct mr_driver *)&_serial_driver[i]);
    }
}
MR_INIT_DRIVER_EXPORT(serial_driver_init);

#endif /* MR_USE_SERIAL */
