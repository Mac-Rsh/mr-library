/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
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
        {"uart1", {0}, USART1, USART1_IRQn},
#endif
#ifdef MR_BSP_UART_2
        {"uart2", {0}, USART2, USART2_IRQn},
#endif
#ifdef MR_BSP_UART_3
        {"uart3", {0}, USART3, USART3_IRQn},
#endif
#ifdef MR_BSP_UART_4
        {"uart4", {0}, UART4, UART4_IRQn},
#endif
#ifdef MR_BSP_UART_5
        {"uart5", {0}, UART5, UART5_IRQn},
#endif
#ifdef MR_BSP_UART_6
        {"uart6", {0}, USART6, USART6_IRQn},
#endif
#ifdef MR_BSP_UART_7
        {"uart7", {0}, UART7, UART7_IRQn},
#endif
#ifdef MR_BSP_UART_8
        {"uart8", {0}, UART8, UART8_IRQn},
#endif
    };

static struct mr_serial serial_device[mr_array_num(drv_uart_data)];

static mr_err_t drv_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    uart_data->handle.Instance = uart_data->instance;

    switch (config->data_bits)
    {
        case MR_SERIAL_DATA_BITS_8:
        {
            uart_data->handle.Init.WordLength = UART_WORDLENGTH_8B;
            break;
        }

        case MR_SERIAL_DATA_BITS_9:
        {
            uart_data->handle.Init.WordLength = UART_WORDLENGTH_9B;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->stop_bits)
    {
        case MR_SERIAL_STOP_BITS_1:
        {
            uart_data->handle.Init.StopBits = UART_STOPBITS_1;
            break;
        }

        case MR_SERIAL_STOP_BITS_2:
        {
            uart_data->handle.Init.StopBits = UART_STOPBITS_2;
            break;
        }

        default:
            return MR_ERR_INVALID;
    }

    switch (config->parity)
    {
        case MR_SERIAL_PARITY_NONE:
        {
            uart_data->handle.Init.Parity = UART_PARITY_NONE;
            break;
        }

        case MR_SERIAL_PARITY_ODD:
        {
            uart_data->handle.Init.Parity = UART_PARITY_ODD;
            break;
        }

        case MR_SERIAL_PARITY_EVEN:
        {
            uart_data->handle.Init.Parity = UART_PARITY_EVEN;
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

    uart_data->handle.Init.BaudRate = config->baud_rate;
    uart_data->handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_data->handle.Init.Mode = UART_MODE_TX_RX;
    uart_data->handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&uart_data->handle) != HAL_OK)
    {
        return MR_ERR_GENERIC;
    }

    HAL_NVIC_SetPriority(uart_data->irq_type, 1, 0);
    HAL_NVIC_EnableIRQ(uart_data->irq_type);
    __HAL_UART_ENABLE_IT(&uart_data->handle, UART_IT_RXNE);

    return MR_ERR_OK;
}

static void drv_serial_write(mr_serial_t serial, mr_uint8_t data)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    mr_size_t i = 0;

    while (__HAL_UART_GET_FLAG(&uart_data->handle, UART_FLAG_TC) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return;
        }
    }
    uart_data->instance->DR = data;
}

static mr_uint8_t drv_serial_read(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    mr_size_t i = 0;

    while (__HAL_UART_GET_FLAG(&uart_data->handle, UART_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > MR_UINT16_MAX)
        {
            return 0;
        }
    }
    return uart_data->instance->DR & 0xff;
}

static void drv_serial_start_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    __HAL_UART_ENABLE_IT(&uart_data->handle, UART_IT_TXE);
}

static void drv_serial_stop_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    __HAL_UART_DISABLE_IT(&uart_data->handle, UART_IT_TXE);
}

static void drv_serial_isr(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    if (__HAL_UART_GET_FLAG(&uart_data->handle, UART_FLAG_RXNE) != RESET &&
        __HAL_UART_GET_IT_SOURCE(&uart_data->handle, UART_IT_RXNE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_RX_INT);
    }

    if (__HAL_UART_GET_FLAG(&uart_data->handle, UART_FLAG_TXE) != RESET &&
        __HAL_UART_GET_IT_SOURCE(&uart_data->handle, UART_IT_TXE) != RESET)
    {
        mr_serial_device_isr(serial, MR_SERIAL_EVENT_TX_INT);
    }
}

#ifdef MR_BSP_UART_1
void USART1_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_1_INDEX]);
}
#endif

#ifdef MR_BSP_UART_2
void USART2_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_2_INDEX]);
}
#endif

#ifdef MR_BSP_UART_3
void USART3_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_3_INDEX]);
}
#endif

#ifdef MR_BSP_UART_4
void UART4_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_4_INDEX]);
}
#endif

#ifdef MR_BSP_UART_5
void UART5_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_5_INDEX]);
}
#endif

#ifdef MR_BSP_UART_6
void USART6_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_6_INDEX]);
}
#endif

#ifdef MR_BSP_UART_7
void UART7_IRQHandler(void)
{
    drv_serial_isr(&serial_device[DRV_UART_7_INDEX]);
}
#endif

#ifdef MR_BSP_UART_8
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
