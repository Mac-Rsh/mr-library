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

#if (MR_CONF_DAC == MR_CONF_ENABLE)

struct ch32_dac_info
{
    mr_uint32_t dac_channel;
    mr_uint32_t dac_periph_clock;
};

struct ch32_dac
{
    char *name;

    struct ch32_dac_info info;
};

mr_err_t ch32_dac_init(void);

#endif

#endif