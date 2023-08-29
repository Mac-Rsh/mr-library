/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _MR_LIB_H_
#define _MR_LIB_H_

#include "mrdef.h"
#include "mrservice.h"

/**
 * @addtogroup Kernel
 * @{
 */
int mr_auto_init(void);
mr_size_t mr_printf_output(const char *buffer, mr_size_t size);
mr_size_t mr_printf(const char *format, ...);
void mr_log_output(mr_base_t level, const char *tag, const char *format, ...);
void mr_assert_handle(char *file, int line);
void mr_interrupt_disable(void);
void mr_interrupt_enable(void);
void mr_delay_us(mr_uint32_t us);
void mr_delay_ms(mr_uint32_t ms);
/** @} */

/**
 * @addtogroup Ring Buffer
 * @{
 */
void mr_rb_init(mr_rb_t rb, void *pool, mr_size_t pool_size);
void mr_rb_reset(mr_rb_t rb);
mr_size_t mr_rb_get_data_size(mr_rb_t rb);
mr_size_t mr_rb_get_space_size(mr_rb_t rb);
mr_size_t mr_rb_get_buffer_size(mr_rb_t rb);
mr_size_t mr_rb_get(mr_rb_t rb, mr_uint8_t *data);
mr_size_t mr_rb_read(mr_rb_t rb, void *buffer, mr_size_t size);
mr_size_t mr_rb_put(mr_rb_t rb, mr_uint8_t data);
mr_size_t mr_rb_put_force(mr_rb_t rb, mr_uint8_t data);
mr_size_t mr_rb_write(mr_rb_t rb, const void *buffer, mr_size_t size);
mr_size_t mr_rb_write_force(mr_rb_t rb, const void *buffer, mr_size_t size);
/** @} */

/**
 * @addtogroup AVL tree
 * @{
 */
void mr_avl_init(mr_avl_t node, mr_uint32_t value);
void mr_avl_insert(mr_avl_t *tree, mr_avl_t node);
void mr_avl_remove(mr_avl_t *tree, mr_avl_t node);
mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value);
mr_size_t mr_avl_get_length(mr_avl_t tree);
mr_uint32_t mr_str2hash(const char *string, mr_size_t length);
/** @} */

/**
 * @addtogroup Object
 * @{
 */
mr_object_container_t mr_object_container_find(enum mr_object_type type);
mr_object_t mr_object_find(const char *name, enum mr_object_type type);
mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_object_type type);
mr_err_t mr_object_remove(mr_object_t object);
mr_err_t mr_object_change_type(mr_object_t object, enum mr_object_type type);
void mr_object_rename(mr_object_t object, char *name);
/** @} */

/**
 * @addtogroup Mutex
 * @{
 */
void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, void *owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, void *owner);
volatile void *mr_mutex_get_owner(mr_mutex_t mutex);
/** @} */

/**
 * @addtogroup Memory
 * @{
 */
void *mr_malloc(mr_size_t size);
void mr_free(void *memory);
/** @} */

/**
 * @addtogroup Finite State Machine
 * @{
 */
#if (MR_CFG_FSM == MR_CFG_ENABLE)
mr_fsm_t mr_fsm_find(const char *name);
mr_err_t mr_fsm_add(mr_fsm_t fsm, const char *name, mr_fsm_table_t table, mr_size_t table_size);
mr_err_t mr_fsm_remove(mr_fsm_t fsm);
void mr_fsm_handle(mr_fsm_t fsm);
mr_err_t mr_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal);
mr_err_t mr_fsm_shift(mr_fsm_t fsm, mr_uint32_t index);
#endif
/** @} */

/**
 * @addtogroup Event
 * @{
 */
#if (MR_CFG_EVENT == MR_CFG_ENABLE)
mr_event_t mr_event_find(const char *name);
mr_err_t mr_event_add(mr_event_t event, const char *name, mr_event_table_t table, mr_size_t table_size);
mr_err_t mr_event_remove(mr_event_t event);
void mr_event_handle(mr_event_t event);
mr_err_t mr_event_notify(mr_event_t event, mr_uint32_t index);
mr_err_t mr_event_toggle(mr_event_t event, mr_uint32_t index);
#endif
/** @} */

/**
 * @addtogroup Soft Timer
 * @{
 */
#if (MR_CFG_SOFT_TIMER == MR_CFG_ENABLE)
mr_soft_timer_t mr_soft_timer_find(const char *name);
mr_err_t mr_soft_timer_add(mr_soft_timer_t timer, const char *name, mr_soft_timer_table_t table, mr_size_t table_size);
mr_err_t mr_soft_timer_remove(mr_soft_timer_t timer);
void mr_soft_timer_update(mr_soft_timer_t timer, mr_uint32_t time);
void mr_soft_timer_handle(mr_soft_timer_t timer);
mr_err_t mr_soft_timer_start(mr_soft_timer_t timer, mr_uint32_t index);
mr_err_t mr_soft_timer_stop(mr_soft_timer_t timer, mr_uint32_t index);
mr_uint32_t mr_soft_timer_get_time(mr_soft_timer_t timer);
#endif
/** @} */

/**
 * @addtogroup Device
 * @{
 */
#if (MR_CFG_DEVICE == MR_CFG_ENABLE)
mr_device_t mr_device_find(const char *name);
mr_err_t mr_device_add(mr_device_t device,
                       const char *name,
                       enum mr_device_type type,
                       mr_uint16_t flags,
                       struct mr_device_ops *ops,
                       void *data);
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
mr_err_t mr_device_close(mr_device_t device);
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size);
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
#endif
/** @} */

/**
 * @addtogroup Lib C
 * @{
 */
#define mr_strncmp                      strncmp
#define mr_strncpy                      strncpy
#define mr_memset                       memset
#define mr_memcpy                       memcpy
#define mr_strlen                       strlen
#define mr_vsnprintf                    vsnprintf
#define mr_snprintf                     snprintf
#define mr_sscanf                       sscanf
/** @} */

#endif /* _MR_LIB_H_ */