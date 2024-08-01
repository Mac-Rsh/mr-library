/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#ifndef __MR_ATOMIC_H__
#define __MR_ATOMIC_H__

#include <include/mr_class.h>
#include <include/mr_def.h>
#include <include/mr_service.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Atomic
 * @{
 */

MR_CLASS(Atomic)
{
    MR_FUNCTION(Atomic)
    {
        int (*load)(struct Atomic *self);
        void (*store)(struct Atomic *self, int val);
        int (*fetch_add)(struct Atomic *self, int val);
        int (*fetch_sub)(struct Atomic *self, int val);
        int (*fetch_and)(struct Atomic *self, int val);
        int (*fetch_or)(struct Atomic *self, int val);
        int (*fetch_xor)(struct Atomic *self, int val);
        int (*exchange)(struct Atomic *self, int val);
        bool (*compare_exchange)(struct Atomic *self, int old, int new);
    } *fn;
    int _val;
};

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_ATOMIC_H__ */
