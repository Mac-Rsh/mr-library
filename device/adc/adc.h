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

#if (MR_CFG_ADC == MR_CFG_ENABLE)

/**
 * @struct ADC device config
 */
struct mr_adc_config
{
    struct mr_device_channel channel;
};
typedef struct mr_adc_config *mr_adc_config_t;

typedef struct mr_adc *mr_adc_t;

/**
 * @struct ADC device operations
 */
struct mr_adc_ops
{
    mr_err_t (*configure)(mr_adc_t adc, mr_state_t state);
    mr_err_t (*channel_configure)(mr_adc_t adc, mr_adc_config_t config);
    mr_uint32_t (*read)(mr_adc_t adc, mr_pos_t channel);
};

/**
 * @struct ADC device
 */
struct mr_adc
{
    struct mr_device device;

    struct mr_adc_config config;

    const struct mr_adc_ops *ops;
};

/**
 * @addtogroup ADC device
 * @{
 */
mr_err_t mr_adc_device_add(mr_adc_t adc, const char *name, struct mr_adc_ops *ops, void *data);
/** @} */

#endif

#endif /* _ADC_H_ */