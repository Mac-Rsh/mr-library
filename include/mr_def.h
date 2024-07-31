/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-18    MacRsh       First version
 * @date 2024-03-24    MacRsh       Updated v1.0.0
 */

#ifndef __MR_DEF_H__
#define __MR_DEF_H__

#include <include/mr_config.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Version
 * @{
 */

#define MR_VERSION                      ("1.0.0")                                   /** Version (X.Y.Z) */

/** @} */

/**
 * @addtogroup Compiler
 * @{
 */

#if defined(__CC_ARM) || defined(__GNUC__) || defined(__clang__)
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_UNUSED                       __attribute__((unused))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#elif defined(__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)                   @x
#define MR_UNUSED
#define MR_USED                         __root
#define MR_WEAK                         __weak
#define MR_INLINE                       static inline
#else
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_UNUSED                       __attribute__((unused))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#endif /* defined(__CC_ARM) || defined(__GNUC__) */

/** @} */

/**
 * @addtogroup Error
 * @{
 */

#define MR_EOK                          (0)                                         /**< No error */
#define MR_EPERM                        (-1)                                        /**< Operation not permitted */
#define MR_ENOENT                       (-2)                                        /**< No such file or directory */
#define MR_EIO                          (-5)                                        /**< I/O error */
#define MR_EAGAIN                       (-11)                                       /**< Resource temporarily unavailable */
#define MR_ENOMEM                       (-12)                                       /**< Out of memory */
#define MR_EACCES                       (-13)                                       /**< Permission denied */
#define MR_EBUSY                        (-16)                                       /**< Resource busy */
#define MR_EEXIST                       (-17)                                       /**< Resource exists */
#define MR_EINVAL                       (-22)                                       /**< Invalid argument */
#define MR_ENOSYS                       (-38)                                       /**< Not supported */
#define MR_ETIMEOUT                     (-110)                                      /**< Operation timed */

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_DEF_H__ */
