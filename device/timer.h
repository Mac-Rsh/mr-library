/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

/**
 * @def Timer device mode
 */
#define MR_TIMER_MODE_PERIOD            0
#define MR_TIMER_MODE_ONE_SHOT          1

/**
 * @def Timer device counter mode
 */
#define MR_TIMER_COUNT_MODE_UP          0
#define MR_TIMER_COUNT_MODE_DOWN        1

/**
 * @def Timer device interrupt event
 */
#define MR_TIMER_EVENT_PIT_INT          0x10000000
#define MR_TIMER_EVENT_MASK             0xf0000000

/**
 * @def Timer device default config
 */
#define MR_TIMER_CONFIG_DEFAULT         \
{                                       \
    MR_TIMER_MODE_PERIOD,               \
}

/**
 * @struct Timer device config
 */
struct mr_timer_config
{
    mr_uint32_t mode: 1;
    mr_uint32_t reserved: 31;
};
typedef struct mr_timer_config *mr_timer_config_t;

/**
 * @struct Timer device data
 */
struct mr_timer_data
{
    mr_uint32_t clk;
    mr_uint32_t prescaler_max;
    mr_uint32_t period_max;
    mr_uint32_t count_mode: 1;
    mr_uint32_t reserved: 31;
};

typedef struct mr_timer *mr_timer_t;

/**
 * @struct Timer device operations
 */
struct mr_timer_ops
{
    mr_err_t (*configure)(mr_timer_t timer, mr_state_t state);
    void (*start)(mr_timer_t timer, mr_uint32_t prescaler, mr_uint32_t period);
    void (*stop)(mr_timer_t timer);
    mr_uint32_t (*get_count)(mr_timer_t timer);
};

/**
 * @struct Timer device
 */
struct mr_timer
{
    struct mr_device device;

    struct mr_timer_config config;
    mr_uint32_t reload;
    mr_uint32_t overflow;
    mr_uint32_t count;
    mr_uint32_t timeout;

    struct mr_timer_data *data;
    const struct mr_timer_ops *ops;
};

/**
 * @addtogroup Timer
 * @{
 */
mr_err_t mr_timer_device_add(mr_timer_t timer,
                             const char *name,
                             struct mr_timer_ops *ops,
                             struct mr_timer_data *timer_data,
                             void *data);
void mr_timer_device_isr(mr_timer_t timer, mr_uint32_t event);
/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
