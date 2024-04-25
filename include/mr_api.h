/*
* @copyright (c) 2023-2024, MR Development Team
*
* @license SPDX-License-Identifier: Apache-2.0
*
* @date 2023-10-20    MacRsh       First version
*/

#ifndef _MR_API_H_
#define _MR_API_H_

#include "../mr-library/include/mr_def.h"
#include "../mr-library/include/mr_service.h"

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
 * @addtogroup Interrupt
 * @{
 */

void mr_interrupt_disable(void);
void mr_interrupt_enable(void);
void mr_critical_enter(void);
void mr_critical_exit(void);

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

/** @} */

/**
 * @addtogroup Assert
 * @{
 */

void mr_assert_handler(const char *ex, const char *tag, const char *fn,
                       const char *file, int line);

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

int mr_fifo_init(struct mr_fifo *fifo, void *buf, size_t size);
void mr_fifo_reset(struct mr_fifo *fifo);
int mr_fifo_allocate(struct mr_fifo *fifo, size_t size);
void mr_fifo_free(struct mr_fifo *fifo);
size_t mr_fifo_used_get(struct mr_fifo *fifo);
size_t mr_fifo_space_get(struct mr_fifo *fifo);
size_t mr_fifo_size_get(struct mr_fifo *fifo);
size_t mr_fifo_peek(struct mr_fifo *fifo, void *_buf, size_t count);
size_t mr_fifo_discard(struct mr_fifo *fifo, size_t count);
size_t mr_fifo_read(struct mr_fifo *fifo, void *buf, size_t count);
size_t mr_fifo_write(struct mr_fifo *fifo, const void *buf, size_t count);
size_t mr_fifo_write_force(struct mr_fifo *fifo, const void *buf, size_t count);

/** @} */

/**
 * @addtogroup Device
 * @{
 */

int mr_device_register_to(struct mr_device *device, const char *path,
                          uint32_t type, struct mr_device_ops *ops,
                          const void *driver, const char *to_path);
int mr_device_register(struct mr_device *device, const char *path,
                       uint32_t type, struct mr_device_ops *ops,
                       const void *driver);
int mr_device_unregister(struct mr_device *device);
int mr_device_isr(struct mr_device *device, uint32_t event, void *args);
int mr_device_open(const char *path, uint32_t flags);
int mr_device_close(int descriptor);
ssize_t mr_device_read(int descriptor, void *buf, size_t count);
ssize_t mr_device_write(int descriptor, const void *buf, size_t count);
int mr_device_ioctl(int descriptor, int cmd, void *args);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_API_H_ */
