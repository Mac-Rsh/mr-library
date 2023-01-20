/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-20     MacRsh       first version
 */

#ifndef _PRINTF_H_
#define _PRINTF_H_

#include "printf_cfg.h"

#if (USING_MR_PRINTF == __CONFIG_ENABLE)

/* include va_list/va_start/va_end/va_arg */
#include <stdarg.h>

/* Compiler Related Definitions */
#if defined(__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
#define MR_WEAK                      __attribute__((weak))

#elif (defined (__CC_ARM) || defined(__CLANG_ARM))   /* ARM Compiler */
#define MR_WEAK                      __weak

#elif defined(__GNUC__)              /* GNU GCC Compiler */
#define MR_WEAK                      __attribute__((weak))

#endif  /* end of __IAR_SYSTEMS_ICC__ */

int mr_printf(char *fmt, ...);

#endif /* end of USING_MR_PRINTF */

#endif /* end of _PRINTF_H_ */
