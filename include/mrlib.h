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
 *  Export kernel functions
 */
mr_container_t mr_container_find(mr_uint8_t type);
mr_object_t mr_object_find(const char *name, mr_uint8_t type);
mr_err_t mr_object_add(mr_object_t object, const char *name, mr_uint8_t type);
mr_err_t mr_object_remove(mr_object_t object);
mr_err_t mr_object_change_type(mr_object_t object, mr_uint8_t type);
void mr_object_rename(mr_object_t object, char *name);
void mr_object_set_err(mr_object_t object, mr_err_t err);
void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner);
mr_object_t mr_mutex_get_owner(mr_mutex_t mutex);

#if (MR_CONF_DEVICE == MR_CONF_ENABLE)
/**
 *  Export device functions
 */
mr_device_t mr_device_find(const char *name);
mr_err_t mr_device_add(mr_device_t device, const char *name, mr_uint16_t flags);
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
mr_err_t mr_device_close(mr_device_t device);
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size);
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
#endif  /* MR_CONF_DEVICE */

#if (MR_CONF_FSM == MR_CONF_ENABLE)
/**
 *  Export fsm functions
 */
mr_err_t mr_fsm_add(mr_fsm_t fsm, const char *name, mr_fsm_table_t table, mr_size_t table_size);
void mr_fsm_handle(mr_fsm_t fsm);
mr_err_t mr_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal);
mr_err_t mr_fsm_shift(mr_fsm_t fsm, mr_uint32_t state);
#endif  /* MR_CONF_FSM */

#if (MR_CONF_EVENT == MR_CONF_ENABLE)
/**
 *  Export event server functions
 */
mr_event_server_t mr_event_server_find(const char *name);
mr_err_t mr_event_server_add(mr_event_server_t server, const char *name, mr_size_t queue_length);
mr_err_t mr_event_server_remove(mr_event_server_t server);
void mr_event_server_handle(mr_event_server_t server);
mr_err_t mr_event_create(mr_uint8_t id,
                         mr_err_t (*cb)(mr_event_server_t server, void *args),
                         void *args,
                         mr_event_server_t server);
mr_err_t mr_event_delete(mr_uint8_t id, mr_event_server_t server);
mr_err_t mr_event_notify(mr_uint8_t id, mr_event_server_t server);
mr_err_t mr_event_trigger(mr_uint8_t id, mr_event_server_t server);
#endif  /* MR_CONF_EVENT */

#if (MR_CONF_SOFT_TIMER == MR_CONF_ENABLE)
/**
 *  Export soft timer server functions
 */
mr_soft_timer_server_t mr_soft_timer_server_find(const char *name);
mr_err_t mr_soft_timer_server_add(mr_soft_timer_server_t server, const char *name);
mr_err_t mr_soft_timer_server_remove(mr_soft_timer_server_t server);
void mr_soft_timer_server_update(mr_soft_timer_server_t server, mr_uint32_t time);
void mr_soft_timer_server_handle(mr_soft_timer_server_t server);
mr_err_t mr_soft_timer_create(mr_uint8_t id,
                              mr_uint32_t timeout,
                              mr_err_t (*cb)(mr_soft_timer_server_t server, void *args),
                              void *args,
                              mr_soft_timer_server_t server);
mr_err_t mr_soft_timer_delete(mr_uint8_t id, mr_soft_timer_server_t server);
mr_err_t mr_soft_timer_start(mr_uint8_t id, mr_soft_timer_server_t server);
mr_err_t mr_soft_timer_stop(mr_uint8_t id, mr_soft_timer_server_t server);
mr_err_t mr_soft_timer_create_and_start(mr_uint8_t id,
                                        mr_uint32_t timeout,
                                        mr_err_t (*cb)(mr_soft_timer_server_t server, void *args),
                                        void *args,
                                        mr_soft_timer_server_t server);
#endif  /* MR_CONF_SOFT_TIMER */

/**
 *  Export kernel service functions
 */
int mr_auto_init(void);
mr_size_t mr_printf_output(const char *buffer, mr_size_t size);
mr_size_t mr_printf(const char *format, ...);
void mr_log_output(mr_base_t level, const char *tag, const char *format, ...);
void mr_assert_handle(void);
void mr_interrupt_disable(void);
void mr_interrupt_enable(void);
void mr_delay_us(mr_uint32_t us);
void mr_delay_ms(mr_uint32_t ms);

/**
 *  Export ringbuffer functions
 */
void mr_rb_init(mr_rb_t rb, void *pool, mr_size_t pool_size);
void mr_rb_reset(mr_rb_t rb);
mr_size_t mr_rb_get_data_size(mr_rb_t rb);
mr_size_t mr_rb_get_space_size(mr_rb_t rb);
mr_size_t mr_rb_get_buffer_size(mr_rb_t rb);
mr_size_t mr_rb_read(mr_rb_t rb, void *buffer, mr_size_t size);
mr_size_t mr_rb_get(mr_rb_t rb, mr_uint8_t *data);
mr_size_t mr_rb_write(mr_rb_t rb, const void *buffer, mr_size_t size);
mr_size_t mr_rb_write_force(mr_rb_t rb, const void *buffer, mr_size_t size);
mr_size_t mr_rb_put(mr_rb_t rb, mr_uint8_t data);

/**
 *  Export avl functions
 */
void mr_avl_init(mr_avl_t node, mr_uint32_t value);
void mr_avl_insert(mr_avl_t *tree, mr_avl_t node);
void mr_avl_remove(mr_avl_t *tree, mr_avl_t node);
mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value);
mr_size_t mr_avl_get_length(mr_avl_t tree);
mr_uint32_t mr_str2hash(const char *string, mr_size_t length);

#endif  /* _MR_LIB_H_ */