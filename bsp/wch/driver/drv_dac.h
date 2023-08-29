/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 */

#ifndef _DRV_DAC_H_
#define _DRV_DAC_H_

#include "device/dac/dac.h"

#if (MR_CFG_DAC == MR_CFG_ENABLE)

/**
 * @struct ch32 DAC data
 */
struct ch32_dac_data
{
    const char *name;

    mr_uint32_t channel;
    mr_uint32_t periph_clock;
};

#endif

#endif /* _DRV_DAC_H_ */