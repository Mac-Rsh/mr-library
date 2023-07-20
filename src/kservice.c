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

static const char *debug_level_name[] =
        {
                "Debug-assert",
                "Debug-error",
                "Debug-warning",
                "Debug-info",
                "Debug-debug",
        };

static int start(void)
{
    return 0;
}
AUTO_INIT_EXPORT(start, "0");

static int driver_start(void)
{
    return 0;
}
AUTO_INIT_EXPORT(driver_start, "0.end");

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

/**
 * @brief This function is auto initialization macro derived functions.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_auto_init(void)
{
    volatile const init_fn_t *fn_ptr;

    /* Auto-initialization */
    for (fn_ptr = &_mr_auto_init_start; fn_ptr < &_mr_auto_init_end; fn_ptr++)
    {
        (*fn_ptr)();
    }

    return MR_ERR_OK;
}

mr_err_t mr_printf_init(void)
{
#if (MR_CONF_CONSOLE == MR_ENABLE && MR_CONF_SERIAL == MR_ENABLE)
    console_device = mr_device_find(MR_CONF_CONSOLE_NAME);
    MR_ASSERT(console_device != MR_NULL);
    return mr_device_open(console_device, MR_OPEN_RDWR | MR_OPEN_NONBLOCKING);
#else
    console_device = MR_NULL;
    return MR_ERR_OK;
#endif
}
AUTO_INIT_DEVICE_EXPORT(mr_printf_init);

mr_weak mr_size_t mr_printf_output(const char *str, mr_size_t size)
{
    return 0;
}

mr_size_t mr_printf(const char *format, ...)
{
    char str_buffer[MR_CONF_CONSOLE_BUFSZ];
    va_list args;
    mr_size_t size = 0;

    va_start(args, format);
    size = mr_vsnprintf(str_buffer, sizeof(str_buffer) - 1, format, args);
    if (console_device != MR_NULL)
    {
        mr_device_write(console_device, 0, str_buffer, size);
    } else
    {
        mr_printf_output(str_buffer, size);
    }
    va_end(args);

    return size;
}

void mr_log_output(mr_base_t level, const char *tag, const char *format, ...)
{
    char str_buffer[MR_CONF_CONSOLE_BUFSZ];
    va_list args;
    mr_size_t size = 0;

    va_start(args, format);
    mr_printf("[%s/%s]: ", debug_level_name[level], tag);
    size = mr_vsnprintf(str_buffer, sizeof(str_buffer) - 1, format, args);
#if (MR_CONF_CONSOLE == MR_ENABLE && MR_CONF_SERIAL == MR_ENABLE)
    if (console_device != MR_NULL)
    {
        mr_device_write(console_device, 0, str_buffer, size);
    }
#else
        mr_printf_output(str_buffer, size);
#endif
    va_end(args);
}

mr_weak void mr_assert_handle(void)
{
    while (1)
    {

    }
}

mr_weak void mr_interrupt_disable(void)
{

}

mr_weak void mr_interrupt_enable(void)
{

}

mr_weak void mr_delay_us(mr_size_t us)
{
    volatile mr_size_t count = 0;

    for (count = 0; count < us * (BSP_SYSCLK_FREQ / 1000000u); count++)
    {
        asm("nop");
    }
}

mr_weak void mr_delay_ms(mr_size_t ms)
{
    mr_delay_us(ms * 1000u);
}

/**
 * @brief This function initialize the fifo.
 *
 * @param fifo The fifo to initialize.
 * @param pool The pool of data.
 * @param pool_size The size of the pool.
 */
void mr_fifo_init(mr_fifo_t fifo, void *pool, mr_size_t pool_size)
{
    MR_ASSERT(fifo != MR_NULL);
    MR_ASSERT((pool != MR_NULL || pool_size == 0));

    fifo->read_index = 0;
    fifo->write_index = 0;
    fifo->read_mirror = 0;
    fifo->write_mirror = 0;

    fifo->size = pool_size;
    fifo->buffer = pool;
}

/**
 * @brief This function reset the fifo.
 *
 * @param fifo The fifo to reset.
 */
void mr_fifo_reset(mr_fifo_t fifo)
{
    MR_ASSERT(fifo != MR_NULL);

    fifo->read_index = 0;
    fifo->write_index = 0;

    fifo->read_mirror = 0;
    fifo->write_mirror = 0;
}

/**
 * @brief This function get the data size from the fifo.
 *
 * @param fifo The fifo to get the data size.
 *
 * @return The data size.
 */
mr_size_t mr_fifo_get_data_size(mr_fifo_t fifo)
{
    MR_ASSERT(fifo != MR_NULL);

    /* Empty or full according to the mirror flag */
    if (fifo->read_index == fifo->write_index)
    {
        if (fifo->read_mirror == fifo->write_mirror)
        {
            return 0;
        } else
        {
            return fifo->size;
        }
    }

    if (fifo->write_index > fifo->read_index)
    {
        return fifo->write_index - fifo->read_index;
    } else
    {
        return fifo->size - fifo->read_index + fifo->write_index;
    }
}

/**
 * @brief This function get the space size from the fifo.
 *
 * @param fifo The fifo to get the space size.
 *
 * @return The space size.
 */
mr_size_t mr_fifo_get_space_size(mr_fifo_t fifo)
{
    MR_ASSERT(fifo != MR_NULL);

    return fifo->size - mr_fifo_get_data_size(fifo);
}

/**
 * @brief This function get the buffer size from the fifo.
 *
 * @param fifo The fifo to get the buffer size.
 *
 * @return  The buffer size.
 */
mr_size_t mr_fifo_get_buffer_size(mr_fifo_t fifo)
{
    MR_ASSERT(fifo != MR_NULL);

    return fifo->size;
}

/**
 * @brief This function reads from the fifo.
 *
 * @param fifo The fifo to be read.
 * @param buffer The data buffer to be read from the fifo.
 * @param size The size of the read.
 *
 * @return The size of the actual read.
 */
mr_size_t mr_fifo_read(mr_fifo_t fifo, void *buffer, mr_size_t size)
{
    mr_uint8_t *buf = (mr_uint8_t *)buffer;
    mr_size_t length = 0;

    MR_ASSERT(fifo != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the data size */
    length = mr_fifo_get_data_size(fifo);

    /* If there is no data to read, return 0 */
    if (length == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to read if it exceeds the available data */
    if (size > length)
    {
        size = length;
    }

    /* Copy the data from the fifo to the buffer */
    if ((fifo->size - fifo->read_index) > size)
    {
        mr_memcpy(buf, &fifo->buffer[fifo->read_index], size);
        fifo->read_index += size;

        return size;
    }

    mr_memcpy(buf, &fifo->buffer[fifo->read_index], fifo->size - fifo->read_index);
    mr_memcpy(&buf[fifo->size - fifo->read_index], &fifo->buffer[0], size - (fifo->size - fifo->read_index));

    fifo->read_mirror = ~fifo->read_mirror;
    fifo->read_index = size - (fifo->size - fifo->read_index);

    return size;
}

/**
 * @brief This function write the fifo.
 *
 * @param fifo The fifo to be written.
 * @param buffer The data buffer to be written to fifo.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_fifo_write(mr_fifo_t fifo, const void *buffer, mr_size_t size)
{
    mr_uint8_t *buf = (mr_uint8_t *)buffer;
    mr_size_t length = 0;

    MR_ASSERT(fifo != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the space size */
    length = mr_fifo_get_space_size(fifo);

    /* If there is no space to write, return 0 */
    if (length == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to write if it exceeds the available data */
    if (size > length)
    {
        size = length;
    }

    /* Copy the data from the buffer to the fifo */
    if ((fifo->size - fifo->write_index) > size)
    {
        mr_memcpy(&fifo->buffer[fifo->write_index], buf, size);
        fifo->write_index += size;

        return size;
    }

    mr_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
    mr_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

    fifo->write_mirror = ~fifo->write_mirror;
    fifo->write_index = size - (fifo->size - fifo->write_index);

    return size;
}

/**
 * @brief This function force write the fifo.
 *
 * @param fifo The fifo to be written.
 * @param buffer The data buffer to be written to fifo.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_fifo_write_force(mr_fifo_t fifo, const void *buffer, mr_size_t size)
{
    mr_uint8_t *buf = (mr_uint8_t *)buffer;
    mr_size_t length = 0;

    MR_ASSERT(fifo != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the space size */
    length = mr_fifo_get_space_size(fifo);

    /* If the data exceeds the buffer length, the front data is discarded */
    if (size > fifo->size)
    {
        buf = &buf[size - fifo->size];
        size = fifo->size;
    }

    /* Copy the data from the buffer to the fifo */
    if ((fifo->size - fifo->write_index) > size)
    {
        mr_memcpy(&fifo->buffer[fifo->write_index], buf, size);
        fifo->write_index += size;
        if (size > length)
        {
            fifo->read_index = fifo->write_index;
        }

        return size;
    }

    mr_memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
    mr_memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

    fifo->write_mirror = ~fifo->write_mirror;
    fifo->write_index = size - (fifo->size - fifo->write_index);

    if (size > length)
    {
        if (fifo->write_index <= fifo->read_index)
        {
            fifo->read_mirror = ~fifo->read_mirror;
        }

        fifo->read_index = fifo->write_index;
    }

    return size;
}

static mr_int8_t mr_avl_get_height(mr_avl_t node)
{
    if (node == MR_NULL)
    {
        return -1;
    }

    return node->height;
}

static mr_int8_t mr_avl_get_balance(mr_avl_t node)
{
    if (node == MR_NULL)
    {
        return 0;
    }

    return (mr_int8_t)(mr_avl_get_height(node->left_child) - mr_avl_get_height(node->right_child));
}

static void mr_avl_left_rotate(mr_avl_t *node)
{
    mr_avl_t right_child = (*node)->right_child;

    MR_ASSERT(node != MR_NULL);

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

    MR_ASSERT(node != MR_NULL);

    (*node)->left_child = left_child->right_child;
    left_child->right_child = (*node);

    (*node)->height = mr_max(mr_avl_get_height((*node)->left_child), mr_avl_get_height((*node)->right_child)) + 1;
    left_child->height =
                    mr_max(mr_avl_get_height(left_child->left_child), mr_avl_get_height(left_child->right_child)) + 1;

    (*node) = left_child;
}

void mr_avl_init(mr_avl_t node, mr_uint32_t value)
{
    MR_ASSERT(node != MR_NULL);

    node->height = 0;
    node->value = value;
    node->left_child = MR_NULL;
    node->right_child = MR_NULL;
}

void mr_avl_insert(mr_avl_t *tree, mr_avl_t node)
{
    mr_int8_t balance = 0;

    if ((*tree) == MR_NULL)
    {
        (*tree) = node;
    }

    if (node->value < (*tree)->value)
    {
        mr_avl_insert(&(*tree)->left_child, node);
    } else if (node->value > (*tree)->value)
    {
        mr_avl_insert(&(*tree)->right_child, node);
    } else
    {
        return;
    }

    (*tree)->height = mr_max(mr_avl_get_height((*tree)->left_child), mr_avl_get_height((*tree)->right_child)) + 1;

    balance = mr_avl_get_balance((*tree));
    if (balance > 1 && node->value < (*tree)->left_child->value)
    {
        mr_avl_right_rotate(&(*tree));
        return;
    }

    if (balance < -1 && node->value > (*tree)->right_child->value)
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

    if (balance < -1 && node->value < (*tree)->right_child->value)
    {
        mr_avl_right_rotate(&(*tree)->right_child);
        mr_avl_left_rotate(&(*tree));
        return;
    }
}

void mr_avl_remove(mr_avl_t *tree, mr_avl_t node)
{
    if (*tree == MR_NULL)
    {
        return;
    }

    if (node->value < (*tree)->value)
    {
        mr_avl_remove(&(*tree)->left_child, node);
    } else if (node->value > (*tree)->value)
    {
        mr_avl_remove(&(*tree)->right_child, node);
    } else
    {
        if ((*tree)->left_child == MR_NULL)
        {
            mr_avl_t temp = (*tree)->right_child;
            (*tree)->right_child = MR_NULL;
            (*tree) = temp;
            return;
        } else if ((*tree)->right_child == MR_NULL)
        {
            mr_avl_t temp = (*tree)->left_child;
            (*tree)->left_child = MR_NULL;
            (*tree) = temp;
            return;
        }

        mr_avl_t temp = (*tree)->right_child->left_child;
        (*tree)->value = temp->value;
        mr_avl_remove(&(*tree)->right_child, temp);
        return;
    }

    (*tree)->height = mr_max(mr_avl_get_height((*tree)->left_child), mr_avl_get_height((*tree)->right_child)) + 1;

    mr_int8_t balance = mr_avl_get_balance(*tree);

    if (balance > 1 && mr_avl_get_balance((*tree)->left_child) >= 0)
    {
        mr_avl_right_rotate(tree);
    }

    if (balance > 1 && mr_avl_get_balance((*tree)->left_child) < 0)
    {
        mr_avl_left_rotate(&(*tree)->left_child);
        mr_avl_right_rotate(tree);
    }

    if (balance < -1 && mr_avl_get_balance((*tree)->right_child) <= 0)
    {
        mr_avl_left_rotate(tree);
    }

    if (balance < -1 && mr_avl_get_balance((*tree)->right_child) > 0)
    {
        mr_avl_right_rotate(&(*tree)->right_child);
        mr_avl_left_rotate(tree);
    }
}

mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value)
{
    if (tree == MR_NULL)
    {
        return tree;
    }

    if (tree->value == value)
    {
        return tree;
    }

    if (value < tree->value)
    {
        return mr_avl_find(tree->left_child, value);
    } else if (value > tree->value)
    {
        return mr_avl_find(tree->right_child, value);
    }

    return MR_NULL;
}

mr_size_t mr_avl_get_length(mr_avl_t tree)
{
    mr_size_t length = 1;

    if (tree == MR_NULL)
    {
        return 0;
    }

    if (tree->left_child != MR_NULL)
    {
        length += mr_avl_get_length(tree->left_child);
    }

    if (tree->right_child != MR_NULL)
    {
        length += mr_avl_get_length(tree->right_child);
    }

    return length;
}

mr_uint32_t mr_str2hash(const char *str, mr_size_t length)
{
    mr_uint32_t hash = 2166136261u;
    mr_size_t count = 0;

    for (count = 0; count < length; count++)
    {
        hash ^= (mr_uint32_t)str[count];
        hash *= 16777619u;
    }

    return hash;
}