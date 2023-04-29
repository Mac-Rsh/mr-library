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

#include <mrlib.h>

#if (MR_DEVICE_TIMER == MR_CONF_ENABLE)

#define MR_TIMER_MODE_PERIOD                        0
#define MR_TIMER_MODE_ONE_SHOT                      1

#define _MR_TIMER_CUT_MODE_UP                       0
#define _MR_TIMER_CUT_MODE_DOWN                     1

#define MR_TIMER_EVENT_PIT_INT                      0x1000
#define _MR_TIMER_EVENT_MASK                        0xf000

/* Default config for mr_timer_config structure */
#define MR_TIMER_CONFIG_DEFAULT                     \
{                                                   \
    1000,                                           \
    MR_TIMER_MODE_PERIOD,                           \
}

struct mr_timer_config
{
	mr_uint32_t freq;
	mr_uint8_t mode;
};

struct mr_timer_information
{
	mr_uint32_t max_freq;
	mr_uint32_t max_cut;
	mr_uint8_t cut_mode;
};

typedef struct mr_timer *mr_timer_t;
struct mr_timer_ops
{
	mr_err_t (*configure)(mr_timer_t timer, struct mr_timer_config *config);
	mr_err_t (*start)(mr_timer_t timer, mr_uint32_t period_reload);
	mr_err_t (*stop)(mr_timer_t timer);
	mr_uint32_t (*get_count)(mr_timer_t timer);
};

struct mr_timer
{
	struct mr_device device;

	struct mr_timer_config config;
	struct mr_timer_information information;
	mr_uint32_t reload;
	mr_uint32_t cycles;
	mr_uint32_t overflow;
	mr_uint32_t timeout;

	const struct mr_timer_ops *ops;
};

mr_err_t mr_hw_timer_add(mr_timer_t timer,
						 const char *name,
						 struct mr_timer_ops *ops,
						 struct mr_timer_information *information,
						 void *data);
void mr_hw_timer_isr(mr_timer_t timer, mr_uint16_t event);

#endif

#endif
