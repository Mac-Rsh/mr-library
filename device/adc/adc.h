/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _ADC_H_
#define _ADC_H_

#include "mrlib.h"

#if (MR_CONF_ADC == MR_CONF_ENABLE)

#define MR_ADC_CHANNEL_DISABLE          MR_DISABLE
#define MR_ADC_CHANNEL_ENABLE           MR_ENABLE

struct mr_adc_config
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

typedef struct mr_adc *mr_adc_t;

struct mr_adc_ops
{
    mr_err_t (*configure)(mr_adc_t adc, mr_state_t state);
    mr_err_t (*channel_configure)(mr_adc_t adc, struct mr_adc_config *config);
    mr_uint32_t (*read)(mr_adc_t adc, mr_pos_t channel);
};

struct mr_adc
{
    struct mr_device device;

    struct mr_adc_config config;

    const struct mr_adc_ops *ops;
};

mr_err_t mr_adc_device_add(mr_adc_t adc, const char *name, void *data, struct mr_adc_ops *ops);

#endif  /* MR_CONF_ADC */

#endif  /* _ADC_H_ */