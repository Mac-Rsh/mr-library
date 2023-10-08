/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#ifndef _DRV_SPI_H_
#define _DRV_SPI_H_

#include "device/spi.h"
#include "mrboard.h"

#if (MR_CFG_SPI == MR_CFG_ENABLE)

/**
 * @struct Driver spi bus data
 */
struct drv_spi_bus_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_SPI_H_ */
