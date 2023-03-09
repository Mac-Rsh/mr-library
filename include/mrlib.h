//
// Created by 10632 on 2023/3/8.
//

#ifndef DESIGN_MR_LIBRARY_INCLUDE_MRLIB_H_
#define DESIGN_MR_LIBRARY_INCLUDE_MRLIB_H_

#include <mrdef.h>
#include <mrserve.h>

mr_container_t mr_container_find(enum mr_container_type type);

mr_object_t mr_object_find(const char *name, enum mr_container_type type);
void mr_object_init(mr_object_t object, const char *name);
mr_object_t mr_object_create(const char *name);
mr_err_t mr_object_add_to_container(mr_object_t object, enum mr_container_type container_type);
mr_err_t mr_object_remove_from_container(mr_object_t object);
mr_err_t mr_object_move(mr_object_t object, enum mr_container_type dest_type);
MR_INLINE void mr_object_rename(mr_object_t object, char *name)
{
    mr_strncpy(object->name, name, MR_NAME_MAX);
}
MR_INLINE mr_bool_t mr_object_is_static(mr_object_t object)
{
    if (object->type & MR_OBJECT_TYPE_STATIC)
        return MR_TRUE;
    else
        return MR_FALSE;
}

void mr_mutex_init(mr_mutex_t mutex);
mr_err_t mr_mutex_take(mr_mutex_t mutex, mr_object_t owner);
mr_err_t mr_mutex_release(mr_mutex_t mutex, mr_object_t owner);

void mr_ringbuffer_init(mr_ringbuffer_t ringbuffer, mr_uint8_t *pool, mr_size_t pool_size);
mr_size_t mr_ringbuffer_get_data_length(mr_ringbuffer_t ringbuffer);
mr_size_t mr_ringbuffer_put(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_putchar(mr_ringbuffer_t ringbuffer, mr_uint8_t data);
mr_size_t mr_ringbuffer_putchar_force(mr_ringbuffer_t ringbuffer, mr_uint8_t data);
mr_size_t mr_ringbuffer_put_force(mr_ringbuffer_t ringbuffer, const mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_get(mr_ringbuffer_t ringbuffer, mr_uint8_t *buffer, mr_size_t length);
mr_size_t mr_ringbuffer_getchar(mr_ringbuffer_t ringbuffer, mr_uint8_t *data);

#endif //DESIGN_MR_LIBRARY_INCLUDE_MRLIB_H_
