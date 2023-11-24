/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_PIN_H_
#define _MR_PIN_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_PIN

/**
 * @brief PIN level.
 */
#define MR_PIN_LOW_LEVEL                (0)                         /* Low level */
#define MR_PIN_HIGH_LEVEL               (1)                         /* High level */

/**
 * @brief PIN mode.
 */
#define MR_PIN_MODE_NONE                (0)                         /* No mode */
#define MR_PIN_MODE_OUTPUT              (1)                         /* Output push-pull mode */
#define MR_PIN_MODE_OUTPUT_OD           (2)                         /* Output open-drain mode */
#define MR_PIN_MODE_INPUT               (3)                         /* Input mode */
#define MR_PIN_MODE_INPUT_DOWN          (4)                         /* Input pull-down mode */
#define MR_PIN_MODE_INPUT_UP            (5)                         /* Input pull-up mode */

/**
 * @brief PIN mode-interrupt.
 */
#define MR_PIN_MODE_IRQ_RISING          (6)                         /* Interrupt rising edge */
#define MR_PIN_MODE_IRQ_FALLING         (7)                         /* Interrupt falling edge */
#define MR_PIN_MODE_IRQ_EDGE            (8)                         /* Interrupt edge */
#define MR_PIN_MODE_IRQ_LOW             (9)                         /* Interrupt low level */
#define MR_PIN_MODE_IRQ_HIGH            (10)                        /* Interrupt high level */

/**
 * @brief PIN mode command.
 */
#define MR_CTRL_PIN_SET_PIN_MODE        ((0x01|0x80) << 16)         /**< Set pin mode */

/**
 * @brief PIN number command.
 */
#define MR_CTRL_PIN_SET_NUMBER          MR_CTRL_SET_OFFSET          /**< Set pin number */
#define MR_CTRL_PIN_GET_NUMBER          MR_CTRL_GET_OFFSET          /**< Get pin number */

/**
 * @brief PIN data type.
 */
typedef uint8_t mr_pin_data_t;                                     /**< PIN read/write data type */

/**
 * @brief PIN structure.
 */
struct mr_pin
{
    struct mr_dev dev;                                              /**< Device */
};

/**
 * @brief PIN operations structure.
 */
struct mr_pin_ops
{
    int (*configure)(struct mr_pin *pin, int number, int mode);
    int (*read)(struct mr_pin *pin, int number);
    void (*write)(struct mr_pin *pin, int number, int value);
};

/**
 * @addtogroup PIN.
 * @{
 */
int mr_pin_register(struct mr_pin *pin, const char *name, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_PIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PIN_H_ */
