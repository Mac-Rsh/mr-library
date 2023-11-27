/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#ifndef _PWM_H_
#define _PWM_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_TIMER

/**
* @brief Timer channel mode.
*/
#define MR_TIMER_MODE_NONE              (0)
#define MR_TIMER_MODE_TIMING            (1)
#define MR_TIMER_MODE_PWM               (2)

#define MR_TIMER_TRIGGER_MODE_PERIODIC  (0)
#define MR_TIMER_TRIGGER_MODE_ONESHOT   (1)

#define MR_TIMER_STATE_DISABLE          MR_DISABLE
#define MR_TIMER_STATE_ENABLE           MR_ENABLE

/**
* @brief Timer default configuration.
*/
#define MR_TIMER_CONFIG_DEFAULT         \
{                                       \
    1000,                               \
}

/**
 * @brief Timer configuration structure.
 */
struct mr_timer_config
{
    uint32_t freq;                                                   /* Frequency */
    uint32_t mode: 2;                                                /* Mode */
    uint32_t trigger_mode: 1;
};

/**
 * @brief Timer channel mode command.
 */
#define MR_CTL_TIMER_SET_CHANNEL_STATE  ((0x01|0x80) << 16)         /**< Set channel mode */
#define MR_CTL_TIMER_GET_CHANNEL_STATE  ((0x01|0x00) << 16)         /**< Get channel mode */

#define MR_CTL_TIMER_SET_TIMING         ((0x02|0x80) << 16)         /**< Set timing */

struct mr_timer_info
{
    uint32_t clk;                                                    /* Clock */
    uint32_t prescaler_max;
    uint32_t period_max;
};

/**
 * @brief Timer data type.
 */
typedef uint8_t mr_timer_data_t;                                    /**< Timer read/write data type */

/**
 * @brief Timer structure.
 */
struct mr_timer
{
    struct mr_dev dev;                                              /* Device */

    struct mr_timer_info info;                                      /* Information */
    struct mr_timer_config config;                                  /* Config */
    uint32_t channel;                                               /* Channel */
    uint32_t prescaler;                                             /* Prescaler */
    uint32_t period;                                                /* Period */
    uint32_t reload;
};

/**
 * @brief Timer operations structure.
 */
struct mr_timer_ops
{
    int (*configure)(struct mr_timer *timer, int state);
    int (*channel_configure)(struct mr_timer *timer, int channel, int mode);
    void (*start)(struct mr_timer *timer, uint32_t prescaler, uint32_t period);
    void (*stop)(struct mr_timer *timer);
    void (*set_ccr)(struct mr_timer *timer, int channel, uint32_t duty);
    uint32_t (*get_duty)(struct mr_timer *timer, int channel);
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
