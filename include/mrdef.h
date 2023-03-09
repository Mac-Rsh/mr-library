/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-08     MacRsh       first version
 */

#ifndef _MR_DEF_H_
#define _MR_DEF_H_

#include <mrconfig.h>

#include "malloc.h"
#define mr_malloc                   malloc
#define mr_free                     free

#include "string.h"
#define mr_strncmp                  strncmp
#define mr_strncpy                  strncpy

#define mr_memset                   memset
#define mr_memcpy                   memcpy

/* mr-library version information */
#define MR_LIBRARY_VERSION          "0.0.1"

/* mr-library basic data_p type definitions */
typedef signed char mr_int8_t;                /**<  8bit integer type */
typedef signed short mr_int16_t;              /**< 16bit integer type */
typedef signed int mr_int32_t;                /**< 32bit integer type */
typedef unsigned char mr_uint8_t;             /**<  8bit unsigned integer type */
typedef unsigned short mr_uint16_t;           /**< 16bit unsigned integer type */
typedef unsigned int mr_uint32_t;             /**< 32bit unsigned integer type */
typedef float mr_float;
typedef double mr_double;

typedef signed long long mr_int64_t;          /**< 64bit integer type */
typedef unsigned long long mr_uint64_t;       /**< 64bit unsigned integer type */

typedef unsigned int mr_size_t;               /**< Type for size number */
typedef signed int mr_base_t;                 /**< Nbit CPU related date type */
typedef unsigned int mr_ubase_t;              /**< Nbit unsigned CPU related cs_data type */

typedef mr_base_t mr_err_t;                   /**< Type for error number */
typedef mr_base_t mr_off_t;                   /**< Type for offset */

#define MR_NULL                     0

/* mr-library error code definitions */
#define MR_ERR_OK                   0                         /**< There is no error */
#define MR_ERR_GENERIC              1                         /**< A generic error happens */
#define MR_ERR_OVERFLOW             2                         /**< Overflow */
#define MR_ERR_NOMEM                3                         /**< No memory */
#define MR_ERR_IO                   4                         /**< IO error */
#define MR_ERR_INVALID              5                         /**< Invalid argument */
#define MR_ERR_TIMEOUT              6                         /**< Timed out */
#define MR_ERR_BUSY                 7                         /**< Busy */
#define MR_ERR_NOT_FOUND            8                         /**< Not found */
#define MR_ERR_UNSUPPORTED          9                         /**< Unsupported feature */


/* Compiler Related Definitions */
#if defined(__ARMCC_VERSION)
#define MR_SECTION(x)               __attribute__((section(x)))
#define MR_USED                     __attribute__((used))
#define MR_ALIGN(n)                 __attribute__((aligned(n)))
#define MR_WEAK                     __attribute__((weak))
#define MR_INLINE                   static __inline

#elif defined (__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)               @ x
#define MR_USED                     __root
#define PRAGMA(x)                   _Pragma(#x)
#define MR_ALIGN(n)                    PRAGMA(data_alignment=n)
#define MR_WEAK                     __weak
#define MR_INLINE                   static inline

#elif defined (__GNUC__)
#define MR_SECTION(x)               __attribute__((section(x)))
#define MR_USED                     __attribute__((used))
#define MR_ALIGN(n)                 __attribute__((aligned(n)))
#define MR_WEAK                     __attribute__((weak))
#define MR_INLINE                   static __inline

#elif defined (__ADSPBLACKFIN__)
#define MR_SECTION(x)               __attribute__((section(x)))
#define MR_USED                     __attribute__((used))
#define MR_ALIGN(n)                 __attribute__((aligned(n)))
#define MR_WEAK                     __attribute__((weak))
#define MR_INLINE                   static inline

#elif defined (_MSC_VER)
#define MR_SECTION(x)
#define MR_USED
#define MR_ALIGN(n)                 __declspec(align(n))
#define MR_WEAK
#define MR_INLINE                   static __inline

#elif defined (__TASKING__)
#define MR_SECTION(x)               __attribute__((section(x)))
#define MR_USED                     __attribute__((used, protect))
#define mrAGMA(x)                   _Pragma(#x)
#define MR_ALIGN(n)                 __attribute__((__align(n)))
#define MR_WEAK                     __attribute__((weak))
#define MR_INLINE                   static inline
#endif

typedef enum mr_bool
{
    MR_FALSE = 0, MR_TRUE = 1
} mr_bool_t;

typedef enum mr_level
{
    MR_LOW = 0, MR_HIGH = 1
} mr_level_t;

struct mr_list
{
    struct mr_list *next;                                     /**< point to next node. */
    struct mr_list *prev;                                     /**< point to prev node. */
};
typedef struct mr_list *mr_list_t;

enum mr_container_type
{
    MR_CONTAINER_TYPE_MISC,
    MR_CONTAINER_TYPE_DEVICE,
    _MR_CONTAINER_TYPE_MASK,
};

struct mr_container
{
    enum mr_container_type type;
    struct mr_list list;
};
typedef struct mr_container *mr_container_t;

#define MR_OBJECT_TYPE_NULL         0x00
#define MR_OBJECT_TYPE_REGISTER     0x40
#define MR_OBJECT_TYPE_STATIC       0x80

struct mr_object
{
    char name[MR_NAME_MAX];
    mr_uint8_t type;
    struct mr_list list;
};
typedef struct mr_object *mr_object_t;

struct mr_mutex
{
    enum
    {
        MR_UNLOCK = 0,
        MR_LOCK,
    } lock;
    mr_object_t owner;
};
typedef struct mr_mutex *mr_mutex_t;

enum mr_ringbuffer_state
{
    MR_RINGBUFFER_EMPTY,
    MR_RINGBUFFER_FULL,
    MR_RINGBUFFER_HALF_FULL,
};

struct mr_ringbuffer
{
    mr_uint8_t *buffer;
    mr_uint16_t read_mirror: 1;
    mr_uint16_t read_index: 15;
    mr_uint16_t write_mirror: 1;
    mr_uint16_t write_index: 15;
    mr_uint16_t buffer_size;
};
typedef struct mr_ringbuffer *mr_ringbuffer_t;

enum mr_device_type
{
    MR_DEVICE_TYPE_NULL,
    MR_DEVICE_TYPE_PIN,
    MR_DEVICE_TYPE_SPI_BUS,
    MR_DEVICE_TYPE_SPI,
    MR_DEVICE_TYPE_I2C_BUS,
    MR_DEVICE_TYPE_I2C,
    MR_DEVICE_TYPE_UART,
    MR_DEVICE_TYPE_ADC,
    MR_DEVICE_TYPE_DAC,

    MR_DEVICE_TYPE_PWM,
    MR_DEVICE_TYPE_TIMER,
    MR_DEVICE_TYPE_WDT,
    MR_DEVICE_TYPE_FLASH,
    MR_DEVICE_TYPE_SDRAM,
    /* ... */

    MR_DEVICE_TYPE_LINK,
    MR_DEVICE_TYPE_LINK_SERVICE,
};

typedef struct mr_device *mr_device_t;
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t this);
    mr_err_t (*close)(mr_device_t this);
    mr_err_t (*ioctl)(mr_device_t this, int cmd, void *args);
    mr_size_t (*read)(mr_device_t this, mr_off_t pos, void *buffer, mr_size_t count);
    mr_size_t (*write)(mr_device_t this, mr_off_t pos, const void *buffer, mr_size_t count);
};

struct mr_device
{
    struct mr_object object;

    mr_err_t (*rx_callback)(mr_device_t this, void *args);
    mr_err_t (*tx_callback)(mr_device_t this, void *args);
    enum mr_device_type type;
    mr_uint16_t support_flag;
    mr_uint16_t open_flag;
    mr_uint8_t ref_count;
    void *data;
};

#endif
