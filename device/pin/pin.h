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

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_PIN == MR_CFG_ENABLE)

/**
 * @def Pin device mode
 */
#define MR_PIN_MODE_NONE                0
#define MR_PIN_MODE_OUTPUT              1
#define MR_PIN_MODE_OUTPUT_OD           2
#define MR_PIN_MODE_INPUT               3
#define MR_PIN_MODE_INPUT_DOWN          4
#define MR_PIN_MODE_INPUT_UP            5

/**
 * @def Pin device interrupt mode
 */
#define MR_PIN_MODE_IRQ_RISING          6
#define MR_PIN_MODE_IRQ_FALLING         7
#define MR_PIN_MODE_IRQ_EDGE            8
#define MR_PIN_MODE_IRQ_LOW             9
#define MR_PIN_MODE_IRQ_HIGH            10

/**
 * @struct Pin device config
 */
struct mr_pin_config
{
    mr_off_t number: 28;
    mr_uint32_t mode: 4;
};
typedef struct mr_pin_config *mr_pin_config_t;

typedef struct mr_pin *mr_pin_t;

/**
 * @struct Pin device operations
 */
struct mr_pin_ops
{
    mr_err_t (*configure)(mr_pin_t pin, mr_pin_config_t config);
    mr_level_t (*read)(mr_pin_t pin, mr_off_t number);
    void (*write)(mr_pin_t pin, mr_off_t number, mr_level_t level);
};

/**
 * @struct Pin device
 */
struct mr_pin
{
    struct mr_device device;

    const struct mr_pin_ops *ops;
};

/**
 * @addtogroup Pin device
 * @{
 */
mr_err_t mr_pin_device_add(mr_pin_t pin, const char *name, struct mr_pin_ops *ops, void *data);
void mr_pin_device_isr(mr_pin_t pin, mr_off_t number);
/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* _PIN_H_ */