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
 * @addtogroup PWM
 * @{
 */

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
#define MR_IOC_PWM_SET_CHANNEL          MR_IOC_SPOS                 /**< Set channel command */
#define MR_IOC_PWM_SET_CHANNEL_CONFIG   MR_IOC_SCFG                 /**< Set channel configuration command */
#define MR_IOC_PWM_SET_FREQ             (0x01)                      /**< Set frequency command */

#define MR_IOC_PWM_GET_CHANNEL          MR_IOC_GPOS                 /**< Get channel command */
#define MR_IOC_PWM_GET_CHANNEL_CONFIG   MR_IOC_GCFG                 /**< Get channel configuration command */
#define MR_IOC_PWM_GET_FREQ             (-(0x01))                   /**< Get frequency command */

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
    int (*read)(struct mr_pwm *pwm, int channel, uint32_t *compare_value);
    int (*write)(struct mr_pwm *pwm, int channel, uint32_t compare_value);
};

int mr_pwm_register(struct mr_pwm *pwm,
                    const char *path,
                    struct mr_drv *drv,
                    struct mr_pwm_info *info);
/** @} */

#endif /* MR_USING_PWM */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PWM_H_ */
