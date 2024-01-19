/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-10    MacRsh       First version
 */

#ifndef _MR_PWM_H_
#define _MR_PWM_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_PWM

/**
 * @brief PWM channel polarity.
 */
#define MR_PWM_POLARITY_NORMAL          (0)                         /**< PWM normal polarity */
#define MR_PWM_POLARITY_INVERTED        (1)                         /**< PWM inverted polarity */

/**
 * @brief PWM configuration structure.
 */
struct mr_pwm_config
{
    int state;                                                      /**< Channel state */
    int polarity;                                                   /**< Channel polarity */
};

/**
 * @brief PWM control command.
 */
#define MR_CTL_PWM_SET_CHANNEL          MR_CTL_SET_OFFSET           /**< Set channel */
#define MR_CTL_PWM_SET_CHANNEL_CONFIG   MR_CTL_SET_CONFIG           /**< Set channel config */
#define MR_CTL_PWM_SET_FREQ             (0x01 << 8)                 /**< Set frequency */

#define MR_CTL_PWM_GET_CHANNEL          MR_CTL_GET_OFFSET           /**< Get channel */
#define MR_CTL_PWM_GET_CHANNEL_CONFIG   MR_CTL_GET_CONFIG           /**< Get channel config */
#define MR_CTL_PWM_GET_FREQ             (-(0x01 << 8))              /**< Get frequency */

/**
 * @brief PWM data type.
 */
typedef uint32_t mr_pwm_data_t;                                     /**< PWM read/write data type */

/**
 * @brief PWM information structure.
 */
struct mr_pwm_info
{
    uint32_t clk;                                                   /**< Clock(Hz) */
    uint32_t prescaler_max;                                         /**< Prescaler max */
    uint32_t period_max;                                            /**< Period max */
};

/**
 * @brief PWM structure.
 */
struct mr_pwm
{
    struct mr_dev dev;                                              /**< Device */

    uint32_t freq;                                                  /**< Frequency */
    uint32_t prescaler;                                             /**< Prescaler */
    uint32_t period;                                                /**< Period */
    uint32_t channel;                                               /**< Channel */
    uint32_t channel_polarity;                                      /**< Channel polarity */

    struct mr_pwm_info *info;                                       /**< Information */
};

/**
 * @brief PWM operations structure.
 */
struct mr_pwm_ops
{
    int (*configure)(struct mr_pwm *pwm, int state);
    int (*channel_configure)(struct mr_pwm *pwm, int channel, int state, int polarity);
    void (*start)(struct mr_pwm *pwm, uint32_t prescaler, uint32_t period);
    void (*write)(struct mr_pwm *pwm, int channel, uint32_t compare_value);
    uint32_t (*read)(struct mr_pwm *pwm, int channel);
};

/**
 * @addtogroup PWM.
 * @{
 */
int mr_pwm_register(struct mr_pwm *pwm, const char *name, struct mr_drv *drv, struct mr_pwm_info *info);
/** @} */
#endif /* MR_USING_PWM */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PWM_H_ */
