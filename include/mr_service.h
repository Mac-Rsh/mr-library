/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_SERVICE_H_
#define _MR_SERVICE_H_

#include "mr_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief This macro function asserts a condition.
 *
 * @param ex The condition to assert.
 */
#ifdef MR_USING_ASSERT
#define mr_assert(ex)                   \
    do{                                 \
        if (!(ex))                      \
        {                               \
            mr_printf("assert > "       \
                      "failed: %s, "    \
                      "file: %s, "      \
                      "line: %d.\r\n",  \
                      #ex,              \
                      (__FUNCTION__),   \
                      (__LINE__));      \
            while(1);                   \
        }                               \
    } while(0)
#else
#define mr_assert(ex)
#endif /* MR_USING_ASSERT */

/**
 * @brief This macro function logs a message.
 *
 * @param fmt The format of the message.
 * @param ... The arguments of the format.
 */
#define mr_log(level, fmt, ...)         \
    do{                                 \
        mr_printf("log %s > "           \
                  fmt"\r\n",            \
                  level,                \
                  ##__VA_ARGS__);       \
    } while(0)

/**
 * @brief This macro function logs a error-warning-debug-info message.
 */
#ifdef MR_USING_LOG_ERROR
#define mr_log_error(fmt, ...)          mr_log("error", fmt, ##__VA_ARGS__)
#else
#define mr_log_error(fmt, ...)
#endif /* MR_USING_LOG_ERROR */
#ifdef MR_USING_LOG_WARN
#define mr_log_warn(fmt, ...)           mr_log("warn", fmt, ##__VA_ARGS__)
#else
#define mr_log_warn(fmt, ...)
#endif /* MR_USING_LOG_WARN */
#ifdef MR_USING_LOG_INFO
#define mr_log_info(fmt, ...)           mr_log("info", fmt, ##__VA_ARGS__)
#else
#define mr_log_info(fmt, ...)
#endif /* MR_USING_LOG_INFO */
#ifdef MR_USING_LOG_DEBUG
#define mr_log_debug(fmt, ...)          mr_log("debug", fmt, ##__VA_ARGS__)
#else
#define mr_log_debug(fmt, ...)
#endif /* MR_USING_LOG_DEBUG */

/**
 * @brief This macro function gets its structure from its member.
 *
 * @param pointer The pointer to the structure.
 * @param type The type of the structure.
 * @param member The member of the structure.
 *
 * @return A pointer to the structure.
 */
#define mr_container_of(pointer, type, member) \
    ((type *)((char *)(pointer) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief This macro function checks if a value is set.
 *
 * @param value The value to check.
 * @param mask The mask to check.
 */
#define mr_bits_is_set(value, mask)     (((value) & (mask)) == (mask))

/**
 * @brief This macro function sets a value.
 *
 * @param value The value to set.
 * @param mask The mask to set.
 */
#define mr_bits_set(value, mask)        ((value) |= (mask))

/**
 * @brief This macro function clears a value.
 *
 * @param value The value to clear.
 * @param mask The mask to clear.
 */
#define mr_bits_clr(value, mask)        ((value) &= ~(mask))

/**
 * @brief This macro function gets the number of elements in an array.
 *
 * @param array The array.
 *
 * @return The number of elements in the array.
 */
#define mr_array_num(array)             (sizeof(array)/sizeof((array)[0]))

/**
 * @brief This macro function creates a local variable.
 *
 * @param type The type of the variable.
 * @param value The value of the variable.
 *
 * @return A pointer to the variable.
 */
#define mr_make_local(type, ...)        (&((type){__VA_ARGS__}))

/**
 * @brief This macro function gets the maximum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The maximum of the two values.
 */
#define mr_max(a, b)                    ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })

/**
 * @brief This macro function gets the minimum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The minimum of the two values.
 */
#define mr_min(a, b)                    ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param value The value.
 * @param min The minimum value.
 * @param max The maximum value.
 *
 * @return The value within the specified range.
 */
#define mr_bound(value, min, max) \
    ({__typeof__(value) _value = (value); __typeof__(min) _min = (min); __typeof__(max) _max = (max); \
    (_value) < (_min) ? (_min) : ((_value) > (_max) ? (_max) : (_value));})

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param value The value.
 * @param min The minimum value.
 * @param max The maximum value.
 *
 * @return The value within the specified range.
 */
#define mr_limit(value, min, max)       mr_bound(value, min, max)

/**
 * @brief This macro function swaps two values.
 *
 * @param a The first value.
 * @param b The second value.
 */
#define mr_swap(a, b)                   do { typeof (a) temp = a; a = b; b = temp; } while (0)

/**
 * @brief This macro function aligns the size up to a multiple of 4.
 *
 * @param size The size to align.
 */
#define mr_align4_up(size)              (((size) + 3) & (~3))

/**
 * @brief This macro function aligns a size down to a multiple of 4.
 *
 * @param size The size to align.
 */
#define mr_align4_down(size)            ((size) & (~3))

/**
 * @brief This macro function concatenates two strings.
 *
 * @param a The first string.
 * @param b The second string.
 *
 * @return The concatenated string.
 */
#define MR_CONCAT(a, b)                 a##b

/**
 * @brief This macro function converts an integer to a string.
 *
 * @param a The integer to convert.
 *
 * @return The string representation of the integer.
 */
#define MR_STR(a)                       #a

/**
 * @brief This macro function checks if a list is empty.
 *
 * @param list The list to check.
 *
 * @return True if the list is empty, mr_false otherwise.
 */
#define mr_list_is_empty(list)          (((list)->next) == (list))

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
MR_INLINE size_t mr_list_get_length(struct mr_list *list)
{
    size_t length = 0;
    struct mr_list *node = list;

    while (node->next != list)
    {
        node = node->next;
        length++;
    }

    return length;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERVICE_H_ */
