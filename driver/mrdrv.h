/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _MR_DRV_H_
#define _MR_DRV_H_

#include <mrconfig.h>

#if (MR_CONF_DEVICE_SERIAL == MR_CONF_ENABLE)
#include <drv_uart.h>
#endif

#if (MR_CONF_DEVICE_PIN == MR_CONF_ENABLE)
#include <drv_gpio.h>
#endif

#endif
