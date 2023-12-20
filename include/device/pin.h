/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_PIN_H_
#define _MR_PIN_H_

#include "include/mr_api.h"

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
 * @brief PIN configuration structure.
 */
struct mr_pin_config
{
    uint32_t mode: 4;                                               /**< Mode */
    uint32_t reserved: 28;                                          /**< Reserved */
};

/**
 * @brief PIN control command.
 */
#define MR_CTL_PIN_SET_NUMBER           MR_CTL_SET_OFFSET           /**< Set pin number */
#define MR_CTL_PIN_SET_MODE             MR_CTL_SET_CONFIG           /**< Set pin mode */
#define MR_CTL_PIN_SET_EXTI_CALL        MR_CTL_SET_RD_CALL          /**< Set pin exti callback */

#define MR_CTL_PIN_GET_NUMBER           MR_CTL_GET_OFFSET           /**< Get pin number */
#define MR_CTL_PIN_GET_EXTI_CALL        MR_CTL_GET_RD_CALL          /**< Get pin exti callback */

/**
 * @brief PIN data type.
 */
typedef uint8_t mr_pin_data_t;                                      /**< PIN read/write data type */

/**
 * @brief PIN ISR events.
 */
#define MR_ISR_PIN_EXTI_INT             (MR_ISR_RD | (0x01 << 8))   /**< Exti interrupt */

/**
 * @brief PIN structure.
 */
struct mr_pin
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_list irq_list;                                        /**< IRQ list */
};

/**
 * @brief PIN operations structure.
 */
struct mr_pin_ops
{
    int (*configure)(struct mr_pin *pin, int number, int mode);
    uint8_t (*read)(struct mr_pin *pin, int number);
    void (*write)(struct mr_pin *pin, int number, uint8_t value);
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
