/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#include "include/mr_atomic.h"

#if defined(__CC_ARM) || defined(__GNUC__) || defined(__clang__)

/**
 * @brief This function loads the value of the atomic.
 *
 * @param self The atomic.
 *
 * @return The value of the atomic variable.
 */
MR_IMPL(Atomic, load, int)
{
    MR_ASSERT(self != NULL);

    return __atomic_load_n(&self->_val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function stores the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 */
MR_IMPL(Atomic, store, void, int val)
{
    MR_ASSERT(self != NULL);

    __atomic_store_n(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function adds the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_add, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_fetch_add(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function subtracts the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_sub, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_fetch_sub(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function and the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_and, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_fetch_and(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function or the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_or, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_fetch_or(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function xor the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_xor, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_fetch_xor(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function exchanges the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, exchange, int, int val)
{
    MR_ASSERT(self != NULL);

    return __atomic_exchange_n(&self->_val, val, __ATOMIC_SEQ_CST);
}

/**
 * @brief This function compares the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, compare_exchange, bool, int old, int val)
{
    MR_ASSERT(self != NULL);
    
    return __atomic_compare_exchange_n(&self->_val, &old, val, false,
                                       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#else

/**
 * @brief This function loads the value of the atomic.
 *
 * @param self The atomic.
 *
 * @return The value of the atomic variable.
 */
MR_IMPL(Atomic, load, int)
{
    size_t mask;
    int val;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Read value */
    val = self->_val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return val;
}

/**
 * @brief This function stores the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 */
MR_IMPL(Atomic, store, void, int val)
{
    size_t mask;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Write value */
    self->_val = val;

    /* Enable interrupt */
    mr_irq_enable(mask);
}

/**
 * @brief This function adds the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_add, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and add */
    fetch = self->_val;
    self->_val += val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function subtracts the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_sub, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and sub */
    fetch = self->_val;
    self->_val -= val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function and the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_and, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and and */
    fetch = self->_val;
    self->_val &= val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function or the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_or, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and or */
    fetch = self->_val;
    self->_val |= val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function xor the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, fetch_xor, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and xor */
    fetch = self->_val;
    self->_val ^= val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function exchange the value of the atomic.
 *
 * @param self The atomic.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, exchange, int, int val)
{
    size_t mask;
    int fetch;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Fetch and exchange */
    fetch = self->_val;
    self->_val = val;

    /* Enable interrupt */
    mr_irq_enable(mask);
    return fetch;
}

/**
 * @brief This function compare and exchange the value of the atomic.
 *
 * @param self The atomic.
 * @param old The old value of the atomic variable.
 * @param val The value of the atomic variable.
 *
 * @return The value of the fetched atomic variable.
 */
MR_IMPL(Atomic, compare_exchange, bool, int old, int val)
{
    size_t mask;
    bool ret;

    MR_ASSERT(self != NULL);

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Compare and exchange */
    if (self->_val == old)
    {
        self->_val = val;
        mr_irq_enable(mask);
        ret = true;
    } else
    {
        mr_irq_enable(mask);
        ret = false;
    }

    /* Enable interrupt */
    mr_irq_enable(mask);
    return ret;
}

#endif /* defined(__CC_ARM) || defined(__GNUC__) || defined(__clang__) */

/**
 * @brief This function initializes the atomic.
 * 
 * @param self The atomic.
 * @param val The value of the atomic variable.
 * 
 * @return The initialized atomic.
 */
MR_IMPL(Atomic, __init__, struct Atomic *, int val)
{
    MR_IMPL_FUNCTION(Atomic, load, store, fetch_add, fetch_sub, fetch_and,
                     fetch_or, fetch_xor, exchange, compare_exchange);

    MR_ASSERT(self != NULL);

    /* Initialize atomic */
    self->fn = &fn;
    self->_val = val;
    return self;
}

/**
 * @brief This function destroys the atomic.
 * 
 * @param self The atomic.
 * 
 * @return The destroyed atomic.
 */
MR_IMPL(Atomic, __del__, void *)
{
    MR_ASSERT(self != NULL);

    self->_val = 0;
    return self;
}

/* Implement Atomic class */
MR_IMPL_CLASS(Atomic);
