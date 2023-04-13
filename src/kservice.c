/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-13     MacRsh       first version
 */

#include <mrlib.h>

static mr_int8_t mr_avl_get_height(mr_avl_t node)
{
	if (node == NULL)
		return - 1;

	return node->height;
}

static mr_int8_t mr_avl_get_balance(mr_avl_t node)
{
	if (node == NULL)
		return 0;

	return mr_avl_get_height(node->left_child) - mr_avl_get_height(node->right_child);
}

static void mr_avl_left_rotate(mr_avl_t *node)
{
	mr_avl_t right_child = (*node)->right_child;

	(*node)->right_child = right_child->left_child;
	right_child->left_child = (*node);

	(*node)->height = mr_max(mr_avl_get_height((*node)->left_child), mr_avl_get_height((*node)->right_child)) + 1;
	right_child->height =
			mr_max(mr_avl_get_height(right_child->left_child), mr_avl_get_height(right_child->right_child)) + 1;

	(*node) = right_child;
}

static void mr_avl_right_rotate(mr_avl_t *node)
{
	mr_avl_t left_child = (*node)->left_child;

	(*node)->left_child = left_child->right_child;
	left_child->right_child = (*node);

	(*node)->height = mr_max(mr_avl_get_height((*node)->left_child), mr_avl_get_height((*node)->right_child)) + 1;
	left_child->height =
			mr_max(mr_avl_get_height(left_child->left_child), mr_avl_get_height(left_child->right_child)) + 1;

	(*node) = left_child;
}

void mr_avl_init(mr_avl_t node, mr_uint32_t value)
{
	node->height = 0;
	node->value = value;
	node->left_child = MR_NULL;
	node->right_child = MR_NULL;
}

void mr_avl_insert(mr_avl_t *tree, mr_avl_t node)
{
	mr_int8_t balance = 0;

	if ((*tree) == MR_NULL)
		(*tree) = node;

	if (node->value < (*tree)->value)
		mr_avl_insert(&(*tree)->left_child, node);
	else if (node->value > (*tree)->value)
		mr_avl_insert(&(*tree)->right_child, node);
	else
		return;

	(*tree)->height = mr_max(mr_avl_get_height((*tree)->left_child), mr_avl_get_height((*tree)->right_child)) + 1;

	balance = mr_avl_get_balance((*tree));
	if (balance > 1 && node->value < (*tree)->left_child->value)
	{
		mr_avl_right_rotate(&(*tree));
		return;
	}

	if (balance < - 1 && node->value > (*tree)->right_child->value)
	{
		mr_avl_left_rotate(&(*tree));
		return;
	}

	if (balance > 1 && node->value > (*tree)->left_child->value)
	{
		mr_avl_left_rotate(&(*tree)->left_child);
		mr_avl_right_rotate(&(*tree));
		return;
	}

	if (balance < - 1 && node->value < (*tree)->right_child->value)
	{
		mr_avl_right_rotate(&(*tree)->right_child);
		mr_avl_left_rotate(&(*tree));
		return;
	}
}

mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value)
{
	if (tree->value == value)
		return tree;

	if (value < tree->value)
		return mr_avl_find(tree->left_child, value);
	else if (value > tree->value)
		return mr_avl_find(tree->right_child, value);

	return MR_NULL;
}