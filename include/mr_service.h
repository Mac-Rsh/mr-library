/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_SERVICE_H_
#define _MR_SERVICE_H_

#include "../mr-library/include/mr_def.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Service-Macros
 * @{
 */

/**
 * @brief This macro function concatenates two strings.
 *
 * @param _a The first string.
 * @param _b The second string.
 *
 * @return The concatenated string.
 */
#define MR_CONCAT(_a, _b)               _a##_b

/**
 * @brief This macro function converts an integer to a string.
 *
 * @param _a The integer to convert.
 *
 * @return The string representation of the integer.
 */
#define MR_STR(_a)                      #_a

/**
 * @brief This macro function gets its structure from its member.
 *
 * @param _pointer The pointer to the structure.
 * @param _type The type of the structure.
 * @param _member The member of the structure.
 *
 * @return A pointer to the structure.
 */
#define MR_CONTAINER_OF(_pointer, _type, _member)                              \
    ((_type *)((char *)(_pointer) - (size_t)(&((_type *)0)->_member)))

/**
 * @brief This macro function aligns a value upwards.
 *
 * @param _value The value to align.
 * @param _align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_UP(_value, _align)                                            \
    (((_value) + (_align)-1) & ~((_align)-1))

/**
 * @brief This macro function aligns a value downwards.
 *
 * @param _value The value to align.
 * @param _align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_DOWN(_value, _align)   ((_value) & ~((_align)-1))

/**
 * @brief This macro function checks if a value is set.
 *
 * @param _value The value to check.
 * @param _mask The mask to check.
 */
#define MR_BIT_IS_SET(_value, _mask)    (((_value) & (_mask)) == (_mask))

/**
 * @brief This macro function sets a value.
 *
 * @param _value The value to set.
 * @param _mask The mask to set.
 */
#define MR_BIT_SET(_value, _mask)       ((_value) |= (_mask))

/**
 * @brief This macro function clears a value.
 *
 * @param _value The value to clear.
 * @param _mask The mask to clear.
 */
#define MR_BIT_CLR(_value, _mask)       ((_value) &= ~(_mask))

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
#define MR_MAKE_LOCAL(_type, ...)       (&((_type){__VA_ARGS__}))

/**
 * @brief This macro function gets the number of elements in an array.
 *
 * @param _array The array.
 *
 * @return The number of elements in the array.
 */
#define MR_ARRAY_NUM(_array)            (sizeof(_array) / sizeof((_array)[0]))

/**
 * @brief This macro function gets the maximum of two values.
 *
 * @param _a The first value.
 * @param _b The second value.
 * @param _type The type of the values.
 *
 * @return The maximum of the two values.
 */
#define MR_MAX(_a, _b, _type)                                                  \
    ({                                                                         \
        _type __a = (_a);                                                      \
        _type __b = (_b);                                                      \
        __a > __b ? __a : __b;                                                 \
    })

/**
 * @brief This macro function gets the minimum of two values.
 *
 * @param _a The first value.
 * @param _b The second value.
 * @param _type The type of the values.
 *
 * @return The minimum of the two values.
 */
#define MR_MIN(_a, _b, _type)                                                  \
    ({                                                                         \
        _type __a = (_a);                                                      \
        _type __b = (_b);                                                      \
        __a < __b ? __a : __b;                                                 \
    })

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param _value The value.
 * @param _min The minimum value.
 * @param _max The maximum value.
 *
 * @return The value within the specified range.
 */
#define MR_BOUND(_value, _min, _max)                                           \
    ({ (_value) < (_min) ? (_min) : ((_value) > (_max) ? (_max) : (_value)); })

/**
 * @brief This macro function swaps two values.
 *
 * @param _a The first value.
 * @param _b The second value.
 * @param _type The type of the values.
 */
#define MR_SWAP(_a, _b, _type)                                                 \
    do                                                                         \
    {                                                                          \
        _type _tmp = (_a);                                                     \
        (_a) = (_b);                                                           \
        (_b) = _tmp;                                                           \
    } while (0)

/**
 * @brief This macro function converts a value to a boolean.
 *
 * @param _value The value to convert.
 *
 * @return The boolean value.
 */
#define MR_TO_BOOL(_value)              (!!(_value))

/** @} */

/**
 * @addtogroup Log
 * @{
 */

#define MR_LOG_TAG                      ("null")

#ifdef MR_USE_LOG_ERROR
#define MR_LOG_E(_fmt, ...)                                                    \
    mr_log_printf(MR_LOG_TAG, "[E/%s] "_fmt, MR_LOG_TAG, ##__VA_ARGS__)
#else
#define MR_LOG_E(_fmt, ...)
#endif /* MR_USE_LOG_ERROR */
#ifdef MR_USE_LOG_WARN
#define MR_LOG_W(_fmt, ...)                                                    \
    mr_log_printf(MR_LOG_TAG, "[W/%s] "_fmt, MR_LOG_TAG, ##__VA_ARGS__)
#else
#define MR_LOG_W(_fmt, ...)
#endif /* MR_USE_LOG_WARN */
#ifdef MR_USE_LOG_INFO
#define MR_LOG_I(_fmt, ...)                                                    \
    mr_log_printf(MR_LOG_TAG, "[I/%s] "_fmt, MR_LOG_TAG, ##__VA_ARGS__)
#else
#define MR_LOG_I(_fmt, ...)
#endif /* MR_USE_LOG_INFO */
#ifdef MR_USE_LOG_DEBUG
#define MR_LOG_D(_fmt, ...)                                                    \
    mr_log_printf(MR_LOG_TAG, "[D/%s] "_fmt, MR_LOG_TAG, ##__VA_ARGS__)
#else
#define MR_LOG_D(_fmt, ...)
#endif /* MR_USE_LOG_DEBUG */

/** @} */

/**
 * @addtogroup Assert
 * @{
 */

#ifdef MR_USE_ASSERT
/**
 * @brief Conditional assertion.
 *
 * @param _ex The assert expression.
 */
#define MR_ASSERT(_ex)                                                         \
    do                                                                         \
    {                                                                          \
        if (!(_ex))                                                            \
        {                                                                      \
            mr_assert_handler(#_ex, MR_LOG_TAG, (__FUNCTION__), (__FILE__),    \
                              (__LINE__));                                     \
        }                                                                      \
    } while (0)
#else
#define MR_ASSERT(_ex)
#endif /* MR_USE_ASSERT */

/** @} */

/**
 * @addtogroup List
 * @{
 */

/**
 * @brief This macro function initializes a list.
 *
 * @param _list The list to initialize.
 */
#define MR_LIST_INIT(_list)                                                    \
    {                                                                          \
        (_list), (_list)                                                       \
    }

/**
 * @brief This function initialize a double list.
 *
 * @param list The list to initialize.
 */
MR_INLINE void mr_list_init(struct mr_list *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * @brief This function insert a node after a node in a double list.
 *
 * @param list The list to insert after.
 * @param node The node to insert.
 */
MR_INLINE void mr_list_insert_after(struct mr_list *list, struct mr_list *node)
{
    list->next->prev = node;
    node->next = list->next;
    list->next = node;
    node->prev = list;
}

/**
 * @brief This function insert a node before a node in a double list.
 *
 * @param list The list to insert before.
 * @param node The node to insert.
 */
MR_INLINE void mr_list_insert_before(struct mr_list *list, struct mr_list *node)
{
    list->prev->next = node;
    node->prev = list->prev;
    list->prev = node;
    node->next = list;
}

/**
 * @brief This function remove a node from a double list.
 *
 * @param node The node to remove.
 */
MR_INLINE void mr_list_remove(struct mr_list *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = node->prev = node;
}

/**
 * @brief This function get the length of a double list.
 *
 * @param list The list to get the length of.
 *
 * @return The length of the list.
 */
MR_INLINE size_t mr_list_len_get(struct mr_list *list)
{
    struct mr_list *node = list;
    size_t len = 0;

    while (node->next != list)
    {
        node = node->next;
        len++;
    }
    return len;
}

/**
 * @brief This function checks if a list is empty.
 *
 * @param list The list to check.
 *
 * @return True if the list is empty, mr_false otherwise.
 */
MR_INLINE int mr_list_is_empty(struct mr_list *list)
{
    return list->next == list;
}

/**
 * @brief This function checks if a list is inited.
 *
 * @param list The list to check.
 *
 * @return True if the list is inited, mr_false otherwise.
 */
MR_INLINE int mr_list_is_inited(struct mr_list *list)
{
    return (list->next != NULL) && (list->prev != NULL);
}

/** @} */

/**
 * @addtogroup Device
 * @{
 */

/**
 * @brief This macro function gets the device read operator.
 *
 * @param _device The device.
 */
#define _MR_DEVICE_OPERATOR_RD_GET(_device)                                    \
    ((((_device)->lock & _MR_OPERATE_MASK_RD) >> 16) - 1)

/**
 * @brief This macro function gets the device write operator.
 *
 * @param _device The device.
 */
#define _MR_DEVICE_OPERATOR_WR_GET(_device)                                    \
    (((_device)->lock & _MR_OPERATE_MASK_WR) - 1)

/**
 * @brief This macro function gets the device operator.
 *
 * @param _device The device.
 *
 * @note This feature is only available for non-full-duplex devices.
 */
#define _MR_DEVICE_OPERATOR_GET(_device)                                       \
    (((_device)->lock & _MR_OPERATE_MASK_WR) - 1)

/**
 * @brief This macro function sets the device read operator.
 *
 * @param _device The device.
 * @param _operator The operator.
 */
#define _MR_DEVICE_OPERATOR_RD_SET(_device, _operator)                         \
    (((_device)->lock =                                                        \
          ((_device)->lock & ~_MR_OPERATE_MASK_RD) | ((_operator + 1) << 16)))

/**
 * @brief This macro function sets the device write operator.
 *
 * @param _device The device.
 * @param _operator The operator.
 */
#define _MR_DEVICE_OPERATOR_WR_SET(_device, _operator)                         \
    (((_device)->lock =                                                        \
          ((_device)->lock & ~_MR_OPERATE_MASK_WR) | (_operator + 1)))

/**
 * @brief This macro function sets the device operator.
 *
 * @param _device The device.
 * @param _operator The operator.
 *
 * @note This feature is only available for non-full-duplex devices.
 */
#define _MR_DEVICE_OPERATOR_SET(_device, _operator)                            \
    (_MR_DEVICE_OPERATOR_RD_SET(_device, _operator),                           \
     _MR_DEVICE_OPERATOR_WR_SET(_device, _operator))

/**
 * @brief This macro function clears the device operator.
 *
 * @param _device The device.
 */
#define _MR_DEVICE_OPERATOR_RD_CLR(_device)                                    \
    _MR_DEVICE_OPERATOR_RD_SET(_device, -1)

/**
 * @brief This macro function clears the device operator.
 *
 * @param _device The device.
 */
#define _MR_DEVICE_OPERATOR_WR_CLR(_device)                                    \
    _MR_DEVICE_OPERATOR_WR_SET(_device, -1)

/**
 * @brief This macro function clears the device operator.
 *
 * @param _device The device.
 *
 * @note This feature is only available for non-full-duplex devices.
 */
#define _MR_DEVICE_OPERATOR_CLR(_device)                                       \
    _MR_DEVICE_OPERATOR_SET(_device, -1)

/**
 * @brief This macro function gets the device parent.
 *
 * @param _device The device.
 *
 * @return The device parent.
 */
#define _MR_DEVICE_PARENT_GET(_device)  ((void *)((_device)->parent))

/**
 * @brief This macro function gets the device driver.
 *
 * @param _device The device.
 *
 * @return The device driver.
 */
#define _MR_DEVICE_DRIVER_GET(_device)  ((void *)((_device)->driver))

/**
 * @brief This macro function gets the driver ops.
 *
 * @param _driver The driver.
 *
 * @return The driver ops.
 */
#define _MR_DRIVER_OPS_GET(_driver)                                            \
    ((void *)(((struct mr_driver *)(_driver))->ops))

/**
 * @brief This macro function gets the driver data.
 *
 * @param _driver The driver.
 *
 * @return The driver data.
 */
#define _MR_DRIVER_DATA_GET(_driver)                                           \
    ((void *)(((struct mr_driver *)(_driver))->data))

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERVICE_H_ */
