/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#ifndef _HX711_H_
#define _HX711_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_HX711

/**
 * @brief HX711 command.
 */
#define MR_CTL_HX711_SET_FILTER_BITS   ((0x01|0x80) << 16)         /**< Set filter bits */
#define MR_CTL_HX711_SET_SELF_CAL      ((0x02|0x80) << 16)         /**< Set self calibration */
#define MR_CTL_HX711_GET_FILTER_BITS   ((0x01|0x00) << 16)         /**< Get filter bits */
#define MR_CTL_HX711_GET_SELF_CAL      ((0x02|0x00) << 16)         /**< Get self calibration */

/**
 * @brief HX711 data type.
 */
typedef uint32_t mr_hx711_data_t;                                   /**< HX711 read data type */

/**
 * @brief Hx711 structure.
 */
struct mr_hx711
{
    struct mr_dev dev;                                              /**< Device */

    int filter_bits;                                                /**< Filter bits */
    uint32_t self_cal;                                              /**< Self calibration */
    int sck_pin;                                                    /**< SCK pin */
    int dout_pin;                                                   /**< DOUT pin */
    int desc;                                                       /**< Descriptor */
};

/**
 * @addtogroup HX711.
 * @{
 */
int mr_hx711_register(struct mr_hx711 *hx711, const char *name, int sck_pin, int dout_pin);
/** @} */
#endif /* MR_USING_HX711 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HX711_H_ */
