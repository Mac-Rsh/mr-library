/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#ifndef _MR_ADC_H_
#define _MR_ADC_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ADC

/**
 * @brief ADC channel state.
 */
#define MR_ADC_STATE_DISABLE            MR_DISABLE                  /**< ADC disabled */
#define MR_ADC_STATE_ENABLE             MR_ENABLE                   /**< ADC enabled */

/**
 * @brief ADC channel state command.
 */
#define MR_CTRL_ADC_SET_CHANNEL_STATE   ((0x01|0x80) << 16)         /**< Set channel state */
#define MR_CTRL_ADC_GET_CHANNEL_STATE   ((0x01|0x00) << 16)         /**< Get channel state */

/**
 * @brief ADC data type.
 */
typedef uint32_t mr_adc_data_t;                                     /**< ADC read data type */

/**
 * @brief ADC structure.
 */
struct mr_adc
{
    struct mr_dev dev;                                              /**< Device */

    uint32_t channel;                                               /**< Channel */
};

/**
 * @brief ADC operations structure.
 */
struct mr_adc_ops
{
    int (*configure)(struct mr_adc *adc, int state);
    int (*channel_configure)(struct mr_adc *adc, int channel, int state);
    uint32_t (*read)(struct mr_adc *adc, int channel);
};

/**
 * @addtogroup ADC.
 * @{
 */
int mr_adc_register(struct mr_adc *adc, const char *name, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_ADC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_ADC_H_ */
