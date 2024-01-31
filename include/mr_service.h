/*
 * @copyright (c) 2023-2024, MR Development Team
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
 * @addtogroup Basic
 * @{
 */

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
 * @brief This macro function gets its structure from its member.
 *
 * @param pointer The pointer to the structure.
 * @param type The type of the structure.
 * @param member The member of the structure.
 *
 * @return A pointer to the structure.
 */
#define MR_CONTAINER_OF(pointer, type, member) \
    ((type *)((char *)(pointer) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief This macro function aligns a value upwards.
 *
 * @param value The value to align.
 * @param align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_UP(value, align)       (((value) + (align) - 1) & ~((align) - 1))

/**
 * @brief This macro function aligns a value downwards.
 *
 * @param value The value to align.
 * @param align The alignment.
 *
 * @return The aligned value.
 */
#define MR_ALIGN_DOWN(value, align)     ((value) & ~((align) - 1))

/**
 * @brief This macro function checks if a value is set.
 *
 * @param value The value to check.
 * @param mask The mask to check.
 */
#define MR_BIT_IS_SET(value, mask)      (((value) & (mask)) == (mask))

/**
 * @brief This macro function sets a value.
 *
 * @param value The value to set.
 * @param mask The mask to set.
 */
#define MR_BIT_SET(value, mask)         ((value) |= (mask))

/**
 * @brief This macro function clears a value.
 *
 * @param value The value to clear.
 * @param mask The mask to clear.
 */
#define MR_BIT_CLR(value, mask)         ((value) &= ~(mask))

/**
 * @brief This macro function creates a local variable.
 *
 * @param type The type of the variable.
 * @param ... The arguments.
 *
 * @return A pointer to the variable.
 *
 * @note The variable is local, please use it carefully.
 */
#define MR_MAKE_LOCAL(type, ...)        (&((type){__VA_ARGS__}))

/**
 * @brief This macro function gets the number of elements in an array.
 *
 * @param array The array.
 *
 * @return The number of elements in the array.
 */
#define MR_ARRAY_NUM(array)             (sizeof(array)/sizeof((array)[0]))

/**
 * @brief This macro function gets the maximum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The maximum of the two values.
 */
#define MR_MAX(a, b)                    ({typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })

/**
 * @brief This macro function gets the minimum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The minimum of the two values.
 */
#define MR_MIN(a, b)                    ({typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param value The value.
 * @param min The minimum value.
 * @param max The maximum value.
 *
 * @return The value within the specified range.
 */
#define MR_BOUND(value, min, max) \
    ({__typeof__(value) _value = (value); __typeof__(min) _min = (min); __typeof__(max) _max = (max); \
    (_value) < (_min) ? (_min) : ((_value) > (_max) ? (_max) : (_value));})

/**
 * @brief This macro function ensures that a value is within a specified range.
 *
 * @param value The value.
 * @param min The minimum value.
 * @param max The maximum value.
 */
#define MR_LIMIT(value, min, max)       (value) = MR_BOUND(value, min, max)

/**
 * @brief This macro function swaps two values.
 *
 * @param a The first value.
 * @param b The second value.
 */
#define MR_SWAP(a, b)                   do { typeof (a) temp = (a); (a) = (b); (b) = temp; } while (0)

/**
 * @brief This macro function converts a value to a boolean.
 *
 * @param value The value to convert.
 *
 * @return The boolean value.
 */
#define MR_TO_BOOL(value)               (!!(value))
/** @} */

/**
 * @addtogroup Log
 * @{
 */

/**
 * @brief Log message with color.
 */
#ifdef MR_USING_LOG_COLOR
#define MR_LOG_COLOR_RED(str, fmt)      "\033[31m"str, fmt"\033[0m" /**< Log red color */
#define MR_LOG_COLOR_YELLOW(str, fmt)   "\033[33m"str, fmt"\033[0m" /**< Log yellow color */
#define MR_LOG_COLOR_BLUE(str, fmt)     "\033[34m"str, fmt"\033[0m" /**< Log blue color */
#define MR_LOG_COLOR_PURPLE(str, fmt)   "\033[35m"str, fmt"\033[0m" /**< Log purple color */
#define MR_LOG_COLOR_GREEN(str, fmt)    "\033[32m"str, fmt"\033[0m" /**< Log green color */
#else
#define MR_LOG_COLOR_RED(str, fmt)      str, fmt
#define MR_LOG_COLOR_YELLOW(str, fmt)   str, fmt
#define MR_LOG_COLOR_BLUE(str, fmt)     str, fmt
#define MR_LOG_COLOR_PURPLE(str, fmt)   str, fmt
#define MR_LOG_COLOR_GREEN(str, fmt)    str, fmt
#endif /* MR_USING_LOG_COLOR */

#ifdef MR_USING_LOG_ERROR
/* Print error message */
#define MR_LOG_ERROR(fmt, ...)          mr_printf("%-8s %s\r\n", MR_LOG_COLOR_RED("ERROR:", fmt), ##__VA_ARGS__)
#else
#define MR_LOG_ERROR(fmt, ...)
#endif /* MR_USING_LOG_ERROR */
#ifdef MR_USING_LOG_WARN
/* Print warning message */
#define MR_LOG_WARN(fmt, ...)           mr_printf("%-8s %s\r\n", MR_LOG_COLOR_YELLOW("WARNING:", fmt), ##__VA_ARGS__)
#else
#define MR_LOG_WARN(fmt, ...)
#endif /* MR_USING_LOG_WARN */
#ifdef MR_USING_LOG_INFO
/* Print information message */
#define MR_LOG_INFO(fmt, ...)           mr_printf("%-8s %s\r\n", MR_LOG_COLOR_BLUE("INFO:", fmt), ##__VA_ARGS__)
#else
#define MR_LOG_INFO(fmt, ...)
#endif /* MR_USING_LOG_INFO */
#ifdef MR_USING_LOG_DEBUG
/* Print debug message */
#define MR_LOG_DEBUG(fmt, ...)          mr_printf("%-8s %s\r\n", MR_LOG_COLOR_PURPLE("DEBUG:", fmt), ##__VA_ARGS__)
#else
#define MR_LOG_DEBUG(fmt, ...)
#endif /* MR_USING_LOG_DEBUG */
#ifdef MR_USING_LOG_SUCCESS
/* Print success message */
#define MR_LOG_SUCCESS(fmt, ...)        mr_printf("%-8s %s\r\n", MR_LOG_COLOR_GREEN("SUCCESS:", fmt), ##__VA_ARGS__)
#else
#define MR_LOG_SUCCESS(fmt, ...)
#endif /* MR_USING_LOG_SUCCESS */
/** @} */

/**
 * @addtogroup Assert
 * @{
 */

/**
 * @brief This macro function asserts a condition.
 *
 * @param ex The condition to assert.
 */
#ifdef MR_USING_ASSERT
#define MR_ASSERT(ex)                   \
    do                                  \
    {                                   \
        if (!(ex))                      \
        {                               \
            mr_printf("%-8s "           \
                      "failed: %s, "    \
                      "function: %s, "  \
                      "file: %s, "      \
                      "line: %d.\r\n",  \
                      "ASSERT:",        \
                      #ex,              \
                      (__FUNCTION__),   \
                      (__FILE__),       \
                      (__LINE__));      \
            while(1);                   \
        }                               \
    } while(0)
#else
#define MR_ASSERT(ex)
#endif /* MR_USING_ASSERT */
/** @} */

/**
 * @addtogroup List
 * @{
 */

/**
 * @brief This macro function initializes a list.
 *
 * @param list The list to initialize.
 */
#define MR_LIST_INIT(list)              {&(list), &(list)}

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
MR_INLINE size_t mr_list_get_len(struct mr_list *list)
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
/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_SERVICE_H_ */
