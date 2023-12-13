/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-13    MacRsh       First version
 */

#ifndef _SOFT_I2C_H_
#define _SOFT_I2C_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C)

#include "include/device/i2c.h"

/**
* @brief Soft-I2C bus structure.
*/
struct mr_soft_i2c_bus
{
    struct mr_i2c_bus i2c_bus;                                      /* I2C-bus device */

    uint32_t delay;                                                 /* Speed delay */
    int scl_pin;                                                    /* SCL pin */
    int sda_pin;                                                    /* SDA pin */
    int desc;                                                       /* SCL-SDA descriptor */
};

/**
* @addtogroup Soft-I2C.
* @{
*/
int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus, const char *name, int scl_pin, int sda_pin);
/** @} */
#endif /* defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SOFT_I2C_H_ */
