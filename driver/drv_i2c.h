/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#include "device/i2c.h"
#include "mrboard.h"

#if (MR_CFG_I2C == MR_CFG_ENABLE)

/**
 * @struct Driver soft i2c bus data
 */
struct drv_soft_i2c_bus_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_I2C_H_ */
