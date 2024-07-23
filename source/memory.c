/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-20    MacRsh       First version
 */

#include <include/mr_library.h>

#ifndef MR_CFG_HEAP_SIZE
#define MR_CFG_HEAP_SIZE                    (4 * 1024)
#elif (MR_CFG_HEAP_SIZE < 16)
#define MR_CFG_HEAP_SIZE                    (16)
#endif /* MR_CFG_HEAP_SIZE */
static uint8_t __heap_memory[MR_CFG_HEAP_SIZE];
static mr_heap_block_t __heap_start;

/**
 * @brief This function initialize the heap.
 */
static int mr_heap_init(void)
{
    mr_heap_block_t *block;

    /* Initialize the first block */
    block = (mr_heap_block_t *)&__heap_memory;
    block->next = NULL;
    block->size = sizeof(__heap_memory) - sizeof(mr_heap_block_t);
    block->allocated = false;
    __heap_start.next = block;
    return MR_EOK;
}
MR_INIT_SYSTEM_EXPORT("heap-init", mr_heap_init);

static void __heap_insert_block(mr_heap_block_t *block)
{
    mr_heap_block_t *prev;

    /* Search for the previous block */
    prev = &__heap_start;
    while (((prev->next != NULL) && (prev->next < block)))
    {
        prev = prev->next;
    }

    /* Insert the block */
    if (prev->next != NULL)
    {
        /* Merge with the previous block */
        if ((void *)(((uint8_t *)prev) + sizeof(mr_heap_block_t) + prev->size) == (void *)block)
        {
            prev->size += block->size + sizeof(mr_heap_block_t);
            block = prev;
        }

        /* Merge with the next block */
        if ((void *)(((uint8_t *)block) + sizeof(mr_heap_block_t) + block->size)
            == (void *)prev->next)
        {
            block->size += prev->next->size + sizeof(mr_heap_block_t);
            block->next = prev->next->next;
            if (block != prev)
            {
                prev->next = block;
                block = prev;
            }
        }
    }

    /* Insert the block */
    if (block != prev)
    {
        block->next = prev->next;
        prev->next = block;
    }
}

/**
 * @brief This function allocate memory.
 *
 * @param size The size of the memory.
 *
 * @return A pointer to the allocated memory.
 */
MR_WEAK void *mr_malloc(size_t size)
{
    mr_heap_block_t *prev, *block, *new;
    uint8_t *memory;
    size_t residual;
    size_t mask;

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Check size and residual memory */
    prev = &__heap_start;
    block = __heap_start.next;
    if ((size == 0) || (size > (UINT32_MAX >> 1) || (block == NULL)))
    {
        memory = NULL;
        goto _exit;
    }

    /* Align the size to the next multiple of 4 bytes */
    size = MR_ALIGN_UP(size, 4);

    /* Search for and take blocks that match the criteria */
    while (block->size < size)
    {
        if (block->next == NULL)
        {
            memory = NULL;
            goto _exit;
        }
        prev = block;
        block = block->next;
    }
    prev->next = block->next;

    /* Allocate memory */
    memory = ((uint8_t *)block + sizeof(mr_heap_block_t));
    residual = block->size - size;

    /* Set the block information */
    block->size = size;
    block->next = NULL;
    block->allocated = true;

    /* Check if we need to allocate a new block */
    if (residual > (sizeof(mr_heap_block_t) << 1))
    {
        new = (mr_heap_block_t *)(memory + size);

        /* Set the new block information */
        new->size = residual - sizeof(mr_heap_block_t);
        new->next = NULL;
        new->allocated = false;

        /* Insert the new block */
        __heap_insert_block(new);
    }

_exit:
    /* Enable interrupt */
    mr_irq_enable(mask);
    return memory;
}

/**
 * @brief This function free memory.
 *
 * @param memory The memory to free.
 */
MR_WEAK void mr_free(void *memory)
{
    mr_heap_block_t *block;
    size_t mask;

    if (memory == NULL)
    {
        return;
    }

    /* Disable interrupt */
    mask = mr_irq_disable();

    /* Check the block */
    block = (mr_heap_block_t *)((uint8_t *)memory - sizeof(mr_heap_block_t));
    if ((block->allocated == true) && (block->size != 0))
    {
        block->allocated = false;

        /* Insert the free block */
        __heap_insert_block(block);
    }

    /* Enable interrupt */
    mr_irq_enable(mask);
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
    mr_heap_block_t *block;

    if (memory == NULL)
    {
        return 0;
    }

    /* Get the block information */
    block = (mr_heap_block_t *)((uint8_t *)memory - sizeof(mr_heap_block_t));
    return block->size;
}

/**
 * @brief This function initialize the memory.
 *
 * @param num The number of the memory.
 * @param size The size of the memory.
 *
 * @return A pointer to the allocated memory.
 */
MR_WEAK void *mr_calloc(size_t num, size_t size)
{
    size_t total;
    void *memory;

    /* Calculate the total size */
    total = num * size;
    if (total == 0)
    {
        return NULL;
    }

    /* Allocate memory */
    memory = mr_malloc(total);
    if (memory == NULL)
    {
        /* Not enough memory */
        return NULL;
    }

    /* Initialize the memory to 0 */
    memset(memory, 0, total);
    return memory;
}

/**
 * @brief This function realloc memory.
 *
 * @param memory The memory.
 * @param size The size of the memory.
 *
 * @return A pointer to the allocated memory.
 */
MR_WEAK void *mr_realloc(void *memory, size_t size)
{
    size_t old_size;
    void *new;

    /* Get the old size */
    old_size = mr_malloc_usable_size(memory);

    /* Reallocate memory */
    new = mr_malloc(size);
    if (new == NULL)
    {
        /* Not enough memory */
        return NULL;
    }

    /* Copy the memory data to the new memory */
    memcpy(new, memory, old_size);
    mr_free(memory);
    return new;
}
