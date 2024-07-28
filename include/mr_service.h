/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef __MR_SERVICE_H__
#define __MR_SERVICE_H__

#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Interrupt
 * @{
 */

size_t mr_irq_disable(void);
void mr_irq_enable(size_t mask);

/** @} */

/**
 * @addtogroup Initialization
 * @{
 */

/**
 * @brief Initialization item structure.
 * 
 */
struct mr_init_item
{
    const char *name;                                                                                   /**< Name */
    int (*fn)(void);                                                                                    /**< Function */
};

#define __MR_INIT_EXPORT(_name, _fn, _level)                                                       \
    MR_USED MR_SECTION("mr_init." _level) const struct mr_init_item _mr_init_item_##_fn            \
        = {(_name), (_fn)}                                                                              /**< Initialization item export */

/**
 * @brief This macro exports initialization items to the board level.
 * 
 * @param _name The name of the initialization item.
 * @param _fn The function of the initialization item.
 */
#define MR_INIT_BOARD_EXPORT(_name, _fn)    __MR_INIT_EXPORT(_name, _fn, "1")

/**
 * @brief This macro exports initialization items to the system level.
 * 
 * @param _name The name of the initialization item.
 * @param _fn The function of the initialization item.
 */
#define MR_INIT_SYSTEM_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "2")

/**
 * @brief This macro exports initialization items to the driver level.
 * 
 * @param _name The name of the initialization item.
 * @param _fn The function of the initialization item.
 */
#define MR_INIT_DRIVER_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "3")

/**
 * @brief This macro exports initialization items to the device level.
 * 
 * @param _name The name of the initialization item.
 * @param _fn The function of the initialization item.
 */
#define MR_INIT_DEVICE_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "4")

/**
 * @brief This macro exports initialization items to the application level.
 * 
 * @param _name The name of the initialization item.
 * @param _fn The function of the initialization item.
 */
#define MR_INIT_APP_EXPORT(_name, _fn)      __MR_INIT_EXPORT(_name, _fn, "5")

/** @} */

/**
 * @addtogroup Service-Macros
 * @{
 */

/**
 * @brief This macro function aligns a value upwards.
 *
 * @param _value The value to align.
 * @param _align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_UP(_value, _align)         (((_value) + (_align) - 1) & ~((_align) - 1))

/**
 * @brief This macro function aligns a value downwards.
 *
 * @param _value The value to align.
 * @param _align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_DOWN(_value, _align)       ((_value) & ~((_align) - 1))

/**
 * @brief This macro gets the number of elements in an array.
 *
 * @param _array The array.
 *
 * @return The number of elements in the array.
 *
 * @note The array must be an array, not a pointer.
 */
#define MR_ARRAY_NUM(_array)                (sizeof(_array) / sizeof((_array)[0]))

/**
 * @brief This macro function checks if a value is set.
 *
 * @param _value The value to check.
 * @param _mask The mask to check.
 */
#define MR_BIT_IS_SET(_value, _mask)        (((_value) & (_mask)) == (_mask))

/**
 * @brief This macro function sets a value.
 *
 * @param _value The value to set.
 * @param _mask The mask to set.
 */
#define MR_BIT_SET(_value, _mask)           ((_value) |= (_mask))

/**
 * @brief This macro function clears a value.
 *
 * @param _value The value to clear.
 * @param _mask The mask to clear.
 */
#define MR_BIT_CLR(_value, _mask)           ((_value) &= ~(_mask))

/**
 * @brief This macro function gets the maximum of two values.
 *
 * @param _a The first value.
 * @param _b The second value.
 *
 * @return The maximum of the two values.
 */
#define MR_MAX(_a, _b)                      ((_a) > (_b) ? (_a) : (_b))

/**
 * @brief This macro function gets the minimum of two values.
 *
 * @param _a The first value.
 * @param _b The second value.
 *
 * @return The minimum of the two values.
 */
#define MR_MIN(_a, _b)                      ((_a) < (_b) ? (_a) : (_b))

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param _min The minimum value.
 * @param _max The maximum value.
 *
 * @return The value within the specified range.
 */
#define MR_CLAMP(_value, _min, _max)        (MR_MAX(MR_MIN((_value), (_max)), (_min)))

/**
 * @brief This macro function swaps two values.
 *
 * @param _type The type of the value.
 * @param _a The first value.
 * @param _b The second value.
 */
#define MR_SWAP(_type, _a, _b)                                                                     \
    do                                                                                             \
    {                                                                                              \
        _type __a = (_a);                                                                          \
        (_a) = (_b);                                                                               \
        (_b) = __a;                                                                                \
    } while (0)

/**
 * @brief This macro concatenates two strings.
 *
 * @param _a The first string.
 * @param _b The second string.
 *
 * @return The concatenated string.
 */
#define MR_CONCAT(_a, _b)                   _a##_b

/**
 * @brief This macro converts an integer to a string.
 *
 * @param _a The integer to convert.
 *
 * @return The string representation of the integer.
 */
#define MR_STR(_a)                          #_a

/**
 * @brief This macro function gets its structure from its member.
 *
 * @param _ptr The pointer to the structure.
 * @param _type The type of the structure.
 * @param _member The member of the structure.
 *
 * @return A pointer to the structure.
 */
#define MR_CONTAINER_OF(_ptr, _type, _member)                                                      \
    ((_type *)((char *)(_ptr) - (uintptr_t)(&((_type *)0)->_member)))

/**
 * @brief This macro function creates a local variable.
 *
 * @param _type The type of the variable.
 * @param ... The arguments.
 *
 * @return A pointer to the variable.
 *
 * @note The variable is local, please use it carefully.
 */
#define MR_LOCAL_MAKE(_type, ...)           (&((_type){__VA_ARGS__}))

/**
 * @brief This macro function asserts a condition.
 * 
 * @param _cond The condition to assert.
 */
#define MR_ASSERT(_cond)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(_cond))                                                                              \
        {                                                                                          \
            while (1);                                                                             \
        }                                                                                          \
    } while (0)

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_SERVICE_H__ */
