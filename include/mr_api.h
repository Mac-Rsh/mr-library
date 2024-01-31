/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef _MR_API_H_
#define _MR_API_H_

#include "mr_def.h"
#include "mr_service.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Interrupt
 * @{
 */
void mr_interrupt_disable(void);
void mr_interrupt_enable(void);
/** @} */

/**
 * @addtogroup Delay
 * @{
 */
void mr_delay_us(uint32_t us);
void mr_delay_ms(uint32_t ms);
/** @} */

/**
 * @addtogroup Memory
 * @{
 */
void *mr_malloc(size_t size);
void mr_free(void *memory);
size_t mr_malloc_usable_size(void *memory);
void *mr_calloc(size_t num, size_t size);
void *mr_realloc(void *memory, size_t size);
/** @} */

/**
 * @addtogroup String
 * @{
 */
int mr_printf(const char *fmt, ...);
const char *mr_strerror(int err);
const char *mr_strflags(int flags);
/** @} */

/**
 * @addtogroup Auto-init
 * @{
 */
void mr_auto_init(void);
/** @} */

/**
 * @addtogroup Ringbuffer
 * @{
 */
void mr_ringbuf_init(struct mr_ringbuf *ringbuf, void *pool, size_t size);
int mr_ringbuf_allocate(struct mr_ringbuf *ringbuf, size_t size);
void mr_ringbuf_free(struct mr_ringbuf *ringbuf);
void mr_ringbuf_reset(struct mr_ringbuf *ringbuf);
size_t mr_ringbuf_get_data_size(struct mr_ringbuf *ringbuf);
size_t mr_ringbuf_get_space_size(struct mr_ringbuf *ringbuf);
size_t mr_ringbuf_get_bufsz(struct mr_ringbuf *ringbuf);
size_t mr_ringbuf_pop(struct mr_ringbuf *ringbuf, uint8_t *data);
size_t mr_ringbuf_read(struct mr_ringbuf *ringbuf, void *buffer, size_t size);
size_t mr_ringbuf_push(struct mr_ringbuf *ringbuf, uint8_t data);
size_t mr_ringbuf_push_force(struct mr_ringbuf *ringbuf, uint8_t data);
size_t mr_ringbuf_write(struct mr_ringbuf *ringbuf, const void *buffer, size_t size);
size_t mr_ringbuf_write_force(struct mr_ringbuf *ringbuf, const void *buffer, size_t size);
/** @} */

/**
 * @addtogroup AVL-tree
 * @{
 */
void mr_avl_init(struct mr_avl *node, uint32_t value);
void mr_avl_insert(struct mr_avl **tree, struct mr_avl *node);
void mr_avl_remove(struct mr_avl **tree, struct mr_avl *node);
struct mr_avl *mr_avl_find(struct mr_avl *tree, uint32_t value);
size_t mr_avl_get_length(struct mr_avl *tree);
/** @} */

/**
 * @addtogroup Device
 * @{
 */
int mr_dev_register(struct mr_dev *dev,
                    const char *path,
                    int type,
                    int flags,
                    struct mr_dev_ops *ops,
                    struct mr_drv *drv);
int mr_dev_isr(struct mr_dev *dev, int event, void *args);
int mr_dev_open(const char *path, int flags);
int mr_dev_close(int desc);
ssize_t mr_dev_read(int desc, void *buf, size_t count);
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
int mr_dev_ioctl(int desc, int cmd, void *args);
/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_API_H_ */
