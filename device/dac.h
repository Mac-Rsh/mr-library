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

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_DAC == MR_CFG_ENABLE)

/**
 * @struct DAC device config
 */
struct mr_dac_config
{
    struct mr_device_channel channel;
};
typedef struct mr_dac_config *mr_dac_config_t;

typedef struct mr_dac *mr_dac_t;

/**
 * @struct DAC device operations
 */
struct mr_dac_ops
{
    mr_err_t (*configure)(mr_dac_t dac, mr_state_t state);
    mr_err_t (*channel_configure)(mr_dac_t dac, mr_dac_config_t config);
    void (*write)(mr_dac_t dac, mr_off_t channel, mr_uint32_t value);
};

/**
 * @struct DAC device
 */
struct mr_dac
{
    struct mr_device device;

    struct mr_dac_config config;

    const struct mr_dac_ops *ops;
};

/**
 * @addtogroup DAC device
 * @{
 */
mr_err_t mr_dac_device_add(mr_dac_t dac, const char *name, struct mr_dac_ops *ops, void *data);
/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* _DAC_H_ */
