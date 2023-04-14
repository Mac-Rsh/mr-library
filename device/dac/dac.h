/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-28     MacRsh       first version
 */

#ifndef _DAC_H_
#define _DAC_H_

#include <mrlib.h>

#if (MR_DEVICE_DAC == MR_CONF_ENABLE)

struct mr_dac_config
{
	mr_uint16_t channel;
	mr_state_t state;
};

typedef struct mr_dac *mr_dac_t;
struct mr_dac_ops
{
	mr_err_t (*configure)(mr_dac_t dac, mr_state_t state);
	mr_err_t (*channel_configure)(mr_dac_t dac, mr_uint16_t channel, mr_state_t state);
	void (*write)(mr_dac_t dac, mr_uint16_t channel, mr_uint16_t value);
};

struct mr_dac
{
	struct mr_device device;

	const struct mr_dac_ops *ops;
};

mr_err_t mr_hw_dac_add_to_container(mr_dac_t dac, const char *name, struct mr_dac_ops *ops, void *data);

#endif

#endif