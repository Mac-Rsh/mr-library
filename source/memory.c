/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-20    MacRsh       First version
 */

#include "include/mr_api.h"

#ifndef MR_CFG_HEAP_SIZE
#define MR_CFG_HEAP_SIZE                (4 * 1024)                  /**< If not defined, use 4KB */
#elif (MR_CFG_HEAP_SIZE < 16)
#define MR_CFG_HEAP_SIZE                (16)                        /**< If less than 16, use 16B */
#endif /* MR_CFG_HEAP_SIZE */
static uint8_t heap_mem[MR_CFG_HEAP_SIZE] = {0};                    /**< Heap memory */

#define MR_HEAP_BLOCK_FREE              (0)
#define MR_HEAP_BLOCK_ALLOCATED         (1)
#define MR_HEAP_BLOCK_MIN_SIZE          (sizeof(struct mr_heap_block) << 1)

static struct mr_heap_block heap_start =                            /**< Heap start block */
    {MR_NULL, 0, MR_HEAP_BLOCK_FREE};

/**
 * @brief This function initialize the heap.
 */
static void mr_heap_init(void)
{
    struct mr_heap_block *first_block = (struct mr_heap_block *)&heap_mem;

    /* Initialize the first block */
    first_block->next = MR_NULL;
    first_block->size = sizeof(heap_mem) - sizeof(struct mr_heap_block);
    first_block->allocated = MR_HEAP_BLOCK_FREE;
    heap_start.next = first_block;
}
MR_INIT_BOARD_EXPORT(mr_heap_init);

static void heap_insert_block(struct mr_heap_block *block)
{
    struct mr_heap_block *block_prev = &heap_start;

    /* Search for the previous block */
    while (((block_prev->next != MR_NULL) && (block_prev->next < block))) {
        block_prev = block_prev->next;
    }

    /* Insert the block */
    if (block_prev->next != MR_NULL) {
        /* Merge with the previous block */
        if ((void *)(((uint8_t *)block_prev) + sizeof(struct mr_heap_block) + block_prev->size) ==
            (void *)block) {
            block_prev->size += block->size + sizeof(struct mr_heap_block);
            block = block_prev;
        }

        /* Merge with the next block */
        if ((void *)(((uint8_t *)block) + sizeof(struct mr_heap_block) + block->size) ==
            (void *)block_prev->next) {
            block->size += block_prev->next->size + sizeof(struct mr_heap_block);
            block->next = block_prev->next->next;
            if (block != block_prev) {
                block_prev->next = block;
                block = block_prev;
            }
        }
    }

    /* Insert the block */
    if (block != block_prev) {
        block->next = block_prev->next;
        block_prev->next = block;
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
    struct mr_heap_block *block_prev = &heap_start;
    struct mr_heap_block *block = block_prev->next;

    mr_interrupt_disable();

    /* Check size and residual memory */
    if ((size == 0) || (size > (UINT32_MAX >> 1) || (block == MR_NULL))) {
        mr_interrupt_enable();
        return MR_NULL;
    }

    /* Align the size to the next multiple of 4 bytes */
    size = MR_ALIGN_UP(size, 4);

    /* Search for and take blocks that match the criteria */
    while (block->size < size) {
        if (block->next == MR_NULL) {
            mr_interrupt_enable();
            return MR_NULL;
        }
        block_prev = block;
        block = block->next;
    }
    block_prev->next = block->next;

    /* Allocate memory */
    void *memory = (void *)((uint8_t *)block + sizeof(struct mr_heap_block));
    size_t residual = block->size - size;

    /* Set the block information */
    block->size = size;
    block->next = MR_NULL;
    block->allocated = MR_HEAP_BLOCK_ALLOCATED;

    /* Check if we need to allocate a new block */
    if (residual > MR_HEAP_BLOCK_MIN_SIZE) {
        struct mr_heap_block *new_block = (struct mr_heap_block *)(((uint8_t *)memory) + size);

        /* Set the new block information */
        new_block->size = residual - sizeof(struct mr_heap_block);
        new_block->next = MR_NULL;
        new_block->allocated = MR_HEAP_BLOCK_FREE;

        /* Insert the new block */
        heap_insert_block(new_block);
    }

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
    if (memory != MR_NULL) {
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory -
                                                               sizeof(struct mr_heap_block));

        mr_interrupt_disable();

        /* Check the block */
        if (block->allocated == MR_HEAP_BLOCK_ALLOCATED && block->size != 0) {
            block->allocated = MR_HEAP_BLOCK_FREE;

            /* Insert the free block */
            heap_insert_block(block);
        }

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
    if (memory != MR_NULL) {
        /* Get the block information */
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory -
                                                               sizeof(struct mr_heap_block));
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
 * @return A pointer to the allocated memory.
 */
MR_WEAK void *mr_calloc(size_t num, size_t size)
{
    size_t total = num * size;

    void *memory = mr_malloc(total);
    if (memory != MR_NULL) {
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
 * @return A pointer to the allocated memory.
 */
MR_WEAK void *mr_realloc(void *memory, size_t size)
{
    size_t old_size = mr_malloc_usable_size(memory);

    void *new_memory = mr_malloc(size);
    if (new_memory != MR_NULL) {
        memcpy(new_memory, memory, old_size);
        mr_free(memory);
    }
    return new_memory;
}
