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

static struct drv_uart_data drv_uart_data[] =
    {
#ifdef MR_BSP_UART_1
        {"uart1", /* ... */},
#endif
#ifdef MR_BSP_UART_2
        {"uart2", /* ... */},
#endif
#ifdef MR_BSP_UART_3
        {"uart3", /* ... */},
#endif
        /* ... */
    };

static struct mr_serial serial_device[mr_array_num(drv_uart_data)];

static mr_err_t drv_serial_configure(mr_serial_t serial, struct mr_serial_config *config)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    /* ... */

    return MR_ERR_OK;
}

static void drv_serial_write(mr_serial_t serial, mr_uint8_t data)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    /* ... */
}

static mr_uint8_t drv_serial_read(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;
    mr_uint8_t data = 0;

    /* ... */

    return data;
}

static void drv_serial_start_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    /* ... */
}

static void drv_serial_stop_tx(mr_serial_t serial)
{
    struct drv_uart_data *uart_data = (struct drv_uart_data *)serial->device.data;

    /* ... */
}

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