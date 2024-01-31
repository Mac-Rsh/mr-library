/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#ifndef _MR_ADC_H_
#define _MR_ADC_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ADC

/**
 * @addtogroup ADC
 * @{
 */

/**
 * @brief ADC configuration structure.
 */
struct mr_adc_config
{
    int state;                                                      /**< Channel state */
};

/**
 * @brief ADC control command.
 */
#define MR_IOC_ADC_SET_CHANNEL          MR_IOC_SPOS                 /**< Set channel command */
#define MR_IOC_ADC_SET_CHANNEL_CONFIG   MR_IOC_SCFG                 /**< Set channel configuration command */

#define MR_IOC_ADC_GET_CHANNEL          MR_IOC_GPOS                 /**< Get channel command */
#define MR_IOC_ADC_GET_CHANNEL_CONFIG   MR_IOC_GCFG                 /**< Get channel configuration command */

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

int mr_adc_register(struct mr_adc *adc, const char *path, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_ADC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_ADC_H_ */
