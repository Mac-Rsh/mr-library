/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#ifndef __MR_LIST_H__
#define __MR_LIST_H__

#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup List
 * @{
 */

/**
 * @brief Double list structure.
 */
struct mr_list
{
    struct mr_list *next;                                                                               /**< Point to next nod */
    struct mr_list *prev;                                                                               /**< Point to prev nod */
};

/**
 * @brief This macro function initializes a double list.
 *
 * @param _list The list to initialize.
 */
#define MR_LIST_INIT(_list)                                                                        \
    {                                                                                              \
        (_list), (_list)                                                                           \
    }

/**
 * @brief This macro function get the entry of a double list.
 *
 * @param _ptr The pointer to the list.
 * @param _type The type of the list.
 * @param _member The member of the list.
 */
#define MR_LIST_ENTRY(_ptr, _type, _member)                                                        \
    ((_type *)((char *)(_ptr) - (uintptr_t)(&((_type *)0)->_member)))

/**
 * @brief This macro function iterates through a double list.
 *
 * @param _pos The position of the list.
 * @param _head The head of the list.
 */
#define MR_LIST_FOR_EACH(_pos, _head)                                                              \
    for ((_pos) = (_head)->next; (_pos) != (_head); (_pos) = _pos->next)

/**
 * @brief This macro function iterates through a double list in safe mode.
 *
 * @param _pos The position of the list.
 * @param _n The next position of the list.
 * @param _head The head of the list.
 */
#define MR_LIST_FOR_EACH_SAFE(_pos, _n, _head)                                                     \
    for ((_pos) = (_head)->next, (_n) = (_pos)->next; (_pos) != (_head);                           \
         (_pos) = (_n), (_n) = (_pos)->next)

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
 * @brief This function prepend a node to a double list.
 *
 * @param list The list to prepend.
 * @param node The node to prepend.
 */
MR_INLINE void mr_list_prepend(struct mr_list *list, struct mr_list *node)
{
    list->next->prev = node;
    node->next = list->next;
    list->next = node;
    node->prev = list;
}

/**
 * @brief This function append a node to a double list.
 *
 * @param list The list to append.
 * @param node The node to append.
 */
MR_INLINE void mr_list_append(struct mr_list *list, struct mr_list *node)
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
 * @brief This function pop a node from the front of a double list.
 *
 * @param list The list to pop from.
 *
 * @return The node that was popped.
 */
MR_INLINE struct mr_list *mr_list_pop_front(struct mr_list *list)
{
    struct mr_list *node;

    /* Get the first node in the list */
    node = list->next;
    if (node == NULL)
    {
        return NULL;
    }

    /* Remove the node */
    mr_list_remove(node);
    return node;
}

/**
 * @brief This function pop a node from the back of a double list.
 *
 * @param list The list to pop from.
 *
 * @return The node that was popped.
 */
MR_INLINE struct mr_list *mr_list_pop_back(struct mr_list *list)
{
    struct mr_list *node;

    /* Get the last node in the list */
    node = list->prev;
    if (node == NULL)
    {
        return NULL;
    }

    /* Remove the node */
    mr_list_remove(node);
    return node;
}

/**
 * @brief This function reverse a double list.
 *
 * @param list The list to reverse.
 */
MR_INLINE void mr_list_reverse(struct mr_list *list)
{
    struct mr_list *node, *next;

    /* Get the first node in the list */
    node = list->next;
    list->next = list->prev;
    list->prev = node;

    /* Reverse the nodes */
    while (node != list)
    {
        next = node->next;
        node->next = node->prev;
        node->prev = next;
        node = next;
    }
}

/**
 * @brief This function concat two double lists.
 *
 * @param list1 The first list to concat.
 * @param list2 The second list to concat.
 */
MR_INLINE void mr_list_concat(struct mr_list *list1, struct mr_list *list2)
{
    list1->next->prev = list2->prev;
    list2->prev->next = list1->next;
    list1->next = list2;
    list2->prev = list1;
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
    struct mr_list *node;
    size_t len;

    /* Get the length of the list */
    node = list->next;
    len = 0;
    while (node->next != list)
    {
        node = node->next;
        len += 1;
    }
    return len;
}

/**
 * @brief This function checks if a double list is empty.
 *
 * @param list The list to check.
 *
 * @return True if the list is empty, False otherwise.
 */
MR_INLINE int mr_list_is_empty(const struct mr_list *list)
{
    return list->next == list;
}

/**
 * @brief This function checks if a list is inited.
 *
 * @param list The list to check.
 *
 * @return True if the list is inited, False otherwise.
 */
MR_INLINE int mr_list_is_inited(const struct mr_list *list)
{
    return (list->next != NULL) && (list->prev != NULL);
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_LIST_H__ */
