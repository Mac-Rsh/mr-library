/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#ifndef _PIN_H_
#define _PIN_H_

#include "stdint.h"

#define PIN_ASSERT(x)

#define PIN_MODE_NONE                   0
#define PIN_MODE_OUTPUT                 1
#define PIN_MODE_OUTPUT_OD              2
#define PIN_MODE_INPUT                  3
#define PIN_MODE_INPUT_DOWN             4
#define PIN_MODE_INPUT_UP               5

#define PIN_MODE_RISING                 6
#define PIN_MODE_FALLING                7
#define PIN_MODE_EDGE                   8
#define PIN_MODE_LOW                    9
#define PIN_MODE_HIGH                   10

struct pin_config
{
    uint16_t number;
    uint8_t mode;
};

typedef struct pin *pin_t;

struct pin_ops
{
    int (*configure)(pin_t pin, struct pin_config *config);
    uint8_t (*read)(pin_t pin, uint16_t number);
    void (*write)(pin_t pin, uint16_t number, uint8_t value);
};

struct pin
{
    int (*callback)(pin_t pin, uint16_t number);
    void *data;

    struct pin_ops *ops;
};

#define PIN_ERR_OK                        0
#define PIN_ERR_IO                        1

#define GET_PIN_NUMBER(port, pin)       ((port -'A') * 16 + pin)

int pin_init(pin_t pin, struct pin_ops *ops, void *data);
void pin_set_rx_callback(pin_t pin, int (*callback)(pin_t pin, uint16_t number));
int pin_configure(pin_t pin, struct pin_config *config);
int pin_mode(pin_t pin, uint16_t number, uint8_t mode);
uint8_t pin_read(pin_t pin, uint16_t number);
void pin_write(pin_t pin, uint16_t number, uint8_t value);
void pin_hw_isr(pin_t pin, uint16_t number);

#endif
