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

#include "mrlib.h"

#if (MR_CONF_PIN == MR_CONF_ENABLE)

#define MR_PIN_MODE_NONE                0
#define MR_PIN_MODE_OUTPUT              1
#define MR_PIN_MODE_OUTPUT_OD           2
#define MR_PIN_MODE_INPUT               3
#define MR_PIN_MODE_INPUT_DOWN          4
#define MR_PIN_MODE_INPUT_UP            5

#define MR_PIN_MODE_RISING              6
#define MR_PIN_MODE_FALLING             7
#define MR_PIN_MODE_EDGE                8
#define MR_PIN_MODE_LOW                 9
#define MR_PIN_MODE_HIGH                10

struct mr_pin_config
{
    mr_uint16_t number;
    mr_uint8_t mode;
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

mr_err_t mr_pin_device_add(mr_pin_t pin, const char *name, struct mr_pin_ops *ops, void *data);
void mr_pin_device_isr(mr_pin_t pin, mr_uint32_t Line);

#endif

#endif