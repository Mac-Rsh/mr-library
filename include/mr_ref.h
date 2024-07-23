/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#ifndef __MR_REF_H__
#define __MR_REF_H__

#include <include/mr_atomic.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Reference
 * @{
 */

/**
 * @brief This macro function initializes a reference.
 *
 * @param _release The release function.
 */
#define MR_REF_INIT(_release)                                                                      \
    {                                                                                              \
        .ref_count = 1, .release = (_release)                                                      \
    }

/**
 * @brief This function initialize a reference.
 *
 * @param ref The reference to initialize.
 */
MR_INLINE void mr_ref_init(mr_ref_t *ref)
{
    /* Initialize reference count */
    mr_atomic_store(&ref->count, 1);
}

/**
 * @brief This function get a reference.
 *
 * @param ref The reference to get.
 */
MR_INLINE void mr_ref_get(mr_ref_t *ref)
{
    /* Increment reference count */
    mr_atomic_fetch_add(&ref->count, 1);
}

/**
 * @brief This function put a reference.
 *
 * @param ref The reference to put.
 * @param release The release function.
 *
 * @return The release result.
 */
MR_INLINE bool mr_ref_put(mr_ref_t *ref, void (*release)(mr_ref_t *))
{
    /* Decrement reference count */
    if (mr_atomic_fetch_sub(&ref->count, 1) == 1)
    {
        /* If reference count will be 0, release it */
        if (release != NULL)
        {
            release(ref);
        }
        return true;
    }

    /* Reference count is not 0 */
    return false;
}

/**
 * @brief This function get the reference count.
 *
 * @param ref The reference to get.
 *
 * @return The reference count.
 */
MR_INLINE mr_atomic_t mr_ref_get_count(mr_ref_t *ref)
{
    /* Get reference count */
    return ref->count;
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_REF_H__ */
