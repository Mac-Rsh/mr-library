/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#ifndef _MR_TIMER_H_
#define _MR_TIMER_H_

#include <include/mr_api.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_TIMER

/**
 * @addtogroup Timer
 * @{
 */

#define MR_TIMER_MODE_PERIOD            (0)                         /**< Periodic mode */
#define MR_TIMER_MODE_ONESHOT           (1)                         /**< One shot mode */

#define MR_CMD_TIMER_CONFIG             MR_CMD_CONFIG               /**< Configuration command */

#define MR_EVENT_TIMER_TIMEOUT_INT                                                                 \
    (MR_EVENT_WR | MR_EVENT_DATA | (0x01))                          /**< Interrupt on timeout event */

typedef uint32_t mr_timer_data_t;                                   /**< Timer read/write data type */

/**
 * @brief Timer default configuration.
 */
#define MR_TIMER_CONFIG_DEFAULT                                                                    \
{                                                                                                  \
    MR_TIMER_MODE_PERIOD,                                                                          \
}

/**
* @brief Timer configuration structure.
*/
struct mr_timer_config
{
    uint32_t mode;                                                  /**< Mode */
};

/**
 * @brief Timer structure.
 */
struct mr_timer
{
    struct mr_device device;                                        /**< Device */

    struct mr_timer_config config;                                  /**< Configuration */
    uint32_t reload;                                                /**< Reload value */
    uint32_t count;                                                 /**< Current count */
    uint32_t timeout;                                               /**< Timeout */
    uint32_t prescaler;                                             /**< Prescaler */
    uint32_t period;                                                /**< Period */
};

/**
 * @brief Timer driver operations structure.
 */
struct mr_timer_driver_ops
{
    int (*configure)(struct mr_driver *driver, bool enable);
    int (*start)(struct mr_driver *driver, uint32_t prescaler, uint32_t period);
    int (*stop)(struct mr_driver *driver);
    int (*get)(struct mr_driver *driver, uint32_t *count_value);
};

/**
 * @brief Timer driver data structure.
 */
struct mr_timer_driver_data
{
    uint32_t clk;                                                   /**< Clock(Hz) */
    uint32_t prescaler_max;                                         /**< Prescaler max */
    uint32_t period_max;                                            /**< Period max */
};

int mr_timer_register(struct mr_timer *timer, const char *path, const struct mr_driver *driver);

/** @} */

#endif /* MR_USE_TIMER */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_TIMER_H_ */
