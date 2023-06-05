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
 *  Export container functions
 */
mr_container_t mr_container_find(enum mr_container_type type);

/**
 *  Export object functions
 */
mr_object_t mr_object_find(const char *name, enum mr_container_type type);
mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_container_type type);
mr_err_t mr_object_remove(mr_object_t object);
mr_err_t mr_object_move(mr_object_t object, enum mr_container_type type);
void mr_object_rename(mr_object_t object, char *name);

/**
 *  Export mutex functions
 */
void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner);

#if (MR_CONF_DEVICE == MR_CONF_ENABLE)
/**
 *  Export device functions
 */
mr_device_t mr_device_find(const char *name);
mr_err_t mr_device_add(mr_device_t device,
                       const char *name,
                       enum mr_device_type type,
                       mr_uint16_t support_flag,
                       const struct mr_device_ops *ops,
                       void *data);
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
mr_err_t mr_device_close(mr_device_t device);
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
#endif

#if (MR_CONF_EVENT == MR_CONF_ENABLE)
/**
 *  Export event functions
 */
mr_event_server_t mr_event_server_find(const char *name);
mr_err_t mr_event_server_add(mr_event_server_t server, const char *name, mr_size_t queue_length);
mr_err_t mr_event_server_remove(mr_event_server_t server);
mr_err_t mr_event_server_notify(mr_event_server_t server, mr_uint8_t id);
void mr_event_server_handle(mr_event_server_t server);
mr_event_client_t mr_event_client_find(mr_uint8_t id, mr_event_server_t server);
mr_err_t mr_event_client_create(mr_uint8_t id,
                                mr_err_t (*cb)(mr_event_server_t server, void *args),
                                void *args,
                                mr_event_server_t server);
mr_err_t mr_client_delete(mr_uint8_t id, mr_event_server_t server);
#endif

/**
 *  Export kservice functions
 */
void mr_auto_init(void);
mr_size_t mr_printf(const char *fmt, ...);
void mr_log_output(mr_base_t level, const char *tag, const char *fmt, ...);
mr_weak void mr_assert_handle(void);
mr_weak void mr_interrupt_disable(void);
mr_weak void mr_interrupt_enable(void);
mr_weak void mr_delay_ms(mr_uint32_t ms);

/**
 *  Export fifo functions
 */
void mr_fifo_init(mr_fifo_t fifo, void *pool, mr_size_t pool_size);
void mr_fifo_reset(mr_fifo_t fifo);
mr_size_t mr_fifo_get_data_size(mr_fifo_t fifo);
mr_size_t mr_fifo_get_space_size(mr_fifo_t fifo);
mr_size_t mr_fifo_get_buffer_size(mr_fifo_t fifo);
mr_size_t mr_fifo_read(mr_fifo_t fifo, void *buffer, mr_size_t size);
mr_size_t mr_fifo_write(mr_fifo_t fifo, const void *buffer, mr_size_t size);
mr_size_t mr_fifo_write_force(mr_fifo_t fifo, const void *buffer, mr_size_t size);

/**
 *  Export avl functions
 */
void mr_avl_init(mr_avl_t node, mr_uint32_t value);
void mr_avl_insert(mr_avl_t *tree, mr_avl_t node);
void mr_avl_remove(mr_avl_t *tree, mr_avl_t node);
mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value);
mr_size_t mr_avl_get_length(mr_avl_t tree);

#endif