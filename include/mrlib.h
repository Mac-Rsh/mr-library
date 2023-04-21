/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-08     MacRsh       first version
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
 *  Export fifo functions
 */
void mr_fifo_init(mr_fifo_t fifo, mr_uint8_t *pool, mr_size_t pool_size);
void mr_fifo_reset(mr_fifo_t fifo);
mr_size_t mr_fifo_get_length(mr_fifo_t fifo);
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
mr_size_t mr_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
mr_size_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);

/**
 *  Export event functions
 */
mr_event_manager_t mr_event_manager_find(const char *name);
mr_err_t mr_event_manager_add(mr_event_manager_t manager,
							  const char *name,
							  enum mr_event_manager_type type,
							  mr_uint8_t *pool,
							  mr_size_t pool_size);
mr_err_t mr_event_manager_remove(mr_event_manager_t manager);
mr_err_t mr_event_manager_notify(mr_event_manager_t manager, mr_uint32_t value);
mr_err_t mr_event_manager_handler(mr_event_manager_t manager);

mr_event_t mr_event_find(mr_event_manager_t manager, mr_uint32_t value);
mr_err_t mr_event_add(mr_event_manager_t manager,
					  mr_event_t event,
					  mr_uint32_t value,
					  mr_err_t (*callback)(mr_event_manager_t event_manager, void *args), void *args);
mr_err_t mr_event_remove(mr_event_manager_t manager, mr_event_t event);
mr_err_t mr_event_create(mr_event_manager_t manager,
						 mr_uint32_t value,
						 mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
						 void *args);
mr_err_t mr_event_delete(mr_event_manager_t manager, mr_event_t event);

/**
 *  Export finite state machine(FSM) functions
 */
mr_fsm_manager_t mr_fsm_manager_find(const char *name);
mr_err_t mr_fsm_manager_add(mr_fsm_manager_t manager, const char *name, mr_uint32_t state);
mr_err_t mr_fsm_create(mr_fsm_manager_t manager,
					   mr_uint32_t state,
					   mr_err_t (*callback)(mr_event_manager_t event_manager, void *args),
					   void *args);
mr_err_t mr_fsm_manager_transfer_state(mr_fsm_manager_t manager, mr_uint32_t state);
mr_err_t mr_fsm_manager_handler(mr_fsm_manager_t manager);

#endif
