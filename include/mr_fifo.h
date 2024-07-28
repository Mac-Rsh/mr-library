/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-07-27    MacRsh       First version
 */

#ifndef __MR_FIFO_H__
#define __MR_FIFO_H__

#include <include/mr_def.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup FIFO
 * @{
 */

/**
 * @brief FIFO structure.
 */
struct mr_fifo
{
    uint8_t *buf;                                                                                       /**< Data buffer */
    size_t in;                                                                                          /**< Input index */
    size_t out;                                                                                         /**< Output index */
    size_t size;                                                                                        /**< Buffer size */
};

int mr_fifo_init(struct mr_fifo *fifo, void *buf, size_t size);
int mr_fifo_allocate(struct mr_fifo *fifo, size_t size);
void mr_fifo_free(struct mr_fifo *fifo);
void mr_fifo_reset(struct mr_fifo *fifo);
size_t mr_fifo_peek(const struct mr_fifo *fifo, void *buf, size_t count);
size_t mr_fifo_discard(struct mr_fifo *fifo, size_t count);
size_t mr_fifo_read(struct mr_fifo *fifo, void *buf, size_t count);
size_t mr_fifo_write(struct mr_fifo *fifo, const void *buf, size_t count);
size_t mr_fifo_write_force(struct mr_fifo *fifo, const void *buf, size_t count);
size_t mr_fifo_get_used(const struct mr_fifo *fifo);
size_t mr_fifo_get_free(const struct mr_fifo *fifo);
size_t mr_fifo_get_size(const struct mr_fifo *fifo);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_CLASS_H__ */
