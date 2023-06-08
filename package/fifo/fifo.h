/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-23     MacRsh       first version
 */

#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdint.h>
#include <string.h>

#define FIFO_ASSERT(x)

struct fifo
{
    uint8_t *buffer;                                                /**< Buffer pool */

    uint16_t read_mirror: 1;                                        /**< Read mirror flag */
    uint16_t read_index: 15;                                        /**< Read index */
    uint16_t write_mirror: 1;                                       /**< Write mirror flag */
    uint16_t write_index: 15;                                       /**< Write index */

    uint16_t size;                                                  /**< Buffer pool size */
};
typedef struct fifo *fifo_t;

void fifo_init(fifo_t fifo, void *pool, size_t pool_size);
void fifo_reset(fifo_t fifo);
size_t fifo_get_data_size(fifo_t fifo);
#define fifo_get_space_size(fifo) ((fifo->size) - fifo_get_data_size(fifo))
#define fifo_get_buffer_size(fifo) (fifo->size)
size_t fifo_read(fifo_t fifo, void *buffer, size_t size);
size_t fifo_write(fifo_t fifo, const void *buffer, size_t size);
size_t fifo_write_force(fifo_t fifo, const void *buffer, size_t size);

#endif
