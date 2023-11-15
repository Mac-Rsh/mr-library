/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_GPIO_H_
#define _MR_GPIO_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_GPIO

/**
 * @brief GPIO level.
 */
#define MR_GPIO_LOW_LEVEL               (0)                         /* Low level */
#define MR_GPIO_HIGH_LEVEL              (1)                         /* High level */

/**
 * @brief GPIO mode.
 */
#define MR_GPIO_MODE_NONE               (0)                         /* No mode */
#define MR_GPIO_MODE_OUTPUT             (1)                         /* Output push-pull mode */
#define MR_GPIO_MODE_OUTPUT_OD          (2)                         /* Output open-drain mode */
#define MR_GPIO_MODE_INPUT              (3)                         /* Input mode */
#define MR_GPIO_MODE_INPUT_DOWN         (4)                         /* Input pull-down mode */
#define MR_GPIO_MODE_INPUT_UP           (5)                         /* Input pull-up mode */

/**
 * @brief GPIO interrupt mode.
 */
#define MR_GPIO_MODE_IRQ_RISING         (6)                         /* Interrupt rising edge */
#define MR_GPIO_MODE_IRQ_FALLING        (7)                         /* Interrupt falling edge */
#define MR_GPIO_MODE_IRQ_EDGE           (8)                         /* Interrupt edge */
#define MR_GPIO_MODE_IRQ_LOW            (9)                         /* Interrupt low level */
#define MR_GPIO_MODE_IRQ_HIGH           (10)                        /* Interrupt high level */

/**
 * @brief GPIO structure.
 */
struct mr_gpio
{
    struct mr_dev dev;                                              /**< Device */
};

/**
 * @brief GPIO operations structure.
 */
struct mr_gpio_ops
{
    int (*configure)(struct mr_gpio *gpio, int pin, int mode);
    int (*read)(struct mr_gpio *gpio, int pin);
    void (*write)(struct mr_gpio *gpio, int pin, int value);
};

/**
 * @addtogroup GPIO.
 * @{
 */
int mr_gpio_register(struct mr_gpio *gpio, const char *name, struct mr_drv *drv);
/** @} */
#endif /* MR_USING_GPIO */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_GPIO_H_ */
