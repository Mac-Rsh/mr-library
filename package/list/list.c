/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-25     MacRsh       first version
 */

#include "list.h"

/**
 * @brief Initialize a single list.
 *
 * @param list The list to initialize.
 */
void slist_init(slist_t list)
{
	LIST_ASSERT(list != NULL);

	list->next = NULL;
}

/**
 * @brief Append a node to a single list.
 *
 * @param list The list to append to.
 * @param node The node to append.
 */
void slist_append(slist_t list, slist_t node)
{
	slist_t temp_node = NULL;

	LIST_ASSERT(list != NULL);
	LIST_ASSERT(node != NULL);

	temp_node = list;
	while (temp_node->next != NULL)
	{
		temp_node = temp_node->next;
	}

	temp_node->next = node;
	node->next = NULL;
}

/**
 * @brief Insert a node after a node in a single list.
 *
 * @param list The list to insert after.
 * @param node The node to insert.
 */
void slist_insert_after(slist_t list, slist_t node)
{
	LIST_ASSERT(list != NULL);
	LIST_ASSERT(node != NULL);

	node->next = list->next;
	list->next = node;
}

/**
 * @brief Remove a node from a single list.
 *
 * @param list The list to remove from.
 * @param node The node to remove.
 */
void slist_remove(slist_t list, slist_t node)
{
	slist_t temp_node = NULL;

	LIST_ASSERT(list != NULL);
	LIST_ASSERT(node != NULL);

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
 * @brief Get the length of a single list.
 *
 * @param list The list to get the length of.
 *
 * @return The length of the list.
 */
size_t slist_get_length(slist_t list)
{
	slist_t temp_node = NULL;
	size_t length = 0;

	LIST_ASSERT(list != NULL);

	temp_node = list;
	while (temp_node->next != NULL)
	{
		temp_node = temp_node->next;
		length ++;
	}

	return length;
}

/**
 * @brief Get the tail of a single list.
 *
 * @param list The list to get the tail of.
 *
 * @return A handle to the tail of the list.
 */
slist_t slist_get_tail(slist_t list)
{
	slist_t temp_node = NULL;

	LIST_ASSERT(list != NULL);

	while (temp_node->next != NULL)
	{
		temp_node = temp_node->next;
	}

	return temp_node;
}

/**
 * @brief Check if a single list is empty.
 *
 * @param list The list to check if it is empty.
 *
 * @return Whether the list is empty.
 */
int slist_is_empty(slist_t list)
{
	LIST_ASSERT(list != NULL);

	return list->next == NULL;
}

/**
 * @brief Initialize a double list.
 *
 * @param list The list to initialize.
 */
void list_init(list_t list)
{
	LIST_ASSERT(list != NULL);

	list->next = list;
	list->prev = list;
}

/**
 * @brief Insert a node after a node in a double list.
 *
 * @param list The list to insert after.
 * @param node The node to insert.
 */
void list_insert_after(list_t list, list_t node)
{
	LIST_ASSERT(list != NULL);
	LIST_ASSERT(node != NULL);

	list->next->prev = node;
	node->next = list->next;

	list->next = node;
	node->prev = list;
}

/**
 * @brief Insert a node before a node in a double list.
 *
 * @param list The list to insert before.
 * @param node The node to insert.
 */
void list_insert_before(list_t list, list_t node)
{
	LIST_ASSERT(list != NULL);
	LIST_ASSERT(node != NULL);

	list->prev->next = node;
	node->prev = list->prev;

	list->prev = node;
	node->next = list;
}

/**
 * @brief Remove a node from a double list.
 *
 * @param node The node to remove.
 */
void list_remove(list_t node)
{
	LIST_ASSERT(node != NULL);

	node->next->prev = node->prev;
	node->prev->next = node->next;

	node->next = node->prev = node;
}

/**
 * @brief Get the length of a double list.
 *
 * @param list The list to get the length of.
 *
 * @return The length of the list.
 */
size_t list_get_length(list_t list)
{
	list_t temp_node = NULL;
	size_t length = 0;

	LIST_ASSERT(list != NULL);

	temp_node = list;
	while (temp_node->next != list)
	{
		temp_node = temp_node->next;
		length ++;
	}

	return length;
}

/**
 * @brief Check if a double list is empty.
 *
 * @param list The list to check if it is empty.
 *
 * @return Whether the list is empty.
 */
int list_is_empty(list_t list)
{
	LIST_ASSERT(list != NULL);

	return list->next == list;
}