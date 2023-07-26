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

#define MR_DAC_CHANNEL_DISABLE          MR_DISABLE
#define MR_DAC_CHANNEL_ENABLE           MR_ENABLE

struct mr_dac_config
{
    union
    {
        struct
        {
            mr_pos_t channel0: 1;
            mr_pos_t channel1: 1;
            mr_pos_t channel2: 1;
            mr_pos_t channel3: 1;
            mr_pos_t channel4: 1;
            mr_pos_t channel5: 1;
            mr_pos_t channel6: 1;
            mr_pos_t channel7: 1;
            mr_pos_t channel8: 1;
            mr_pos_t channel9: 1;
            mr_pos_t channel10: 1;
            mr_pos_t channel11: 1;
            mr_pos_t channel12: 1;
            mr_pos_t channel13: 1;
            mr_pos_t channel14: 1;
            mr_pos_t channel15: 1;
            mr_pos_t channel16: 1;
            mr_pos_t channel17: 1;
            mr_pos_t channel18: 1;
            mr_pos_t channel19: 1;
            mr_pos_t channel20: 1;
            mr_pos_t channel21: 1;
            mr_pos_t channel22: 1;
            mr_pos_t channel23: 1;
            mr_pos_t channel24: 1;
            mr_pos_t channel25: 1;
            mr_pos_t channel26: 1;
            mr_pos_t channel27: 1;
            mr_pos_t channel28: 1;
            mr_pos_t channel29: 1;
            mr_pos_t channel30: 1;
            mr_pos_t channel31: 1;
        };
        mr_pos_t _channel_mask;
    };
};

typedef struct mr_dac *mr_dac_t;

struct mr_dac_ops
{
    mr_err_t (*configure)(mr_dac_t dac, mr_state_t state);
    mr_err_t (*channel_configure)(mr_dac_t dac, struct mr_dac_config *config);
    void (*write)(mr_dac_t dac, mr_pos_t channel, mr_uint32_t value);
};

struct mr_dac
{
    struct mr_device device;

    struct mr_dac_config config;

    const struct mr_dac_ops *ops;
};

mr_err_t mr_dac_device_add(mr_dac_t dac, const char *name, void *data, struct mr_dac_ops *ops);

#endif  /* MR_CONF_DAC */

#endif  /* _DAC_H_ */