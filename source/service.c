/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "include/mr_api.h"

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
void mr_auto_init(void)
{
    volatile const mr_init_fn_t *fn = MR_NULL;

    /* Auto-initialization */
    for (fn = &_mr_auto_init_start; fn < &_mr_auto_init_end; fn++)
    {
        (*fn)();
    }
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
 * @brief Heap memory.
 */
#ifndef MR_CFG_HEAP_SIZE
#define MR_CFG_HEAP_SIZE                (4 * 1024)                  /* If not defined, use 4KB */
#endif /* MR_CFG_HEAP_SIZE */
static uint8_t heap_mem[MR_CFG_HEAP_SIZE] = {0};

#define MR_HEAP_BLOCK_FREE              (0)
#define MR_HEAP_BLOCK_ALLOCATED         (1)
#define MR_HEAP_BLOCK_MIN_SIZE          (sizeof(struct mr_heap_block) << 1)

/**
 * @brief Heap block structure.
 */
static struct mr_heap_block
{
    struct mr_heap_block *next;
    uint32_t size: 31;
    uint32_t allocated: 1;
} heap_start = {MR_NULL, 0, MR_HEAP_BLOCK_FREE};

/**
 * @brief This function initialize the heap.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_heap_init(void)
{
    struct mr_heap_block *first_block = (struct mr_heap_block *)&heap_mem;

    /* Initialize the first block */
    first_block->next = MR_NULL;
    first_block->size = sizeof(heap_mem) - sizeof(struct mr_heap_block);
    first_block->allocated = MR_HEAP_BLOCK_FREE;

    /* Initialize the heap */
    heap_start.next = first_block;
    return MR_EOK;
}
MR_BOARD_EXPORT(mr_heap_init);

static void heap_insert_block(struct mr_heap_block *block)
{
    struct mr_heap_block *block_prev = &heap_start;

    /* Search for the previous block */
    while (((block_prev->next != MR_NULL) && ((uint32_t)block_prev->next < (uint32_t)block)))
    {
        block_prev = block_prev->next;
    }

    /* Insert the block */
    if (block_prev->next != MR_NULL)
    {
        /* Merge with the previous block */
        if ((void *)(((uint8_t *)block_prev) + sizeof(struct mr_heap_block) + block_prev->size) == (void *)block)
        {
            block_prev->size += block->size + sizeof(struct mr_heap_block);
            block = block_prev;
        }

        /* Merge with the next block */
        if ((void *)(((uint8_t *)block) + sizeof(struct mr_heap_block) + block->size) == (void *)block_prev->next)
        {
            block->size += block_prev->next->size + sizeof(struct mr_heap_block);
            block->next = block_prev->next->next;
            if (block != block_prev)
            {
                block_prev->next = block;
                block = block_prev;
            }
        }
    }

    /* Insert the block */
    if (block != block_prev)
    {
        block->next = block_prev->next;
        block_prev->next = block;
    }
}

/**
 * @brief This function allocate memory.
 *
 * @param size The size of the memory.
 *
 * @return The allocated memory.
 */
MR_WEAK void *mr_malloc(size_t size)
{
    struct mr_heap_block *block_prev = &heap_start;
    struct mr_heap_block *block = block_prev->next;
    void *memory = MR_NULL;
    size_t residual = 0;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Check size and residual memory */
    if ((size == 0) || (size > (UINT32_MAX >> 1) || (block == MR_NULL)))
    {
        /* Enable interrupt */
        mr_interrupt_enable();
        return MR_NULL;
    }

    /* Align the size up 4 bytes */
    size = mr_align4_up(size);

    /* Search for and take blocks that match the criteria */
    while (block->size < size)
    {
        if (block->next == MR_NULL)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return MR_NULL;
        }
        block_prev = block;
        block = block->next;
    }
    block_prev->next = block->next;

    /* Allocate memory */
    memory = (void *)((uint8_t *)block + sizeof(struct mr_heap_block));
    residual = block->size - size;

    /* Set the block information */
    block->size = size;
    block->next = MR_NULL;
    block->allocated = MR_HEAP_BLOCK_ALLOCATED;

    /* Check if we need to allocate a new block */
    if (residual > MR_HEAP_BLOCK_MIN_SIZE)
    {
        struct mr_heap_block *new_block = (struct mr_heap_block *)(((uint8_t *)memory) + size);

        /* Set the new block information */
        new_block->size = residual - sizeof(struct mr_heap_block);
        new_block->next = MR_NULL;
        new_block->allocated = MR_HEAP_BLOCK_FREE;

        /* Insert the new block */
        heap_insert_block(new_block);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    return memory;
}

/**
 * @brief This function free memory.
 *
 * @param memory The memory to free.
 */
MR_WEAK void mr_free(void *memory)
{
    if (memory != MR_NULL)
    {
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory - sizeof(struct mr_heap_block));

        /* Disable interrupt */
        mr_interrupt_disable();

        /* Check the block */
        if (block->allocated == MR_HEAP_BLOCK_ALLOCATED && block->size != 0)
        {
            block->allocated = MR_HEAP_BLOCK_FREE;

            /* Insert the free block */
            heap_insert_block(block);
        }

        /* Enable interrupt */
        mr_interrupt_enable();
    }
}

/**
 * @brief This function get the usable size of the memory.
 *
 * @param memory The memory.
 *
 * @return The usable size of the memory.
 */
MR_WEAK size_t mr_malloc_usable_size(void *memory)
{
    if (memory != MR_NULL)
    {
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory - sizeof(struct mr_heap_block));
        return block->size;
    }
    return 0;
}

/**
 * @brief This function initialize the memory.
 *
 * @param num The number of the memory.
 * @param size The size of the memory.
 *
 * @return The initialized memory.
 */
MR_WEAK void *mr_calloc(size_t num, size_t size)
{
    size_t total = num * size;
    void *memory = MR_NULL;

    memory = mr_malloc(total);
    if (memory != MR_NULL)
    {
        memset(memory, 0, total);
    }
    return memory;
}

/**
 * @brief This function realloc memory.
 *
 * @param memory The memory.
 * @param size The size of the memory.
 *
 * @return The reallocated memory.
 */
MR_WEAK void *mr_realloc(void *memory, size_t size)
{
    size_t old_size = mr_malloc_usable_size(memory);
    void *new_memory = MR_NULL;

    new_memory = mr_malloc(size);
    if (new_memory != MR_NULL)
    {
        memcpy(new_memory, memory, old_size);
        mr_free(memory);
    }
    return new_memory;
}

/**
 * @brief This function delay us.
 *
 * @param us The delay time.
 */
MR_WEAK void mr_delay_us(uint32_t us)
{
    volatile uint32_t i = 0;

#ifndef MR_CFG_SYSCLK_FREQ
#define MR_CFG_SYSCLK_FREQ              (72000000)
#endif /* MR_CFG_SYSCLK_FREQ */
#if (MR_CFG_SYSCLK_FREQ > 1000000)
#define MR_DELAY_COUNT                  (MR_CFG_SYSCLK_FREQ / 1000000)
#else
#define MR_DELAY_COUNT                  (1)
#endif /* (MR_CFG_SYSCLK_FREQ > 1000000) */
    for (i = 0; i < us * MR_DELAY_COUNT; i++)
    {
        __asm__("nop");
    }
#undef MR_DELAY_COUNT
#undef MR_CFG_SYSCLK_FREQ
}

/**
 * @brief This function delay ms.
 *
 * @param ms The delay time.
 */
MR_WEAK void mr_delay_ms(uint32_t ms)
{
    volatile uint32_t i = 0;

    for (i = 0; i < ms; i++)
    {
        mr_delay_us(1000);
    }
}

/**
 * @brief This function printf output.
 *
 * @param buf The buffer to output.
 * @param size The size of the buffer.
 *
 * @return The actual output size.
 */
MR_WEAK int mr_printf_output(const char *buf, size_t size)
{
#ifdef MR_USING_CONSOLE
    static int console = -1;

    if (console < 0)
    {
#ifndef MR_USING_CONSOLE_NONBLOCK
        console = mr_dev_open(MR_CFG_CONSOLE_NAME, MR_OFLAG_RDWR);
#else
        console = mr_dev_open(MR_CFG_CONSOLE_NAME, MR_OFLAG_RDWR | MR_OFLAG_NONBLOCK);
#endif /* MR_USING_CONSOLE_NONBLOCK */
        if (console < 0)
        {
            return console;
        }
    }
    return (int)mr_dev_write(console, buf, size);
#else
    return 0;
#endif /* MR_USING_CONSOLE */
}

/**
 * @brief This function printf.
 *
 * @param fmt The format string.
 * @param ... The arguments.
 *
 * @return The actual output size.
 */
int mr_printf(const char *fmt, ...)
{
#ifndef MR_CFG_PRINTF_BUFSZ
#define MR_CFG_PRINTF_BUFSZ             (128)
#endif /* MR_CFG_PRINTF_BUFSZ */
    char buf[MR_CFG_PRINTF_BUFSZ] = {0};
    va_list args;

    va_start(args, fmt);
    int ret = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    ret = mr_printf_output(buf, ret);
    va_end(args);
    return ret;
}

/**
 * @brief This function get the error message.
 *
 * @param err The error code.
 *
 * @return The error message.
 */
const char *mr_strerror(int err)
{
    switch (err)
    {
        case MR_EOK:
            return "no error";
        case MR_ENOMEM:
            return "no enough memory";
        case MR_EIO:
            return "I/O error";
        case MR_ENOTFOUND:
            return "not found";
        case MR_EBUSY:
            return "resource busy";
        case MR_EEXIST:
            return "exists";
        case MR_ENOTSUP:
            return "operation not supported";
        case MR_EINVAL:
            return "invalid argument";
        default:
            return "unknown error";
    }
}

/**
 * @brief This function initialize the ringbuffer.
 *
 * @param ringbuf The ringbuffer to initialize.
 * @param pool The pool of buf.
 * @param size The size of the pool.
 */
void mr_ringbuf_init(struct mr_ringbuf *ringbuf, void *pool, size_t size)
{
    mr_assert(ringbuf != MR_NULL);
    mr_assert((pool != MR_NULL) || (size == 0));

    ringbuf->read_index = 0;
    ringbuf->write_index = 0;
    ringbuf->read_mirror = 0;
    ringbuf->write_mirror = 0;

    ringbuf->size = size;
    ringbuf->buffer = pool;
}

/**
 * @brief This function allocate memory for the ringbuffer.
 *
 * @param ringbuf The ringbuffer to allocate.
 * @param size The size of the memory.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_ringbuf_allocate(struct mr_ringbuf *ringbuf, size_t size)
{
    void *pool = MR_NULL;

    mr_assert(ringbuf != MR_NULL);

    /* Check the buffer size */
    if (mr_ringbuf_get_bufsz(ringbuf) == size)
    {
        mr_ringbuf_reset(ringbuf);
        return MR_EOK;
    }

    /* Free old buffer */
    if (ringbuf->size != 0)
    {
        mr_free(ringbuf->buffer);
    }

    /* Allocate new buffer */
    pool = mr_malloc(size);
    if (pool == MR_NULL && size != 0)
    {
        return MR_ENOMEM;
    }
    mr_ringbuf_init(ringbuf, pool, size);

    return MR_EOK;
}

/** 
 * @brief This function free the ringbuffer.
 * 
 * @param ringbuf The ringbuffer to free. 
 */
void mr_ringbuf_free(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    mr_free(ringbuf->buffer);
    mr_ringbuf_init(ringbuf, MR_NULL, 0);
}

/**
 * @brief This function reset the ringbuffer.
 *
 * @param ringbuf The ringbuffer to reset.
 */
void mr_ringbuf_reset(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    ringbuf->read_index = 0;
    ringbuf->write_index = 0;

    ringbuf->read_mirror = 0;
    ringbuf->write_mirror = 0;
}

/**
 * @brief This function get the buf size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the buf size.
 *
 * @return The buf size.
 */
size_t mr_ringbuf_get_data_size(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    /* Empty or full according to the mirror flag */
    if (ringbuf->read_index == ringbuf->write_index)
    {
        if (ringbuf->read_mirror == ringbuf->write_mirror)
        {
            return 0;
        } else
        {
            return ringbuf->size;
        }
    }

    if (ringbuf->write_index > ringbuf->read_index)
    {
        return ringbuf->write_index - ringbuf->read_index;
    } else
    {
        return ringbuf->size - ringbuf->read_index + ringbuf->write_index;
    }
}

/**
 * @brief This function get the space size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the space size.
 *
 * @return The space size.
 */
size_t mr_ringbuf_get_space_size(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    return ringbuf->size - mr_ringbuf_get_data_size(ringbuf);
}

/**
 * @brief This function get the buffer size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the buffer size.
 *
 * @return  The buffer size.
 */
size_t mr_ringbuf_get_bufsz(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    return ringbuf->size;
}

/**
 * @brief This function pop the buf from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to pop the buf.
 * @param data The buf to pop.
 *
 * @return The size of the actual pop.
 */
size_t mr_ringbuf_pop(struct mr_ringbuf *ringbuf, uint8_t *data)
{
    mr_assert(ringbuf != MR_NULL);
    mr_assert(data != MR_NULL);

    /* Get the buf size */
    if (mr_ringbuf_get_data_size(ringbuf) == 0)
    {
        return 0;
    }

    *data = ringbuf->buffer[ringbuf->read_index];

    if (ringbuf->read_index == ringbuf->size - 1)
    {
        ringbuf->read_mirror = ~ringbuf->read_mirror;
        ringbuf->read_index = 0;
    } else
    {
        ringbuf->read_index++;
    }
    return 1;
}

/**
 * @brief This function reads from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be read.
 * @param buffer The buf buffer to be read from the ringbuffer.
 * @param size The size of the read.
 *
 * @return The size of the actual read.
 */
size_t mr_ringbuf_read(struct mr_ringbuf *ringbuf, void *buffer, size_t size)
{
    uint8_t *read_buffer = (uint8_t *)buffer;
    size_t data_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert((buffer != MR_NULL) || (size == 0));

    /* Get the buf size */
    data_size = mr_ringbuf_get_data_size(ringbuf);
    if (data_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to read if it exceeds the available buf */
    if (size > data_size)
    {
        size = data_size;
    }

    /* Copy the buf from the ringbuf to the buffer */
    if ((ringbuf->size - ringbuf->read_index) > size)
    {
        memcpy(read_buffer, &ringbuf->buffer[ringbuf->read_index], size);
        ringbuf->read_index += size;

        return size;
    }

    memcpy(read_buffer, &ringbuf->buffer[ringbuf->read_index], ringbuf->size - ringbuf->read_index);
    memcpy(&read_buffer[ringbuf->size - ringbuf->read_index],
           &ringbuf->buffer[0],
           size - (ringbuf->size - ringbuf->read_index));

    ringbuf->read_mirror = ~ringbuf->read_mirror;
    ringbuf->read_index = size - (ringbuf->size - ringbuf->read_index);
    return size;
}

/**
 * @brief This function push the buf to the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be pushed.
 * @param data The buf to be pushed.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_push(struct mr_ringbuf *ringbuf, uint8_t data)
{
    mr_assert(ringbuf != MR_NULL);

    /* Get the space size */
    if (mr_ringbuf_get_space_size(ringbuf) == 0)
    {
        return 0;
    }

    ringbuf->buffer[ringbuf->write_index] = data;

    if (ringbuf->write_index == ringbuf->size - 1)
    {
        ringbuf->write_mirror = ~ringbuf->write_mirror;
        ringbuf->write_index = 0;
    } else
    {
        ringbuf->write_index++;
    }
    return 1;
}

/**
 * @brief This function force to push the buf to the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be pushed.
 * @param data The buf to be pushed.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_push_force(struct mr_ringbuf *ringbuf, uint8_t data)
{
    int state = 0;

    mr_assert(ringbuf != MR_NULL);

    /* Get the buffer size */
    if (mr_ringbuf_get_bufsz(ringbuf) == 0)
    {
        return 0;
    }

    /* Get the space size */
    if (mr_ringbuf_get_space_size(ringbuf) == 0)
    {
        state = 1;
    }

    ringbuf->buffer[ringbuf->write_index] = data;

    if (ringbuf->write_index == ringbuf->size - 1)
    {
        ringbuf->write_mirror = ~ringbuf->write_mirror;
        ringbuf->write_index = 0;
        if (state == 1)
        {
            ringbuf->read_mirror = ~ringbuf->read_mirror;
            ringbuf->read_index = ringbuf->write_index;
        }
    } else
    {
        ringbuf->write_index++;
        if (state == 1)
        {
            ringbuf->read_index = ringbuf->write_index;
        }
    }
    return 1;
}

/**
 * @brief This function write the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be written.
 * @param buffer The buf buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_write(struct mr_ringbuf *ringbuf, const void *buffer, size_t size)
{
    uint8_t *write_buffer = (uint8_t *)buffer;
    size_t space_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert((buffer != MR_NULL) || (size == 0));

    /* Get the space size */
    space_size = mr_ringbuf_get_space_size(ringbuf);
    if (space_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to write if it exceeds the available buf */
    if (size > space_size)
    {
        size = space_size;
    }

    /* Copy the buf from the buffer to the ringbuf */
    if ((ringbuf->size - ringbuf->write_index) > size)
    {
        memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, size);
        ringbuf->write_index += size;

        return size;
    }

    memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, ringbuf->size - ringbuf->write_index);
    memcpy(&ringbuf->buffer[0],
           &write_buffer[ringbuf->size - ringbuf->write_index],
           size - (ringbuf->size - ringbuf->write_index));

    ringbuf->write_mirror = ~ringbuf->write_mirror;
    ringbuf->write_index = size - (ringbuf->size - ringbuf->write_index);
    return size;
}

/**
 * @brief This function force write the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be written.
 * @param buffer The buf buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_write_force(struct mr_ringbuf *ringbuf, const void *buffer, size_t size)
{
    uint8_t *write_buffer = (uint8_t *)buffer;
    size_t space_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert((buffer != MR_NULL) || (size == 0));

    if ((mr_ringbuf_get_bufsz(ringbuf) == 0) || (size == 0))
    {
        return 0;
    }

    /* Get the space size */
    space_size = mr_ringbuf_get_space_size(ringbuf);

    /* If the buf exceeds the buffer space_size, the front buf is discarded */
    if (size > ringbuf->size)
    {
        write_buffer = &write_buffer[size - ringbuf->size];
        size = ringbuf->size;
    }

    /* Copy the buf from the buffer to the ringbuf */
    if ((ringbuf->size - ringbuf->write_index) > size)
    {
        memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, size);
        ringbuf->write_index += size;
        if (size > space_size)
        {
            ringbuf->read_index = ringbuf->write_index;
        }

        return size;
    }

    memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, ringbuf->size - ringbuf->write_index);
    memcpy(&ringbuf->buffer[0],
           &write_buffer[ringbuf->size - ringbuf->write_index],
           size - (ringbuf->size - ringbuf->write_index));

    ringbuf->write_mirror = ~ringbuf->write_mirror;
    ringbuf->write_index = size - (ringbuf->size - ringbuf->write_index);

    if (size > space_size)
    {
        if (ringbuf->write_index <= ringbuf->read_index)
        {
            ringbuf->read_mirror = ~ringbuf->read_mirror;
        }

        ringbuf->read_index = ringbuf->write_index;
    }
    return size;
}

static int mr_avl_get_height(struct mr_avl *node)
{
    if (node == MR_NULL)
    {
        return -1;
    }
    return node->height;
}

static int mr_avl_get_balance(struct mr_avl *node)
{
    if (node == MR_NULL)
    {
        return 0;
    }
    return (mr_avl_get_height(node->left_child) - mr_avl_get_height(node->right_child));
}

static void mr_avl_left_rotate(struct mr_avl **node)
{
    struct mr_avl *right_child = (*node)->right_child;

    (*node)->right_child = right_child->left_child;
    right_child->left_child = (*node);

    (*node)->height = mr_max(mr_avl_get_height((*node)->left_child), mr_avl_get_height((*node)->right_child)) + 1;
    right_child->height =
        mr_max(mr_avl_get_height(right_child->left_child), mr_avl_get_height(right_child->right_child)) + 1;

    (*node) = right_child;
}

static void mr_avl_right_rotate(struct mr_avl **node)
{
    struct mr_avl *left_child = (*node)->left_child;

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
void mr_avl_init(struct mr_avl *node, uint32_t value)
{
    mr_assert(node != MR_NULL);

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
void mr_avl_insert(struct mr_avl **tree, struct mr_avl *node)
{
    int balance = 0;

    mr_assert(tree != MR_NULL);
    mr_assert(node != MR_NULL);

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
void mr_avl_remove(struct mr_avl **tree, struct mr_avl *node)
{
    mr_assert(tree != MR_NULL);
    mr_assert(node != MR_NULL);

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
            struct mr_avl *temp = (*tree)->right_child;
            (*tree)->right_child = MR_NULL;
            (*tree) = temp;
            return;
        } else if ((*tree)->right_child == MR_NULL)
        {
            struct mr_avl *temp = (*tree)->left_child;
            (*tree)->left_child = MR_NULL;
            (*tree) = temp;
            return;
        }

        struct mr_avl *temp = (*tree)->right_child->left_child;
        (*tree)->value = temp->value;
        mr_avl_remove(&(*tree)->right_child, temp);
        return;
    }

    (*tree)->height = mr_max(mr_avl_get_height((*tree)->left_child), mr_avl_get_height((*tree)->right_child)) + 1;

    int balance = mr_avl_get_balance(*tree);

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
struct mr_avl *mr_avl_find(struct mr_avl *tree, uint32_t value)
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
size_t mr_avl_get_length(struct mr_avl *tree)
{
    size_t length = 1;

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
