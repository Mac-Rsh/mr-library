/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#ifndef __MR_ATOMIC_H__
#define __MR_ATOMIC_H__

#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Atomic
 * @{
 */

#if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#define mr_atomic_load(_ptr)                                                                       \
    ((typeof(*(_ptr)))__atomic_load_n((_ptr), __ATOMIC_SEQ_CST))
#define mr_atomic_store(_ptr, _var)                                                                \
    (__atomic_store_n((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_fetch_add(_ptr, _var)                                                            \
    ((typeof(*(_ptr)))__atomic_fetch_add((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_fetch_sub(_ptr, _var)                                                            \
    ((typeof(*(_ptr)))__atomic_fetch_sub((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_fetch_and(_ptr, _var)                                                            \
    ((typeof(*(_ptr)))__atomic_fetch_and((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_fetch_or(_ptr, _var)                                                             \
    ((typeof(*(_ptr)))__atomic_fetch_or((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_fetch_xor(_ptr, _var)                                                            \
    ((typeof(*(_ptr)))__atomic_fetch_xor((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_exchange(_ptr, _var)                                                             \
    ((typeof(*(_ptr)))__atomic_exchange_n((_ptr), (_var), __ATOMIC_SEQ_CST))
#define mr_atomic_compare_exchange_strong(_ptr, _old, _new)                                        \
    ((bool)__atomic_compare_exchange_n((_ptr), (_old), (_new), 0, __ATOMIC_SEQ_CST,                \
                                       __ATOMIC_SEQ_CST))
#else
/**
 * @brief This macro function loads the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 *
 * @return The value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_load(volatile mr_atomic_t *ptr)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value */
    tmp = *(mr_atomic_t *)ptr;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function stores the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 */
MR_INLINE void mr_atomic_store(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Write value */
    *ptr = val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
}

/**
 * @brief This macro function adds the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_fetch_add(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and add */
    tmp = *ptr;
    *ptr += val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function subtracts the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_fetch_sub(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and sub */
    tmp = *ptr;
    *ptr -= val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function ands the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_fetch_and(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and and */
    tmp = *ptr;
    *ptr &= val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function ors the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_fetch_or(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and or */
    tmp = *ptr;
    *ptr |= val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function xors the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_fetch_xor(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and xor */
    tmp = *ptr;
    *ptr ^= val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function exchanges the value of the atomic variable.
 *
 * @param ptr The address of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The old value of the atomic variable.
 */
MR_INLINE mr_atomic_t mr_atomic_exchange(volatile mr_atomic_t *ptr, mr_atomic_t val)
{
    mr_atomic_t tmp;
    size_t mask;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Read value and exchange */
    tmp = *ptr;
    *ptr = val;

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}

/**
 * @brief This macro function compares and exchanges the value of the atomic
 *        variable.
 *
 * @param ptr The address of the atomic variable.
 * @param old The old value of the atomic variable.
 * @param new The new value of the atomic variable.
 *
 * @return The result of the comparison and exchange.
 */
MR_INLINE bool mr_atomic_compare_exchange_strong(volatile mr_atomic_t *ptr, mr_atomic_t *old,
                                                 mr_atomic_t new)
{
    size_t mask;
    bool ret;

    /* Disable interrupt */
    mask = mr_interrupt_disable();

    /* Compare and exchange */
    if (*ptr == *old)
    {
        *ptr = new;
        ret = 1;
    } else
    {
        ret = 0;
    }

    /* Enable interrupt */
    mr_interrupt_enable(mask);
    return tmp;
}
#endif /* defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM) */

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_ATOMIC_H__ */
