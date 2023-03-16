/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-16     MacRsh       first version
 */

#ifndef _PIN_H_
#define _PIN_H_

#include <mrlib.h>

#if (MR_DEVICE_PIN == MR_CONF_ENABLE)

#define MR_PIN_MODE_OUT            0
#define MR_PIN_MODE_OUT_OD         1
#define MR_PIN_MODE_IN             2
#define MR_PIN_MODE_IN_UP          3
#define MR_PIN_MODE_IN_DOWN        4

struct mr_pin_config
{
	mr_uint16_t number;
	mr_uint16_t mode;
};

typedef struct mr_pin *mr_pin_t;
struct mr_pin_ops
{
	mr_err_t (*configure)(mr_pin_t pin, mr_uint16_t number, mr_uint16_t mode);
	void (*write)(mr_pin_t pin, mr_uint16_t number, mr_uint8_t value);
	mr_uint8_t (*read)(mr_pin_t pin, mr_uint16_t number);
};

struct mr_pin
{
	struct mr_device device;

	const struct mr_pin_ops *ops;
};

mr_err_t mr_hw_pin_add_to_container(mr_pin_t pin, const char *name, struct mr_pin_ops *ops, void *data);

#endif

#endif
