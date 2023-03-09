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
#include <mrserve.h>

mr_container_t mr_container_find(enum mr_container_type type);

mr_object_t mr_object_find(const char *name, enum mr_container_type type);
void mr_object_init(mr_object_t object, const char *name);
mr_object_t mr_object_create(const char *name);
mr_err_t mr_object_add_to_container(mr_object_t object, enum mr_container_type container_type);
mr_err_t mr_object_remove_from_container(mr_object_t object);
mr_err_t mr_object_move(mr_object_t object, enum mr_container_type dest_type);
void mr_object_rename(mr_object_t object, char *name);
mr_bool_t mr_object_is_static(mr_object_t object);

void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner);

void mr_ringbuffer_init(mr_ringbuffer_t ringbuffer, mr_uint8_t *pool, mr_size_t pool_size);
mr_size_t mr_ringbuffer_get_data_length(mr_ringbuffer_t ringbuffer);
mr_size_t mr_ringbuffer_write(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_write_byte(mr_ringbuffer_t ringbuffer, mr_uint8_t data);
mr_size_t mr_ringbuffer_write_byte_force(mr_ringbuffer_t ringbuffer, mr_uint8_t data);
mr_size_t mr_ringbuffer_write_force(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_read(mr_ringbuffer_t ringbuffer, mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_read_byte(mr_ringbuffer_t ringbuffer, mr_uint8_t *data);

mr_device_t mr_device_find(const char *name);
void mr_device_init(mr_device_t device, const char *name);
mr_err_t mr_device_add_to_container(mr_device_t device,
                                    enum mr_device_type type,
                                    mr_uint16_t support_flag,
                                    const struct mr_device_ops *ops,
                                    void *data);
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
mr_err_t mr_device_close(mr_device_t device);
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
mr_size_t mr_device_read(mr_device_t device, mr_off_t pos, void *buf, mr_size_t count);
mr_size_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buf, mr_size_t count);

#define MR_LOG_D(string)  \
    printf("[ DEBUG ] [ %s ][ %d ][ %s ] { %s }\r\n", \
    __FILE__,                 \
    __LINE__,                 \
    __FUNCTION__,              \
    string                      \
    )

#define MR_LOG_W(string)    \
    printf("[ WARING ] [ %s ][ %d ][ %s ] { %s }\r\n", \
    __FILE__,                 \
    __LINE__,                 \
    __FUNCTION__,             \
    string                      \
    )

#define MR_LOG_E(string, error) \
    if (error != MR_ERR_OK)     \
    {                           \
        printf("[ ERROR ] [ %s ][ %d ][ %s ] { %s }{ %d }\r\n", \
        __FILE__,                 \
        __LINE__,                 \
        __FUNCTION__,             \
        string,                    \
        error);   \
    }

#endif
