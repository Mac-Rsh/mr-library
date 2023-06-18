/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     MacRsh       first version
 */

#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include "device/pin/pin.h"

#if (MR_CONF_PIN == MR_CONF_ENABLE)

mr_err_t ch32_gpio_init(void);

#endif /* MR_CONF_PIN */

#endif /* _DRV_GPIO_H_ */