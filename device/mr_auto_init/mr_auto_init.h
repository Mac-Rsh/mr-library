/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-01     MacRsh       first version
 */

#ifndef MR_AUTO_INIT_H_
#define MR_AUTO_INIT_H_

/* Compiler Related Definitions */
#if defined(__ARMCC_VERSION)        /* ARM Compiler */
#define mr_section(x)               __attribute__((section(x)))
#define mr_used                     __attribute__((used))
#define mr_align(n)                 __attribute__((aligned(n)))
#define mr_weak                     __attribute__((weak))
#define mr_inline                   static __inline
#elif defined (__IAR_SYSTEMS_ICC__) /* for IAR Compiler */
#define mr_section(x)               @ x
#define mr_used                     __root
#define PRAGMA(x)                   _Pragma(#x)
#define mr_align(n)                    PRAGMA(data_alignment=n)
#define mr_weak                     __weak
#define mr_inline                   static inline
#elif defined (__GNUC__)            /* GNU GCC Compiler */
#define mr_section(x)               __attribute__((section(x)))
#define mr_used                     __attribute__((used))
#define mr_align(n)                 __attribute__((aligned(n)))
#define mr_weak                     __attribute__((weak))
#define mr_inline                   static __inline
#elif defined (__ADSPBLACKFIN__)    /* for VisualDSP++ Compiler */
#define mr_section(x)               __attribute__((section(x)))
#define mr_used                     __attribute__((used))
#define mr_align(n)                 __attribute__((aligned(n)))
#define mr_weak                     __attribute__((weak))
#define mr_inline                   static inline
#elif defined (_MSC_VER)
#define mr_section(x)
#define mr_used
#define mr_align(n)                 __declspec(align(n))
#define mr_weak
#define mr_inline                   static __inline
#elif defined (__TASKING__)
#define mr_section(x)               __attribute__((section(x)))
#define mr_used                     __attribute__((used, protect))
#define mrAGMA(x)                   _Pragma(#x)
#define mr_align(n)                 __attribute__((__align(n)))
#define mr_weak                     __attribute__((weak))
#define mr_inline                   static inline
#endif  /* end of __ARMCC_VERSION */

typedef int (*init_fn_t)(void);
#define INIT_EXPORT(fn,level) \
    mr_used const init_fn_t _mr_init_##fn mr_section(".mri_fn."level) = fn

/**
 *  Export auto-init functions
 */
#define INIT_BOARD_EXPORT(fn)        INIT_EXPORT(fn, "1")
#define INIT_DEVICE_EXPORT(fn)       INIT_EXPORT(fn, "2")
#define INIT_ENV_EXPORT(fn)          INIT_EXPORT(fn, "3")
#define INIT_APP_EXPORT(fn)          INIT_EXPORT(fn, "4")
void mr_auto_init(void);

#endif /* end of MR_AUTO_INIT_H_ */
