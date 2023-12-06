/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#ifndef _PWM_H_
#define _PWM_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_TIMER

/**
* @brief Timer mode.
*/
#define MR_TIMER_MODE_PERIOD            (0)                         /**< Periodic mode */
#define MR_TIMER_MODE_ONESHOT           (1)                         /**< One shot mode */

/**
* @brief Timer default configuration.
*/
#define MR_TIMER_CONFIG_DEFAULT         \
{                                       \
    MR_TIMER_MODE_PERIOD,               \
}

/**
 * @brief Timer configuration structure.
 */
struct mr_timer_config
{
    uint32_t mode: 1;                                                /* Mode */
    uint32_t reserved: 31;
};

/**
 * @brief Timer control command.
 */
#define MR_CTL_TIMER_SET_CONFIG         MR_CTL_SET_CONFIG           /**< Set configuration */
#define MR_CTL_TIMER_SET_TIMEOUT_CALL   MR_CTL_SET_RD_CALL          /**< Set timeout callback */

#define MR_CTL_TIMER_GET_CONFIG         MR_CTL_GET_CONFIG           /**< Get configuration */
#define MR_CTL_TIMER_GET_TIMEOUT_CALL   MR_CTL_GET_RD_CALL          /**< Get timeout callback */

/**
 * @brief Timer data type.
 */
typedef uint8_t mr_timer_data_t;                                    /**< Timer read/write data type */

/**
 * @brief Timer ISR events.
 */
#define MR_ISR_TIMER_TIMEOUT_INT        (MR_ISR_RD | (0x01 << 16))  /**< Timeout interrupt */

/**
 * @brief Timer information structure.
 */
struct mr_timer_info
{
    uint32_t clk;                                                   /* Clock(MHz) */
    uint32_t prescaler_max;                                         /* Prescaler max */
    uint32_t period_max;                                            /* Period max */
};

/**
 * @brief Timer structure.
 */
struct mr_timer
{
    struct mr_dev dev;                                              /* Device */

    struct mr_timer_config config;                                  /* Config */
    uint32_t reload;                                                /* Reload */
    uint32_t count;                                                 /* Count */
    uint32_t timeout;                                               /* Timeout */
    uint32_t period;                                                /* Period */
    uint32_t prescaler;                                             /* Prescaler */

    struct mr_timer_info *info;                                     /* Information */
};

/**
 * @brief Timer operations structure.
 */
struct mr_timer_ops
{
    int (*configure)(struct mr_timer *timer, int state);
    void (*start)(struct mr_timer *timer, uint32_t prescaler, uint32_t period);
    void (*stop)(struct mr_timer *timer);
    uint32_t (*get_count)(struct mr_timer *timer);
};

/**
 * @addtogroup Timer.
 * @{
 */
int mr_timer_register(struct mr_timer *timer, const char *name, struct mr_drv *drv, struct mr_timer_info *info);
/** @} */

#endif /* MR_USING_TIMER */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PWM_H_ */
