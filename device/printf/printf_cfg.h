/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-20     MacRsh       first version
 */

#ifndef _PRINTF_CFG_H_
#define _PRINTF_CFG_H_

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
#include "device_def.h"
//< change able >
//<<< Using built-in mr_printf >>>
#define USING_MR_PRINTF             __CONFIG_ENABLE
//<<< Using the decimal-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_DEC         __CONFIG_ENABLE
//<<< Using the hexadecimal-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_HEX         __CONFIG_ENABLE
//<<< Using the octal-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_OCT         __CONFIG_ENABLE
//<<< Using the unsigned-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_UNSIGNED    __CONFIG_ENABLE
//<<< Using the char-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_CHAR        __CONFIG_ENABLE
//<<< Using the string-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_STRING      __CONFIG_ENABLE
//<<< Using the float-printf mode in the built-in mr_printf >>>
#define USING_MR_PRINTF_FLOAT       __CONFIG_DISABLE

#endif /* end of _PRINTF_CFG_H_ */
