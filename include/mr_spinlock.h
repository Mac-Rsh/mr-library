/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#ifndef __MR_SPINLOCK_H__
#define __MR_SPINLOCK_H__

#include <include/mr_atomic.h>
#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Spinlock
 * @{
 */

typedef mr_atomic_t                         mr_spinlock_t;                                              /**< Spinlock type */

/**
 * @brief This macro function initializes the spinlock.
 */
#define MR_SPIN_LOCK_INIT()                                                                        \
    {                                                                                              \
        .lock = 0                                                                                  \
    }

/**
 * @brief This function initializes the spinlock.
 *
 * @param lock The spinlock.
 */
MR_INLINE void mr_spinlock_init(mr_spinlock_t *lock)
{
    mr_atomic_store(lock, 0);
}

/**
 * @brief This function locks the spinlock.
 *
 * @param lock The spinlock.
 */
MR_INLINE void mr_spinlock_lock(mr_spinlock_t *lock)
{
    while (mr_atomic_exchange(lock, 1) != 0);
}

/**
 * @brief This function unlocks the spinlock.
 *
 * @param lock The spinlock.
 */
MR_INLINE void mr_spinlock_unlock(mr_spinlock_t *lock)
{
    mr_atomic_store(lock, 0);
}

/**
 * @brief This function tries to lock the spinlock.
 *
 * @param lock The spinlock.
 *
 * @return True if the lock was successful acquired, false otherwise.
 */
MR_INLINE bool mr_spinlock_trylock(mr_spinlock_t *lock)
{
    mr_atomic_t unlock;

    /* Try to lock */
    return mr_atomic_compare_exchange_strong(lock, &unlock, 1);
}

/**
 * @brief This function save and disable interrupt and lock spinlock.
 *
 * @param lock The spinlock.
 *
 * @return Saved interrupt mask.
 */
MR_INLINE size_t mr_spinlock_irqsave(mr_spinlock_t *lock)
{
    size_t mask;

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Lock spinlock */
    mr_spinlock_lock(lock);

    /* Return saved mask */
    return mask;
}

/**
 * @brief This function enable interrupt and unlock spinlock.
 *
 * @param lock The spinlock.
 * @param mask Saved interrupt mask.
 */
MR_INLINE void mr_spinlock_irqrestore(mr_spinlock_t *lock, size_t mask)
{
    /* Unlock spinlock */
    mr_spinlock_unlock(lock);

    /* Enable mask */
    mr_irq_enable(mask);
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_SPINLOCK_H__ */
