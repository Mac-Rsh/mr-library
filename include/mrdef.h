/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-08     MacRsh       first version
 */

#ifndef _MR_DEF_H_
#define _MR_DEF_H_

#include "mrconfig.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#define mr_malloc                       malloc
#define mr_free                         free
#define mr_strncmp                      strncmp
#define mr_strncpy                      strncpy
#define mr_memset                       memset
#define mr_memcpy                       memcpy
#define mr_strlen                       strlen
#define mr_vsnprintf                    vsnprintf
#define mr_snprintf                     snprintf
#define mr_sscanf                       sscanf

/* mr-library version information */
#define MR_LIBRARY_VERSION              "0.0.2"

/* Compiler Related Definitions */
#if defined(__ARMCC_VERSION)
#define mr_section(x)              		__attribute__((section(x)))
#define mr_used                    		__attribute__((used))
#define mr_align(n)                		__attribute__((aligned(n)))
#define mr_weak                    		__attribute__((weak))
#define mr_inline                  		static __inline

#elif defined (__IAR_SYSTEMS_ICC__)
#define mr_section(x)               	@ x
#define mr_used                     	__root
#define mr_pragma(x)                   	_Pragma(#x)
#define mr_align(n)                 	PRAGMA(data_alignment=n)
#define mr_weak                     	__weak
#define mr_inline                   	static inline

#elif defined (__GNUC__)
#define mr_section(x)                   __attribute__((section(x)))
#define mr_used                         __attribute__((used))
#define mr_align(n)                     __attribute__((aligned(n)))
#define mr_weak                         __attribute__((weak))
#define mr_inline                       static __inline

#elif defined (__ADSPBLACKFIN__)
#define mr_section(x)               	__attribute__((section(x)))
#define mr_used                     	__attribute__((used))
#define mr_align(n)                 	__attribute__((aligned(n)))
#define mr_weak                     	__attribute__((weak))
#define mr_inline                   	static inline

#elif defined (_MSC_VER)
#define mr_section(x)
#define mr_used
#define mr_align(n)                 	__declspec(align(n))
#define mr_weak
#define mr_inline                   	static __inline

#elif defined (__TASKING__)
#define mr_section(x)               	__attribute__((section(x)))
#define mr_used                     	__attribute__((used, protect))
#define mr_align(n)                 	__attribute__((__align(n)))
#define mr_weak                     	__attribute__((weak))
#define mr_inline                   	static inline
#endif

/* mr-library error code definitions */
#define MR_ERR_OK                       0                           /* There is no error */
#define MR_ERR_GENERIC                  1                           /* A generic error happens */
#define MR_ERR_NO_MEMORY                2                           /* No memory */
#define MR_ERR_IO                       3                           /* IO error */
#define MR_ERR_TIMEOUT                  4                           /* Timed out */
#define MR_ERR_BUSY                     5                           /* Busy */
#define MR_ERR_NOT_FOUND                6                           /* Not found */
#define MR_ERR_UNSUPPORTED              7                           /* Unsupported feature */
#define MR_ERR_INVALID                  8                           /* Invalid parameter */

/* mr-library basic open flag definitions */
#define MR_OPEN_CLOSED                  0x0000                      /* Closed */
#define MR_OPEN_RDONLY                  0x1000                      /* Read only */
#define MR_OPEN_WRONLY                  0x2000                      /* Write only */
#define MR_OPEN_RDWR                    0x3000                      /* Read and write */
#define MR_OPEN_NONBLOCKING             0x4000                      /* Non-blocking */
#define MR_OPEN_ACTIVE                  0x8000                      /* Active */
#define MR_OPEN_DMA                     0x0100                      /* DMA */
#define _MR_OPEN_FLAG_MASK              0xf000                      /* Mask for getting open flag */

/* mr-library basic control flag definitions */
#define MR_CTRL_NONE                    0x0000                      /* No control */
#define MR_CTRL_SET_CONFIG              0x1000                      /* Set configure */
#define MR_CTRL_GET_CONFIG              0x2000                      /* Get configure */
#define MR_CTRL_SET_RX_CB               0x3000                      /* Set receive callback */
#define MR_CTRL_SET_TX_CB               0x4000                      /* Set transmit callback */
#define MR_CTRL_SET_RX_BUFSZ            0x5000                      /* Set receive buffer size */
#define MR_CTRL_SET_TX_BUFSZ            0x6000                      /* Set transmit buffer size */
#define MR_CTRL_ATTACH                  0x7000                      /* Attach the bus */
#define MR_CTRL_TRANSFER                0x8000                      /* Transfer */
#define MR_CTRL_REBOOT                  0x9000                      /* Reboot */
#define _MR_CTRL_FLAG_MASK              0xf000                      /* Mask for getting control flag */

/* mr-library basic data type definitions */
#define MR_NULL                         0                           /* Null pointer */
#define MR_FALSE                        0                           /* False */
#define MR_TRUE                         1                           /* True */
#define MR_LOW                          0                           /* Low level */
#define MR_HIGH                         1                           /* High level */
#define MR_DISABLE                      0                           /* Disable */
#define MR_ENABLE                       1                           /* Enable */
#define MR_UNLOCK                       0                           /* Unlock */
#define MR_LOCK                         1                           /* Lock */

#define MR_UINT8_MAX                    0xff                        /* Maximum unsigned 8bit integer */
#define MR_UINT16_MAX                   0xffff                      /* Maximum unsigned 16bit integer */
#define MR_UINT32_MAX                   0xffffffff                  /* Maximum unsigned 32bit integer */

typedef signed char mr_int8_t;                                      /* Type for 8bit integer */
typedef signed short mr_int16_t;                                    /* Type for 16bit integer */
typedef signed int mr_int32_t;                                      /* Type for 32bit integer */
typedef signed long long mr_int64_t;                                /* Type for 64bit integer */
typedef unsigned char mr_uint8_t;                                   /* Type for 8bit unsigned integer */
typedef unsigned short mr_uint16_t;                                 /* Type for 16bit unsigned integer */
typedef unsigned int mr_uint32_t;                                   /* Type for 32bit unsigned integer */
typedef unsigned long long mr_uint64_t;                             /* Type for 64bit unsigned integer */
typedef float mr_fp32_t;                                            /* Type for single-precision floating point */
typedef double mr_fp64_t;                                           /* Type for double-precision floating point */

typedef signed int mr_base_t;                                       /* Type for Nbit CPU related date */
typedef unsigned int mr_ubase_t;                                    /* Type for Nbit unsigned CPU related data */
typedef mr_ubase_t mr_size_t;                                       /* Type for size number */
typedef mr_base_t mr_ssize_t;                                       /* Type for signed size number */
typedef mr_base_t mr_err_t;                                         /* Type for error number */
typedef mr_base_t mr_pos_t;                                         /* Type for position */
typedef mr_int8_t mr_bool_t;                                        /* Type for boolean */
typedef mr_int8_t mr_level_t;                                       /* Type for level */
typedef mr_int8_t mr_state_t;                                       /* Type for state */
typedef mr_int8_t mr_lock_t;                                        /* Type for lock */

/**
 *  Auto-Init
 */
typedef int (*init_fn_t)(void);

#define AUTO_INIT_EXPORT(fn, level) \
    mr_used const init_fn_t _mr_auto_init_##fn mr_section(".auto_init."level) = fn

#define AUTO_INIT_DRIVER_EXPORT(fn)     AUTO_INIT_EXPORT(fn, "1")   /* Driver auto-init export */
#define AUTO_INIT_DEVICE_EXPORT(fn)     AUTO_INIT_EXPORT(fn, "2")   /* Device auto-init export */
#define AUTO_INIT_MODULE_EXPORT(fn)     AUTO_INIT_EXPORT(fn, "3")   /* Module auto-init export */

/**
 *  List
 */
struct mr_slist
{
    struct mr_slist *next;                                          /* Point to next node */
};
typedef struct mr_slist *mr_slist_t;                                /* Type for slist */

struct mr_list
{
    struct mr_list *next;                                           /* Point to next node */
    struct mr_list *prev;                                           /* Point to prev node */
};
typedef struct mr_list *mr_list_t;                                  /* Type for list */

/**
 *  Avl-tree
 */
struct mr_avl
{
    mr_int8_t height;                                               /* Balance factor */
    mr_uint32_t value;                                              /* Key-hold */

    struct mr_avl *left_child;                                      /* Point to left-child node */
    struct mr_avl *right_child;                                     /* Point to right-child node */
};
typedef struct mr_avl *mr_avl_t;                                    /* Type for avl-tree */

/**
 *  Fifo
 */
struct mr_fifo
{
    mr_uint16_t read_mirror: 1;                                     /* Read mirror flag */
    mr_uint16_t read_index: 15;                                     /* Read index */
    mr_uint16_t write_mirror: 1;                                    /* Write mirror flag */
    mr_uint16_t write_index: 15;                                    /* Write index */

    mr_uint16_t size;                                               /* Buffer pool size */
    mr_uint8_t *buffer;                                             /* Buffer pool */
};
typedef struct mr_fifo *mr_fifo_t;                                  /* Type for fifo */

/**
 *  Transfer
 */
struct mr_transfer
{
    void *data;                                                     /* Transfer data */
    mr_size_t size;                                                 /* Transfer size */

    struct mr_transfer *next;                                       /* Point to next transfer */
};
typedef struct mr_transfer *mr_transfer_t;                          /* Type for transfer */

/**
 *  Container
 */
struct mr_container
{
    mr_uint8_t type;                                                /* Object type */
    struct mr_list list;                                            /* Container list */
};
typedef struct mr_container *mr_container_t;                        /* Type for container */

/**
 *  Object
 */
enum mr_object_type
{
    MR_OBJECT_TYPE_NONE,                                            /* No object */
    MR_OBJECT_TYPE_DEVICE,                                          /* Device object */
    MR_OBJECT_TYPE_EVENT,                                           /* Event object */
    MR_OBJECT_TYPE_SOFT_TIMER,                                      /* Soft timer object */
    MR_OBJECT_TYPE_MODULE,                                          /* Module object */
};

struct mr_object
{
    char name[MR_CONF_NAME_MAX];                                    /* Object name */
    mr_uint8_t type;                                                /* Object type */
    struct mr_list list;                                            /* Object list */
};
typedef struct mr_object *mr_object_t;                              /* Type for object */

/**
 *  Mutex
 */
struct mr_mutex
{
    mr_uint16_t hold;                                               /* Mutex hold count of the same object */
    mr_object_t owner;                                              /* Mutex owns the object */
};
typedef struct mr_mutex *mr_mutex_t;                                /* Type for mutex */

#if (MR_CONF_DEVICE == MR_CONF_ENABLE)
/**
 *  Device
 */
enum mr_device_type
{
    MR_DEVICE_TYPE_NONE,                                            /* No device */
    MR_DEVICE_TYPE_PIN,                                             /* GPIO device */
    MR_DEVICE_TYPE_SPI_BUS,                                         /* SPI-BUS device */
    MR_DEVICE_TYPE_SPI,                                             /* SPI device */
    MR_DEVICE_TYPE_I2C_BUS,                                         /* I2C-BUS device */
    MR_DEVICE_TYPE_I2C,                                             /* I2C device */
    MR_DEVICE_TYPE_SERIAL,                                          /* UART device */
    MR_DEVICE_TYPE_ADC,                                             /* ADC device */
    MR_DEVICE_TYPE_DAC,                                             /* DAC device */
    MR_DEVICE_TYPE_PWM,                                             /* PWM device */
    MR_DEVICE_TYPE_TIMER,                                           /* TIMER device */
    MR_DEVICE_TYPE_FLASH,                                           /* FLASH device */
    /* ... */
};

typedef struct mr_device *mr_device_t;                              /* Type for device */

struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*read)(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
};

struct mr_device
{
    struct mr_object object;                                        /* Device object */

    enum mr_device_type type;                                       /* Device type */
    mr_uint16_t support_flag;                                       /* Open mode supported by the device */
    mr_uint16_t open_flag;                                          /* Opening mode of the device */
    mr_size_t ref_count;                                            /* Number of device references */
    void *data;                                                     /* Device data */

    mr_err_t (*rx_cb)(mr_device_t device, void *args);              /* Device receives the baud function */
    mr_err_t (*tx_cb)(mr_device_t device, void *args);              /* Device sends the baud function */

    const struct mr_device_ops *ops;                                /* Operations of the device */
};
#endif /* MR_CONF_DEVICE */

#if (MR_CONF_EVENT == MR_CONF_ENABLE)
/**
 *  Event server
 */
struct mr_event_server
{
    struct mr_object object;                                        /* Event server object */

    struct mr_fifo queue;                                           /* Event server queue */
    mr_avl_t list;                                                  /* Event server list */
};
typedef struct mr_event_server *mr_event_server_t;                  /* Type for event server */

typedef struct mr_event *mr_event_t;                                /* Type for event */

struct mr_event
{
    struct mr_avl list;                                             /* Event list */

    mr_err_t (*cb)(mr_event_server_t server, void *args);           /* Event callback */
    void *args;                                                     /* Event arguments */
};
#endif /* MR_CONF_EVENT */

#if (MR_CONF_SOFT_TIMER == MR_CONF_ENABLE)
/**
 *  Soft-timer server
 */
struct mr_soft_timer_server
{
    struct mr_object object;                                        /* Soft-timer server object */

    mr_uint32_t time;                                               /* Current time */
    struct mr_list run_list;                                        /* Soft-timer running list */
    mr_avl_t list;                                                  /* Soft-timer server list */
};
typedef struct mr_soft_timer_server *mr_soft_timer_server_t;        /* Type for soft-timer server */

typedef struct mr_soft_timer *mr_soft_timer_t;                      /* Type for soft-timer */

struct mr_soft_timer
{
    struct mr_avl list;                                             /* Timer list */
    struct mr_list run_list;                                        /* Timer running list */
    mr_uint32_t interval;                                           /* Timer interval time */
    mr_uint32_t timeout;                                            /* Timer timeout time */

    mr_err_t (*cb)(mr_soft_timer_server_t server, void *args);      /* Timer callback */
    void *args;                                                     /* Timer arguments */
};
#endif /* MR_CONF_SOFT_TIMER */

#endif /* _MR_DEF_H_ */