/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrlib.h"

static mr_device_t console_device = MR_NULL;

static int start(void)
{
	return 0;
}
AUTO_INIT_EXPORT(start, "0");

static int driver_state(void)
{
	return 0;
}
AUTO_INIT_EXPORT(driver_state, "0.end");

static int driver_end(void)
{
	return 0;
}
AUTO_INIT_EXPORT(driver_end, "1.end");

static int end(void)
{
	return 0;
}
AUTO_INIT_EXPORT(end, "3.end");

void mr_auto_init(void)
{
	volatile const init_fn_t *fn_ptr;

	/* auto-init */
	for (fn_ptr = &_mr_auto_init_start; fn_ptr < &_mr_auto_init_end; fn_ptr ++)
	{
		(*fn_ptr)();
	}
}

mr_weak mr_size_t mr_printf_output(const char *str, mr_size_t length)
{
	return 0;
}

mr_err_t mr_printf_init(void)
{
	console_device = mr_device_find(MR_CONF_CONSOLE_NAME);
	MR_ASSERT(console_device != MR_NULL);

	return MR_ERR_OK;
}
AUTO_INIT_DEVICE_EXPORT(mr_printf_init);

mr_weak mr_size_t mr_printf(const char *fmt, ...)
{
	char str_buffer[MR_CONF_CONSOLE_BUFSZ];
	mr_size_t length = 0;

	va_list arg;
	va_start(arg, fmt);
	length = vsnprintf(str_buffer, sizeof(str_buffer) - 1, fmt, arg);
#if (MR_CONF_CONSOLE == MR_ENABLE && MR_CONF_SERIAL == MR_ENABLE)
	mr_device_write(console_device, 0, str_buffer, length);
#else
	mr_printf_output(str_buffer, length);
#endif
	va_end(arg);

	return length;
}

static mr_int8_t mr_avl_get_height(mr_avl_t node)
{
	if (node == MR_NULL)
		return - 1;

	return node->height;
}

static mr_int8_t mr_avl_get_balance(mr_avl_t node)
{
	if (node == MR_NULL)
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

void mr_avl_remove(mr_avl_t *tree, mr_avl_t node)
{

}

mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value)
{
	if (tree == MR_NULL)
		return tree;

	if (tree->value == value)
		return tree;

	if (value < tree->value)
		return mr_avl_find(tree->left_child, value);
	else if (value > tree->value)
		return mr_avl_find(tree->right_child, value);

	return MR_NULL;
}

mr_size_t mr_avl_get_length(mr_avl_t tree)
{
	mr_size_t length = 1;

	if (tree == MR_NULL)
		return 0;

	if (tree->left_child != MR_NULL)
		length += mr_avl_get_length(tree->left_child);

	if (tree->right_child != MR_NULL)
		length += mr_avl_get_length(tree->right_child);

	return length;
}

mr_uint32_t mr_strhase(const char *str)
{
	mr_uint32_t value = 0;

	while (*str)
	{
		value ^= value << 15;
		value ^= value >> 10;
		value ^= *str ++;
	}
	value ^= value << 3;
	value ^= value >> 6;
	value ^= value << 2;
	value ^= value >> 15;

	return value;
}

mr_uint32_t mr_strnhase(const char *str, mr_size_t length)
{
	mr_uint32_t value = 0;

	while (length --)
	{
		value ^= value << 15;
		value ^= value >> 10;
		value ^= *str ++;
	}
	value ^= value << 3;
	value ^= value >> 6;
	value ^= value << 2;
	value ^= value >> 15;

	return value;
}