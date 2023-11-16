/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_DAC_H_
#define _MR_DAC_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_DAC

/**
 * @brief DAC channel state.
 */
#define MR_DAC_STATE_DISABLE            MR_DISABLE                  /**< DAC disabled */
#define MR_DAC_STATE_ENABLE             MR_ENABLE                   /**< DAC enabled */

/**
 * @brief DAC channel state command.
 */
#define MR_CTRL_DAC_SET_CHANNEL_STATE   ((0x01|0x80) << 16)         /**< Set channel state */
#define MR_CTRL_DAC_GET_CHANNEL_STATE   ((0x01|0x00) << 16)         /**< Get channel state */

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

    uint32_t channel;                                               /**< Channel */
};

/**
 * @brief DAC operations structure.
 */
struct mr_dac_ops
{
    int (*configure)(struct mr_dac *dac, int state);
    int (*channel_configure)(struct mr_dac *dac, int channel, int state);
    void (*write)(struct mr_dac *dac, int channel, uint32_t data);
};

/**
 * @addtogroup DAC.
 * @{
 */
int mr_dac_register(struct mr_dac *dac, const char *name, struct mr_drv *drv);
/** @} */

#endif /* MR_USING_DAC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DAC_H_ */
