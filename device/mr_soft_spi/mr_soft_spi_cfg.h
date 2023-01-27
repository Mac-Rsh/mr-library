/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-14     MacRsh       first version
 */

#ifndef _SOFT_SPI_CFG_H_
#define _SOFT_SPI_CFG_H_

/** user specification :
 *  ( __CONFIG_ENABLE ) it means enable function
 *  ( __CONFIG_DISABLE ) it means disable function
 *  ( //< must > ) it means this option need user complement
 *  ( //< const > ) it means this option unchangeable
 *  ( //< change able > ) it means this option can change, you could change parameter or annotation parameter
 *  ( //< optional > ) it means this option is optional, not essential
 *  ( //<<< XXXX >>> ) it means interpretation this define means
 */

#define __CONFIG_ENABLE             1
#define __CONFIG_DISABLE            0

//<---------------------------- FRAME ----------------------------------------->
//< must >
//<<< If you not using mr_device frame, please remove the following include, replace with your chip head-file >>>
//#include "device_def.h"
#include "ch32v30x.h"
//< change able >
//<<< Using soft-spi >>>
#define USING_SOFT_SPI              __CONFIG_ENABLE
//<<< spi-bus cs valid only when using, default: __CONFIG_ENABLE >>>
#define U_SOFT_SPI_BUS_CS_VALID     __CONFIG_ENABLE
//<<< Using debug >>>
#define U_SOFT_SPI_DEBUG            __CONFIG_ENABLE

//< optional >
//<<< rtos critical, disable interrupt, replace with rtos function, default: 0 >>>
#define mr_hw_interrupt_disable()   0
//<<< rtos critical, enable interrupt >>>
#define mr_hw_interrupt_enable(EX);

#endif /* end of _SOFT_SPI_CFG_H_ */
