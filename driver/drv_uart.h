/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "device/serial.h"
#include "mrboard.h"

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

/**
 * @struct Driver uart data
 */
struct drv_uart_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_UART_H_ */