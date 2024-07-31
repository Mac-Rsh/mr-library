/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef __MR_MEMORY_H__
#define __MR_MEMORY_H__

#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Memory
 * @{
 */

/**
 * @brief Heap block structure.
 */
struct mr_heap_block
{
    struct mr_heap_block *next;                                                     /**< Point to next block */
    size_t size;                                                                    /**< Block size */
};

void *mr_malloc(size_t size);
void mr_free(void *memory);
size_t mr_malloc_usable_size(void *memory);
void *mr_calloc(size_t num, size_t size);
void *mr_realloc(void *memory, size_t size);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_MEMORY_H__ */
