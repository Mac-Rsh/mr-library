/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#include "pin.h"

static int _err_io_pin_configure(pin_t pin, struct pin_config *config)
{
    PIN_ASSERT(0);
    return -PIN_ERR_IO;
}

static uint8_t _err_io_pin_read(pin_t pin, uint16_t number)
{
    PIN_ASSERT(0);
    return 0;
}

static void _err_io_pin_write(pin_t pin, uint16_t number, uint8_t value)
{
    PIN_ASSERT(0);
}

int pin_init(pin_t pin, struct pin_ops *ops, void *data)
{
    PIN_ASSERT(pin != NULL);
    PIN_ASSERT(ops != NULL);

    pin->callback = NULL;
    pin->data = data;

    ops->configure = ops->configure ? ops->configure : _err_io_pin_configure;
    ops->read = ops->read ? ops->read : _err_io_pin_read;
    ops->write = ops->write ? ops->write : _err_io_pin_write;
    pin->ops = ops;

    return PIN_ERR_OK;
}

void pin_set_rx_callback(pin_t pin, int (*callback)(pin_t pin, uint16_t number))
{
    PIN_ASSERT(pin != NULL);

    pin->callback = callback;
}

int pin_configure(pin_t pin, struct pin_config *config)
{
    PIN_ASSERT(pin != NULL);
    PIN_ASSERT(config != NULL);

    return pin->ops->configure(pin, config);
}

int pin_mode(pin_t pin, uint16_t number, uint8_t mode)
{
    struct pin_config config = {number,
                                mode};

    PIN_ASSERT(pin != NULL);

    return pin->ops->configure(pin, &config);
}

uint8_t pin_read(pin_t pin, uint16_t number)
{
    PIN_ASSERT(pin != NULL);

    return pin->ops->read(pin, number);
}

void pin_write(pin_t pin, uint16_t number, uint8_t value)
{
    PIN_ASSERT(pin != NULL);

    pin->ops->write(pin, number, value);
}

void pin_hw_isr(pin_t pin, uint16_t number)
{
    PIN_ASSERT(pin != NULL);

    if (pin->callback != NULL)
    {
        pin->callback(pin, number);
    }
}