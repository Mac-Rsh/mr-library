/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_DAC_H_
#define _MR_DAC_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_DAC

/**
 * @addtogroup DAC
 * @{
 */

/**
 * @brief DAC configuration structure.
 */
struct mr_dac_config
{
    int state;                                                      /**< Channel state */
};

/**
 * @brief DAC control command.
 */
#define MR_IOC_DAC_SET_CHANNEL          MR_IOC_SPOS                 /**< Set channel command */
#define MR_IOC_DAC_SET_CHANNEL_CONFIG   MR_IOC_SCFG                 /**< Set channel configuration command */

#define MR_IOC_DAC_GET_CHANNEL          MR_IOC_GPOS                 /**< Get channel command */
#define MR_IOC_DAC_GET_CHANNEL_CONFIG   MR_IOC_GCFG                 /**< Get channel configuration command */

/**
 * @brief DAC data type.
 */
typedef uint32_t mr_dac_data_t;                                     /**< DAC write data type */

/**
 * @brief DAC structure.
 */
struct mr_dac
{
    struct mr_dev dev;                                              /**< Device */

    uint32_t channels;                                              /**< Channels */
};

/**
 * @brief DAC operations structure.
 */
struct mr_dac_ops
{
    int (*configure)(struct mr_dac *dac, int state);
    int (*channel_configure)(struct mr_dac *dac, int channel, int state);
    int (*write)(struct mr_dac *dac, int channel, uint32_t data);
};

int mr_dac_register(struct mr_dac *dac, const char *path, struct mr_drv *drv);
/** @} */

#endif /* MR_USING_DAC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DAC_H_ */
