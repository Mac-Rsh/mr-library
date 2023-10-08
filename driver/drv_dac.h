/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-08     MacRsh       first version
 */

#ifndef _DRV_DAC_H_
#define _DRV_DAC_H_

#include "device/dac.h"
#include "mrboard.h"

#if (MR_CFG_DAC == MR_CFG_ENABLE)

/**
 * @struct Driver dac data
 */
struct drv_dac_data
{
    const char *name;

    /* ... */
};

#endif

#endif /* _DRV_DAC_H_ */
