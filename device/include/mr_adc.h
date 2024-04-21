/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#ifndef _MR_DEVICE_ADC_H_
#define _MR_DEVICE_ADC_H_

#include "../mr-library/include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_ADC

/**
 * @addtogroup ADC
 * @{
 */

#define MR_ADC_CHANNEL_STATE_ENABLE     true                /**< Channel enable */
#define MR_ADC_CHANNEL_STATE_DISABLE    false               /**< Channel disable */

#define MR_CMD_ADC_CHANNEL              MR_CMD_POS          /**< ADC channel command */
#define MR_CMD_ADC_CHANNEL_CONFIG       MR_CMD_CONFIG       /**< ADC channel configure command */
#define MR_CMD_ADC_RESOLUTION           (0x01)              /**< ADC resolution command */

typedef uint32_t mr_adc_data_t;                             /**< ADC read data type */

/**
 * @brief ADC configuration structure.
 */
struct mr_adc_config
{
    uint32_t channel_state;                                 /**< Channel state */
};

/**
 * @brief ADC structure.
 */
struct mr_adc
{
    struct mr_device device;                                /**< Device */

    uint32_t channels_mask;                                 /**< Channels state mask */
};

/**
 * @brief ADC driver operations structure.
 */
struct mr_adc_driver_ops
{
    int (*configure)(struct mr_driver *driver, bool enable);
    int (*channel_configure)(struct mr_driver *driver, uint32_t channel,
                             bool enable);
    int (*read)(struct mr_driver *driver, uint32_t channel, uint32_t *data);
};

/**
 * @brief ADC driver data structure.
 */
struct mr_adc_driver_data
{
    uint32_t channels_mask;                                 /**< Channels mask */
    uint32_t resolution;                                    /**< Resolution */
};

int mr_adc_register(struct mr_adc *adc, const char *path,
                    struct mr_driver *driver);

/** @} */

#endif /* MR_USE_ADC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DEVICE_ADC_H_ */
