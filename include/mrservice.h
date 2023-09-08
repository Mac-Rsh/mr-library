/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _MR_SERVICE_H_
#define _MR_SERVICE_H_

#include "mrdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def Debug
 */
#if (MR_CFG_DEBUG_ASSERT == MR_CFG_ENABLE)
#define MR_DEBUG_A(TAG, FORMAT, ...)    mr_log_output(0, TAG, FORMAT, ##__VA_ARGS__)
#else
#define MR_DEBUG_A(TAG, FORMAT, ...)
#endif
#if (MR_CFG_DEBUG_ERROR == MR_CFG_ENABLE)
#define MR_DEBUG_E(TAG, FORMAT, ...)    mr_log_output(1, TAG, FORMAT, ##__VA_ARGS__)
#else
#define MR_DEBUG_E(TAG, FORMAT, ...)
#endif
#if (MR_CFG_DEBUG_WARNING == MR_CFG_ENABLE)
#define MR_DEBUG_W(TAG, FORMAT, ...)    mr_log_output(2, TAG, FORMAT, ##__VA_ARGS__)
#else
#define MR_DEBUG_W(TAG, FORMAT, ...)
#endif
#if (MR_CFG_DEBUG_INFO == MR_CFG_ENABLE)
#define MR_DEBUG_I(TAG, FORMAT, ...)    mr_log_output(3, TAG, FORMAT, ##__VA_ARGS__)
#else
#define MR_DEBUG_I(TAG, FORMAT, ...)
#endif
#if (MR_CFG_DEBUG_DEBUG == MR_CFG_ENABLE)
#define MR_DEBUG_D(TAG, FORMAT, ...)    mr_log_output(4, TAG, FORMAT, ##__VA_ARGS__)
#else
#define MR_DEBUG_D(TAG, FORMAT, ...)
#endif

#if (MR_CFG_DEBUG_ASSERT == MR_CFG_ENABLE)
/**
 * @def Assert
 */
#define MR_ASSERT(EX)                           \
    do{                                         \
        if (!(EX))                              \
        {                                       \
          mr_assert_handle(__FILE__, __LINE__); \
        }                                       \
    }while(0)
#else
#define MR_ASSERT(EX)
#endif

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
 * @brief This macro function gets the number of elements in an array.
 *
 * @param array The array.
 *
 * @return The number of elements in the array.
 */
#define mr_array_number_of(array)    sizeof(array)/sizeof(array[0])

/**
 * @brief This macro function aligns down a size.
 *
 * @param size The size to align.
 * @param align The alignment.
 *
 * @return The aligned size.
 */
#define mr_align_down(size, align)    (size & (~(align - 1)))

/**
 * @brief This macro function gets the maximum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The maximum of the two values.
 */
#define mr_max_of(a, b)    ((a) > (b)) ? (a):(b)

/**
 * @brief This macro function gets the minimum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The minimum of the two values.
 */
#define mr_min_of(a, b)    ((a) < (b)) ? (a):(b)

/**
 * @brief This macro function gets the limit of a value.
 *
 * @param x The value.
 * @param a The lower limit.
 * @param b The upper limit.
 *
 * @return The limit of the value.
 */
#define mr_limit_of(x, a, b)    do{ (x) = ((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x); }while(0)

/**
 * @brief This function initialize a single list.
 *
 * @param list The list to initialize.
 */
MR_INLINE void mr_slist_init(mr_slist_t list)
{
    list->next = NULL;
}

/**
 * @brief This function append a node to a single list.
 *
 * @param list The list to append to.
 * @param node The node to append.
 */
MR_INLINE void mr_slist_append(mr_slist_t list, mr_slist_t node)
{
    mr_slist_t temp_node = NULL;

    temp_node = list;
    while (temp_node->next != NULL)
    {
        temp_node = temp_node->next;
    }

    temp_node->next = node;
    node->next = NULL;
}

/**
 * @brief This function insert a node after a node in a single list.
 *
 * @param list The list to insert after.
 * @param node The node to insert.
 */
MR_INLINE void mr_slist_insert_after(mr_slist_t list, mr_slist_t node)
{
    node->next = list->next;
    list->next = node;
}

/**
 * @brief This function remove a node from a single list.
 *
 * @param list The list to remove from.
 * @param node The node to remove.
 */
MR_INLINE void mr_slist_remove(mr_slist_t list, mr_slist_t node)
{
    mr_slist_t temp_node = NULL;

    temp_node = list;
    while (temp_node->next != NULL && temp_node->next != node)
    {
        temp_node = temp_node->next;
    }

    if (temp_node->next != NULL)
    {
        temp_node->next = temp_node->next->next;
    }
}

/**
 * @brief This function get the length of a single list.
 *
 * @param list The list to get the length of.
 *
 * @return The length of the list.
 */
MR_INLINE mr_size_t mr_slist_get_length(mr_slist_t list)
{
    mr_slist_t temp_node = NULL;
    size_t length = 0;

    temp_node = list;
    while (temp_node->next != NULL)
    {
        temp_node = temp_node->next;
        length++;
    }

    return length;
}

/**
 * @brief This function get the tail of a single list.
 *
 * @param list The list to get the tail of.
 *
 * @return A pointer to the tail of the list.
 */
MR_INLINE mr_slist_t mr_slist_get_tail(mr_slist_t list)
{
    mr_slist_t temp_node = NULL;

    while (temp_node->next != NULL)
    {
        temp_node = temp_node->next;
    }

    return temp_node;
}

/**
 * @brief This function check if a single list is empty.
 *
 * @param list The list to check if it is empty.
 *
 * @return Whether the list is empty.
 */
MR_INLINE mr_bool_t mr_slist_is_empty(mr_slist_t list)
{
    return (mr_bool_t)(list->next == NULL);
}

/**
 * @brief This function initialize a double list.
 *
 * @param list The list to initialize.
 */
MR_INLINE void mr_list_init(mr_list_t list)
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
MR_INLINE void mr_list_insert_after(mr_list_t list, mr_list_t node)
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
MR_INLINE void mr_list_insert_before(mr_list_t list, mr_list_t node)
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
MR_INLINE void mr_list_remove(mr_list_t node)
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
MR_INLINE mr_size_t mr_list_get_length(mr_list_t list)
{
    mr_size_t length = 0;
    mr_list_t temp_list = list;

    while (temp_list->next != list)
    {
        temp_list = temp_list->next;
        length++;
    }

    return length;
}

/**
 * @brief This function check if a double list is empty.
 *
 * @param list The list to check if it is empty.
 *
 * @return Whether the list is empty.
 */
MR_INLINE mr_bool_t mr_list_is_empty(mr_list_t list)
{
    return (mr_bool_t)(list->next == list);
}

#ifdef __cplusplus
}
#endif

#endif /* _MR_SERVICE_H_ */