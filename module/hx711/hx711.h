/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#ifndef _HX711_H
#define _HX711_H

#include "mr_lib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_HX711

/**
 * @brief HX711 command.
 */
#define HX711_CTRL_SET_FILTER_BITS      ((0x01|0x80) << 16)
#define HX711_CTRL_SET_SELF_CALIBRATION ((0x02|0x80) << 16)
#define HX711_CTRL_GET_FILTER_BITS      ((0x01|0x00) << 16)
#define HX711_CTRL_GET_SELF_CALIBRATION ((0x02|0x00) << 16)

/**
 * @brief Hx711 structure.
 */
struct hx711
{
    struct mr_dev dev;                                              /**< Device */

    int filter_bits;                                                /**< Filter bits */
    uint32_t self_calibration;                                      /**< Self calibration */
    int sck_pin;                                                    /**< SCK pin */
    int dout_pin;                                                   /**< DOUT pin */
    int desc;                                                       /**< Descriptor */
};

/**
 * @addtogroup HX711.
 * @{
 */
int hx711_register(struct hx711 *hx711, const char *name, int sck_pin, int dout_pin);
/** @} */
#endif /* MR_USING_HX711 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HX711_H */
