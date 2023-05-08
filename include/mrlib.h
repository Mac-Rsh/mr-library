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

#include <mrdef.h>
#include <mrservice.h>
#include <mrlog.h>

/**
 *  Export hardware functions
 */
void mr_hw_interrupt_disable(void);
void mr_hw_interrupt_enable(void);
void mr_hw_delay_ms(mr_uint32_t ms);

/**
 *  Export avl-tree functions
 */
void mr_avl_init(mr_avl_t node, mr_uint32_t value);
void mr_avl_insert(mr_avl_t *tree, mr_avl_t node);
void mr_avl_remove(mr_avl_t *tree, mr_avl_t node);
mr_avl_t mr_avl_find(mr_avl_t tree, mr_uint32_t value);
mr_size_t mr_avl_get_length(mr_avl_t tree);

/**
 *  Export kservice functions
 */
mr_uint32_t mr_strhase(const char *str);
mr_uint32_t mr_strnhase(const char *str, mr_size_t length);

/**
 *  Export fifo functions
 */
void mr_fifo_init(mr_fifo_t fifo, mr_uint8_t *pool, mr_size_t pool_size);
void mr_fifo_reset(mr_fifo_t fifo);
mr_size_t mr_fifo_get_length(mr_fifo_t fifo);
mr_size_t mr_fifo_get_size(mr_fifo_t fifo);
mr_size_t mr_fifo_read(mr_fifo_t fifo, mr_uint8_t *buffer, mr_size_t size);
mr_size_t mr_fifo_write(mr_fifo_t fifo, const mr_uint8_t *buffer, mr_size_t size);
mr_size_t mr_fifo_write_force(mr_fifo_t fifo, const mr_uint8_t *buffer, mr_size_t size);

/**
 *  Export container functions
 */
mr_container_t mr_container_find(enum mr_container_type type);

/**
 *  Export object functions
 */
mr_object_t mr_object_find(const char *name, enum mr_container_type type);
mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_container_type container_type);
mr_err_t mr_object_remove(mr_object_t object);
mr_err_t mr_object_move(mr_object_t object, enum mr_container_type dest_type);
void mr_object_rename(mr_object_t object, char *name);

/**
 *  Export mutex functions
 */
void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner);

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

/**
 *  Export manager functions
 */
mr_manager_t mr_manager_find(const char *name);
mr_err_t mr_manager_add(mr_manager_t manager,
						const char *name,
						enum mr_manager_type type,
						mr_size_t queue_number,
						mr_err_t (*err_cb)(struct mr_manager *manager, mr_uint32_t agent_id, mr_err_t err));
mr_err_t mr_manager_remove(mr_manager_t manager);
mr_err_t mr_manager_notify(mr_manager_t manager, mr_uint32_t agent_id);
void mr_manager_handler(mr_manager_t manager);
void mr_manager_at_isr(mr_manager_t manager, char data);

/**
 *  Export agent functions
 */
mr_agent_t mr_agent_find(mr_uint32_t agent_id, mr_manager_t manager);
mr_err_t mr_agent_create(mr_uint32_t agent_id,
						 mr_err_t (*callback)(mr_manager_t manager, void *args),
						 void *args,
						 mr_manager_t agent_manager);
mr_err_t mr_agent_delete(mr_uint32_t agent_id, mr_manager_t agent_manager);
mr_uint32_t mr_agent_str_to_id(const char *str);

#endif
