/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-13    MacRsh       First version
 */

#ifndef _MR_SOFT_I2C_H_
#define _MR_SOFT_I2C_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C)

#include "include/device/mr_i2c.h"

/**
* @addtogroup I2C
* @{
*/

/**
* @brief Soft-I2C bus structure.
*/
struct mr_soft_i2c_bus
{
    struct mr_i2c_bus i2c_bus;                                      /**< I2C-bus device */

    uint32_t delay;                                                 /**< Speed delay */
    int scl_pin;                                                    /**< SCL pin */
    int sda_pin;                                                    /**< SDA pin */
};

int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus,
                             const char *path,
                             int scl_pin,
                             int sda_pin);
/** @} */

#endif /* defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SOFT_I2C_H_ */
