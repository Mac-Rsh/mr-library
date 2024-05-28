/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#ifndef _MR_PIN_H_
#define _MR_PIN_H_

#include "../mr-library/include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USE_PIN

/**
 * @addtogroup PIN
 * @{
 */

#define MR_PIN_MODE_NONE                (0)                 /**< No mode */
#define MR_PIN_MODE_OUTPUT              (1)                 /**< Output push-pull mode */
#define MR_PIN_MODE_OUTPUT_OD           (2)                 /**< Output open-drain mode */
#define MR_PIN_MODE_INPUT               (3)                 /**< Input mode */
#define MR_PIN_MODE_INPUT_UP            (4)                 /**< Input pull-up mode */
#define MR_PIN_MODE_INPUT_DOWN          (5)                 /**< Input pull-down mode */
#define MR_PIN_MODE_IRQ_RISING          (6)                 /**< Interrupt rising edge mode */
#define MR_PIN_MODE_IRQ_FALLING         (7)                 /**< Interrupt falling edge mode */
#define MR_PIN_MODE_IRQ_EDGE            (8)                 /**< Interrupt edge mode */
#define MR_PIN_MODE_IRQ_LOW             (9)                 /**< Interrupt low level mode */
#define MR_PIN_MODE_IRQ_HIGH            (10)                /**< Interrupt high level mode */

#define MR_CMD_PIN_NUMBER               MR_CMD_POS          /**< PIN number command */
#define MR_CMD_PIN_MODE                 MR_CMD_CONFIG       /**< PIN mode command */

#define MR_EVENT_PIN_EXTI_INT                                                  \
    (MR_EVENT_RD | MR_EVENT_DATA | (0x01))                  /**< Interrupt on EXTI event */

/**
 * @brief PIN configuration structure.
 */
struct mr_pin_config
{
    uint32_t mode;                                          /**< Mode */
};

typedef uint8_t mr_pin_data_t;                              /**< PIN read/write data type */

/**
 * @brief PIN structure.
 */
struct mr_pin
{
    struct mr_device device;                                /**< Device */

    uint32_t pins[16];                                      /**< Pins mode mask */
};

/**
 * @brief PIN driver operations structure.
 */
struct mr_pin_driver_ops
{
    int (*configure)(struct mr_driver *driver, uint32_t number, uint32_t mode);
    int (*read)(struct mr_driver *driver, uint32_t number, uint8_t *value);
    int (*write)(struct mr_driver *driver, uint32_t number, uint8_t value);
};

/**
 * @brief PIN driver data structure.
 */
struct mr_pin_driver_data
{
    uint32_t pins[16];                                      /**< Pins exists mask */
};

  int mr_pin_register(struct mr_pin *pin, const char *path,
                      const struct mr_driver *driver);

/** @} */

#endif /* MR_USE_PIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_PIN_H_ */
