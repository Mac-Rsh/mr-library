/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#ifndef __MR_LIBRARY_H__
#define __MR_LIBRARY_H__

#include <include/mr_def.h>
#include <include/mr_list.h>
#include <include/mr_ref.h>
#include <include/mr_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Auto-init
 * @{
 */

void mr_auto_init(void);

/** @} */

/**
 * @addtogroup Delay
 * @{
 */

void mr_delay_us(size_t us);
void mr_delay_ms(size_t ms);

/** @} */

/**
 * @addtogroup String
 * @{
 */

const char *mr_strerror(int error);
int mr_printf_output(const char *buf, size_t size);
int mr_printf(const char *fmt, ...);
int mr_log_printf_output(const char *buf, size_t size);
int mr_log_printf(const char *tag, const char *fmt, ...);

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
 * @addtogroup FIFO
 * @{
 */

int mr_fifo_init(mr_fifo_t *fifo, void *buf, size_t size);
int mr_fifo_allocate(mr_fifo_t *fifo, size_t size);
void mr_fifo_free(mr_fifo_t *fifo);
void mr_fifo_reset(mr_fifo_t *fifo);
size_t mr_fifo_peek(const mr_fifo_t *fifo, void *buf, size_t count);
size_t mr_fifo_discard(mr_fifo_t *fifo, size_t count);
size_t mr_fifo_read(mr_fifo_t *fifo, void *buf, size_t count);
size_t mr_fifo_write(mr_fifo_t *fifo, const void *buf, size_t count);
size_t mr_fifo_write_force(mr_fifo_t *fifo, const void *buf, size_t count);
size_t mr_fifo_get_used(const mr_fifo_t *fifo);
size_t mr_fifo_get_free(const mr_fifo_t *fifo);
size_t mr_fifo_get_size(const mr_fifo_t *fifo);

/** @} */

/**
 * @addtogroup Class
 * @{
 */

void mr_class_init(mr_class_t *class, const char *name, size_t privsize,
                   void (*release)(mr_class_t *), void *methods);
mr_class_t *mr_class_create(const char *name, uint32_t privsize, void *methods);
int mr_class_add(mr_class_t *class, mr_class_t *parent);
void mr_class_delete(mr_class_t *class);
int mr_class_rename(mr_class_t *class, const char *name);
int mr_class_register(mr_class_t *class, mr_class_t *parent, const char *dir);
void mr_class_unregister(mr_class_t *class);
mr_class_t *mr_class_get(mr_class_t *class);
void mr_class_put(mr_class_t *class);
mr_class_t *mr_class_find(mr_class_t *parent, const char *path);
bool mr_class_is_subclass(mr_class_t *class, mr_class_t *base);
void *mr_class_get_privdata(mr_class_t *class);
void *mr_class_get_methods(mr_class_t *class);
char *mr_extract_name(const char *path);
char *mr_extract_dir(const char *path);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_LIBRARY_H__ */
