/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _DAC_H_
#define _DAC_H_

#include "mrlib.h"

#if (MR_CONF_DAC == MR_CONF_ENABLE)

#define MR_DAC_STATE_DISABLE            0
#define MR_DAC_STATE_ENABLE             1

struct mr_dac_config
{
    mr_uint16_t channel;
    mr_uint8_t state;
};

typedef struct mr_dac *mr_dac_t;

struct mr_dac_ops
{
    mr_err_t (*configure)(mr_dac_t dac, mr_uint8_t state);
    mr_err_t (*channel_configure)(mr_dac_t dac, struct mr_dac_config *config);
    void (*write)(mr_dac_t dac, mr_uint16_t channel, mr_uint32_t value);
};

struct mr_dac
{
    struct mr_device device;

    const struct mr_dac_ops *ops;
};

mr_err_t mr_dac_device_add(mr_dac_t dac, const char *name, struct mr_dac_ops *ops, void *data);

#endif

#endif