/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-10    MacRsh       First version
 */

#ifndef _MR_PWM_H_
#define _MR_PWM_H_

#include <include/mr_api.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_PWM

/**
 * @addtogroup Timer
 * @{
 */

#define MR_PWM_CHANNEL_STATE_DISABLE    (false)                     /**< Channel disable */
#define MR_PWM_CHANNEL_STATE_ENABLE     (true)                      /**< Channel enable */

#define MR_PWM_POLARITY_NORMAL          (0)                         /**< Normal polarity */
#define MR_PWM_POLARITY_INVERTED        (1)                         /**< Inverted polarity */

#define MR_CMD_PWM_CHANNEL              MR_CMD_POS                  /**< Channel command */
#define MR_CMD_PWM_CONFIG               MR_CMD_CONFIG               /**< Configuration command */
#define MR_CMD_PWM_FREQ                 (0x01)                      /**< Frequency command */

typedef uint32_t mr_pwm_data_t;                                     /**< PWM read/write data type */
typedef uint32_t mr_pwm_freq_t;                                     /**< PWM frequency type */

/**
 * @brief PWM configuration structure.
 */
struct mr_pwm_config
{
    uint32_t channel_state;                                         /**< Channel state */
    uint32_t polarity;                                              /**< Polarity */
};

/**
 * @brief PWM structure.
 */
struct mr_pwm
{
    struct mr_device device;                                        /**< Device */

    uint32_t freq;                                                  /**< Frequency */
    uint32_t prescaler;                                             /**< Prescaler */
    uint32_t period;                                                /**< Period */
    uint32_t channels;                                              /**< Channels state mask */
};

/**
 * @brief PWM driver operations structure.
 */
struct mr_pwm_driver_ops
{
    int (*configure)(struct mr_driver *driver, bool enable);
    int (*start)(struct mr_driver *driver, uint32_t prescaler, uint32_t period);
    int (*channel_configure)(struct mr_driver *driver, uint32_t channel, bool enable,
                             uint32_t polarity);
    int (*get)(struct mr_driver *driver, uint32_t channel, uint32_t *compare_value);
    int (*set)(struct mr_driver *driver, uint32_t channel, uint32_t compare_value);
};

/**
 * @brief PWM driver data structure.
 */
struct mr_pwm_driver_data
{
    uint32_t clk;                                                   /**< Clock(Hz) */
    uint32_t prescaler_max;                                         /**< Prescaler max */
    uint32_t period_max;                                            /**< Period max */
    uint32_t channels;                                              /**< Channels exists mask */
};

/** @} */

#endif /* MR_USE_PWM */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PWM_H_ */
