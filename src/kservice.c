/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrapi.h"

#if (MR_CFG_DEVICE == MR_CFG_ENABLE)
static mr_device_t console_device = MR_NULL;
#endif

#if (MR_CFG_AUTO_INIT == MR_CFG_ENABLE)

static int start(void)
{
    return 0;
}
MR_INIT_EXPORT(start, "0");

static int end(void)
{
    return 0;
}
MR_INIT_EXPORT(end, "5.end");

/**
 * @brief This function is auto initialized.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_auto_init(void)
{
    volatile const mr_init_fn_t *fn = MR_NULL;

    /* Auto-initialization */
    for (fn = &_mr_auto_init_start; fn < &_mr_auto_init_end; fn++)
    {
        (*fn)();
    }

    return MR_ERR_OK;
}

#endif

#if (MR_CFG_DEBUG == MR_CFG_ENABLE)

/**
 * @brief This function outputs debugging information.
 *
 * @param level The debug level.
 * @param tag The tag of the output.
 * @param format The format of the output.
 * @param ... The arguments of the format.
 */
void mr_log_output(mr_base_t level, const char *tag, const char *format, ...)
{
    static const char *debug_level_name[] =
        {
            "Debug-assert",
            "Debug-error",
            "Debug-warning",
            "Debug-data",
            "Debug-debug",
        };
    char buffer[256];
    mr_size_t size = 0;
    va_list args;

    if (level > 4)
    {
        return;
    }

    va_start(args, format);
    mr_printf("[%s / %s]: ", debug_level_name[level], tag);
    size = mr_vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    mr_printf_output(buffer, size);
    va_end(args);
}

/**
 * @brief This function assert the handle.
 *
 * @param file The file of the assert.
 * @param line The line of the file.
 */
MR_WEAK void mr_assert_handle(char *file, int line)
{
    while (1)
    {

    }
}

#endif

#if (MR_CFG_DEVICE == MR_CFG_ENABLE)

/**
 * @brief This function initializes the console device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_console_init(void)
{
    console_device = mr_device_find(MR_CFG_CONSOLE_NAME);
    if (console_device == MR_NULL)
    {
        return MR_ERR_NOT_FOUND;
    }

    return mr_device_open(console_device, MR_DEVICE_OFLAG_RDWR);
}
MR_INIT_DEVICE_EXPORT(mr_console_init);

#endif

/**
 * @brief This function print the output.
 *
 * @param buffer The buffer to print.
 * @param size The size of the buffer.
 *
 * @return The size of the actual write.
 */
MR_WEAK mr_size_t mr_printf_output(const char *buffer, mr_size_t size)
{
#if (MR_CFG_DEVICE == MR_CFG_ENABLE)
    if (console_device == MR_NULL)
    {
        return 0;
    }

    return mr_device_write(console_device, -1, buffer, size);
#else
    return 0;
#endif
}

mr_size_t mr_printf(const char *format, ...)
{
    char buffer[256];
    mr_size_t size = 0;
    va_list args;

    va_start(args, format);
    size = mr_vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    mr_printf_output(buffer, size);
    va_end(args);

    return size;
}

/**
 * @brief This function disable the interrupt.
 */
MR_WEAK void mr_interrupt_disable(void)
{

}

/**
 * @brief This function enable the interrupt.
 */
MR_WEAK void mr_interrupt_enable(void)
{

}

/**
 * @brief This function delay the us.
 *
 * @param us The us to delay.
 */
MR_WEAK void mr_delay_us(mr_size_t us)
{
    volatile mr_size_t tick = 0;

    while (tick < us)
    {
        tick++;
    }
}

/**
 * @brief This function delay the ms.
 *
 * @param ms The ms to delay.
 */
MR_WEAK void mr_delay_ms(mr_size_t ms)
{
    mr_delay_us(ms * 1000u);
}

/**
 * @brief This function initialize the ringbuffer.
 *
 * @param rb The ringbuffer to initialize.
 * @param pool The pool of data.
 * @param size The size of the pool.
 */
void mr_rb_init(mr_rb_t rb, void *pool, mr_size_t size)
{
    MR_ASSERT(rb != MR_NULL);
    MR_ASSERT((pool != MR_NULL || size == 0));

    rb->read_index = 0;
    rb->write_index = 0;
    rb->read_mirror = 0;
    rb->write_mirror = 0;

    rb->size = size;
    rb->buffer = pool;
}

/**
 * @brief This function allocate memory for the ringbuffer.
 *
 * @param rb The ringbuffer to allocate.
 * @param size The size of the memory.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_rb_allocate_buffer(mr_rb_t rb, mr_size_t size)
{
    mr_uint8_t *pool = MR_NULL;

    MR_ASSERT(rb != MR_NULL);

    /* Free old buffer */
    if (rb->size != 0)
    {
        mr_free(rb->buffer);
    }

    /* Allocate new buffer */
    pool = mr_malloc(size);
    if (pool == MR_NULL && size != 0)
    {
        return MR_ERR_NO_MEMORY;
    }
    mr_rb_init(rb, pool, size);

    return MR_ERR_OK;
}

/**
 * @brief This function reset the ringbuffer.
 *
 * @param rb The ringbuffer to reset.
 */
void mr_rb_reset(mr_rb_t rb)
{
    MR_ASSERT(rb != MR_NULL);

    rb->read_index = 0;
    rb->write_index = 0;

    rb->read_mirror = 0;
    rb->write_mirror = 0;
}

/**
 * @brief This function get the data size from the ringbuffer.
 *
 * @param rb The ringbuffer to get the data size.
 *
 * @return The data size.
 */
mr_size_t mr_rb_get_data_size(mr_rb_t rb)
{
    MR_ASSERT(rb != MR_NULL);

    /* Empty or full according to the mirror flag */
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
        {
            return 0;
        } else
        {
            return rb->size;
        }
    }

    if (rb->write_index > rb->read_index)
    {
        return rb->write_index - rb->read_index;
    } else
    {
        return rb->size - rb->read_index + rb->write_index;
    }
}

/**
 * @brief This function get the space size from the ringbuffer.
 *
 * @param rb The ringbuffer to get the space size.
 *
 * @return The space size.
 */
mr_size_t mr_rb_get_space_size(mr_rb_t rb)
{
    MR_ASSERT(rb != MR_NULL);

    return rb->size - mr_rb_get_data_size(rb);
}

/**
 * @brief This function get the buffer size from the ringbuffer.
 *
 * @param rb The ringbuffer to get the buffer size.
 *
 * @return  The buffer size.
 */
mr_size_t mr_rb_get_buffer_size(mr_rb_t rb)
{
    MR_ASSERT(rb != MR_NULL);

    return rb->size;
}

/**
 * @brief This function pop the data from the ringbuffer.
 *
 * @param rb The ringbuffer to pop the data.
 * @param data The data to pop.
 *
 * @return The size of the actual pop.
 */
mr_size_t mr_rb_pop(mr_rb_t rb, mr_uint8_t *data)
{
    /* Get the data size */
    if (mr_rb_get_data_size(rb) == 0)
    {
        return 0;
    }

    *data = rb->buffer[rb->read_index];

    if (rb->read_index == rb->size - 1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    } else
    {
        rb->read_index++;
    }

    return 1;
}

/**
 * @brief This function reads from the ringbuffer.
 *
 * @param rb The ringbuffer to be read.
 * @param buffer The data buffer to be read from the ringbuffer.
 * @param size The size of the read.
 *
 * @return The size of the actual read.
 */
mr_size_t mr_rb_read(mr_rb_t rb, void *buffer, mr_size_t size)
{
    mr_uint8_t *read_buffer = (mr_uint8_t *)buffer;
    mr_size_t data_size = 0;

    MR_ASSERT(rb != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    /* Get the data size */
    data_size = mr_rb_get_data_size(rb);
    if (data_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to read if it exceeds the available data */
    if (size > data_size)
    {
        size = data_size;
    }

    /* Copy the data from the rb to the buffer */
    if ((rb->size - rb->read_index) > size)
    {
        mr_memcpy(read_buffer, &rb->buffer[rb->read_index], size);
        rb->read_index += size;

        return size;
    }

    mr_memcpy(read_buffer, &rb->buffer[rb->read_index], rb->size - rb->read_index);
    mr_memcpy(&read_buffer[rb->size - rb->read_index], &rb->buffer[0], size - (rb->size - rb->read_index));

    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = size - (rb->size - rb->read_index);

    return size;
}

/**
 * @brief This function push the data to the ringbuffer.
 *
 * @param rb The ringbuffer to be pushed.
 * @param data The data to be pushed.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_rb_push(mr_rb_t rb, mr_uint8_t data)
{
    /* Get the space size */
    if (mr_rb_get_space_size(rb) == 0)
    {
        return 0;
    }

    rb->buffer[rb->write_index] = data;

    if (rb->write_index == rb->size - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    } else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * @brief This function force to push the data to the ringbuffer.
 *
 * @param rb The ringbuffer to be pushed.
 * @param data The data to be pushed.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_rb_push_force(mr_rb_t rb, mr_uint8_t data)
{
    mr_bool_t state = MR_FALSE;

    /* Get the space size */
    if (mr_rb_get_space_size(rb) == 0)
    {
        state = MR_TRUE;
    }

    rb->buffer[rb->write_index] = data;

    if (rb->write_index == rb->size - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (state == MR_TRUE)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    } else
    {
        rb->write_index++;
        if (state == MR_TRUE)
        {
            rb->read_index = rb->write_index;
        }
    }

    return 1;
}

/**
 * @brief This function write the ringbuffer.
 *
 * @param rb The ringbuffer to be written.
 * @param buffer The data buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_rb_write(mr_rb_t rb, const void *buffer, mr_size_t size)
{
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_size_t space_size = 0;

    MR_ASSERT(rb != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    /* Get the space size */
    space_size = mr_rb_get_space_size(rb);
    if (space_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to write if it exceeds the available data */
    if (size > space_size)
    {
        size = space_size;
    }

    /* Copy the data from the buffer to the rb */
    if ((rb->size - rb->write_index) > size)
    {
        mr_memcpy(&rb->buffer[rb->write_index], write_buffer, size);
        rb->write_index += size;

        return size;
    }

    mr_memcpy(&rb->buffer[rb->write_index], write_buffer, rb->size - rb->write_index);
    mr_memcpy(&rb->buffer[0], &write_buffer[rb->size - rb->write_index], size - (rb->size - rb->write_index));

    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = size - (rb->size - rb->write_index);

    return size;
}

/**
 * @brief This function force write the ringbuffer.
 *
 * @param rb The ringbuffer to be written.
 * @param buffer The data buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
mr_size_t mr_rb_write_force(mr_rb_t rb, const void *buffer, mr_size_t size)
{
    mr_uint8_t *write_buffer = (mr_uint8_t *)buffer;
    mr_size_t space_size = 0;

    MR_ASSERT(rb != MR_NULL);
    MR_ASSERT(buffer != MR_NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the space size */
    space_size = mr_rb_get_space_size(rb);

    /* If the data exceeds the buffer space_size, the front data is discarded */
    if (size > rb->size)
    {
        write_buffer = &write_buffer[size - rb->size];
        size = rb->size;
    }

    /* Copy the data from the buffer to the rb */
    if ((rb->size - rb->write_index) > size)
    {
        mr_memcpy(&rb->buffer[rb->write_index], write_buffer, size);
        rb->write_index += size;
        if (size > space_size)
        {
            rb->read_index = rb->write_index;
        }

        return size;
    }

    mr_memcpy(&rb->buffer[rb->write_index], write_buffer, rb->size - rb->write_index);
    mr_memcpy(&rb->buffer[0], &write_buffer[rb->size - rb->write_index], size - (rb->size - rb->write_index));

    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = size - (rb->size - rb->write_index);

    if (size > space_size)
    {
        if (rb->write_index <= rb->read_index)
        {
            rb->read_mirror = ~rb->read_mirror;
        }

        rb->read_index = rb->write_index;
    }

    return size;
}

static mr_int32_t mr_avl_get_height(mr_avl_t node)
{
    if (node == MR_NULL)
    {
        return -1;
    }

    return node->height;
}

static mr_int32_t mr_avl_get_balance(mr_avl_t node)
{
    if (node == MR_NULL)
    {
        return 0;
    }

    return (mr_avl_get_height(node->left_child) - mr_avl_get_height(node->right_child));
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

/**
 * @brief This function initialize the avl tree.
 *
 * @param node The node to be initialized.
 * @param value The value to be initialized.
 */
void mr_avl_init(mr_avl_t node, mr_uint32_t value)
{
    MR_ASSERT(node != MR_NULL);

    node->height = 0;
    node->value = value;
    node->left_child = MR_NULL;
    node->right_child = MR_NULL;
}

/**
 * @brief This function insert the node in the avl tree.
 *
 * @param tree The tree to be inserted.
 * @param node The node to insert.
 */
void mr_avl_insert(mr_avl_t *tree, mr_avl_t node)
{
    mr_int32_t balance = 0;

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

/**
 * @brief This function remove the node from the avl tree.
 *
 * @param tree The tree to be removed.
 * @param node The node to be removed.
 */
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

    mr_int32_t balance = mr_avl_get_balance(*tree);

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

/**
 * @brief This function find the node in the avl tree.
 *
 * @param tree The tree to be searched.
 * @param value The value to be searched.
 *
 * @return A pointer to the found node, or MR_NULL if not found.
 */
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

/**
 * @brief This function get the length of the avl tree.
 *
 * @param tree The tree to be searched.
 *
 * @return The length of the avl tree.
 */
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
