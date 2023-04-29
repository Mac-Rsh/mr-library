/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _PIN_H_
#define _PIN_H_

#include <mrlib.h>

#if (MR_DEVICE_PIN == MR_CONF_ENABLE)

#define MR_PIN_MODE_NONE                0
#define MR_PIN_MODE_OUT                 1
#define MR_PIN_MODE_OUT_OD              2
#define MR_PIN_MODE_IN                  3
#define MR_PIN_MODE_IN_UP               4
#define MR_PIN_MODE_IN_DOWN             5

#define MR_PIN_IRQ_MODE_NONE            0
#define MR_PIN_IRQ_MODE_RISING          1
#define MR_PIN_IRQ_MODE_FALLING         2
#define MR_PIN_IRQ_MODE_EDGE            3
#define MR_PIN_IRQ_MODE_HIGH            4
#define MR_PIN_IRQ_MODE_LOW             5

struct mr_pin_config
{
	mr_uint16_t number;
	mr_uint8_t mode;
	mr_uint8_t irq_mode;
};

typedef struct mr_pin *mr_pin_t;
struct mr_pin_ops
{
	mr_err_t (*configure)(mr_pin_t pin, struct mr_pin_config *config);
	void (*write)(mr_pin_t pin, mr_uint16_t number, mr_uint8_t value);
	mr_uint8_t (*read)(mr_pin_t pin, mr_uint16_t number);
};

struct mr_pin
{
	struct mr_device device;

	const struct mr_pin_ops *ops;
};

mr_err_t mr_hw_pin_add(mr_pin_t pin, const char *name, struct mr_pin_ops *ops, void *data);
void mr_hw_pin_isr(mr_pin_t pin, mr_uint32_t Line);

#endif

#endif
