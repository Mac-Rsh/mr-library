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
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @version mr-library version
 */
#define MR_LIBRARY_VERSION              "0.0.5"

/**
 * @def Compiler related
 */
#if defined(__ARMCC_VERSION)
#define MR_SECTION(x)              		__attribute__((section(x)))
#define MR_USED                    		__attribute__((used))
#define MR_ALIGN(n)                		__attribute__((aligned(n)))
#define MR_WEAK                    		__attribute__((weak))
#define MR_INLINE                  		static __inline
#elif defined (__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)               	@ x
#define MR_USED                     	__root
#define mr_pragma(x)                   	_Pragma(#x)
#define MR_ALIGN(n)                 	PRAGMA(data_alignment=n)
#define MR_WEAK                     	__weak
#define MR_INLINE                   	static inline
#elif defined (__GNUC__)
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_ALIGN(n)                     __attribute__((aligned(n)))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#elif defined (__ADSPBLACKFIN__)
#define MR_SECTION(x)               	__attribute__((section(x)))
#define MR_USED                     	__attribute__((used))
#define MR_ALIGN(n)                 	__attribute__((aligned(n)))
#define MR_WEAK                     	__attribute__((weak))
#define MR_INLINE                   	static inline

#elif defined (_MSC_VER)
#define MR_SECTION(x)
#define MR_USED
#define MR_ALIGN(n)                 	__declspec(align(n))
#define MR_WEAK
#define MR_INLINE                   	static __inline

#elif defined (__TASKING__)
#define MR_SECTION(x)               	__attribute__((section(x)))
#define MR_USED                     	__attribute__((used, protect))
#define MR_ALIGN(n)                 	__attribute__((__align(n)))
#define MR_WEAK                     	__attribute__((weak))
#define MR_INLINE                   	static inline
#endif

/**
 * @def Basic data type
 */
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
typedef mr_base_t mr_off_t;                                         /* Type for offset */
typedef mr_int8_t mr_bool_t;                                        /* Type for boolean */
typedef mr_int8_t mr_level_t;                                       /* Type for level */
typedef mr_int8_t mr_state_t;                                       /* Type for state */

#define MR_UINT8_MAX                    0xff                        /* Maximum unsigned 8bit integer */
#define MR_UINT16_MAX                   0xffff                      /* Maximum unsigned 16bit integer */
#define MR_UINT32_MAX                   0xffffffff                  /* Maximum unsigned 32bit integer */

#define MR_INT8_MAX                     0x7f                        /* Maximum signed 8bit integer */
#define MR_INT16_MAX                    0x7fff                      /* Maximum signed 16bit integer */
#define MR_INT32_MAX                    0x7fffffff                  /* Maximum signed 32bit integer */

/**
 * @def Null pointer
 */
#define MR_NULL                         (void *)0                   /* Null pointer */

/**
 * @def Boolean value
 */
#define MR_FALSE                        0                           /* False */
#define MR_TRUE                         1                           /* True */

/**
 * @def Level value
 */
#define MR_LOW                          0                           /* Low level */
#define MR_HIGH                         1                           /* High level */

/**
 * @def State value
 */
#define MR_DISABLE                      0                           /* Disable */
#define MR_ENABLE                       1                           /* Enable */

/**
 * @def Error code
 */
#define MR_ERR_OK                       0                           /* There is no error */
#define MR_ERR_GENERIC                  1                           /* Generic error happens */
#define MR_ERR_NO_MEMORY                2                           /* No memory */
#define MR_ERR_IO                       3                           /* IO error */
#define MR_ERR_TIMEOUT                  4                           /* Timed out */
#define MR_ERR_BUSY                     5                           /* Busy */
#define MR_ERR_NOT_FOUND                6                           /* Not found */
#define MR_ERR_UNSUPPORTED              7                           /* Unsupported feature */
#define MR_ERR_INVALID                  8                           /* Invalid parameter */

/**
 * @addtogroup Auto init
 * @{
 */
#if (MR_CFG_AUTO_INIT == MR_CFG_ENABLE)

typedef int (*mr_init_fn_t)(void);

/**
 * @def Auto-init export
 */
#define MR_INIT_EXPORT(fn, level) \
    MR_USED const mr_init_fn_t _mr_auto_init_##fn MR_SECTION(".auto_init."level) = fn

/**
 * @def Driver auto-init export
 */
#define MR_INIT_DRIVER_EXPORT(fn)       MR_INIT_EXPORT(fn, "1")

/**
 * @def Device auto-init export
 */
#define MR_INIT_DEVICE_EXPORT(fn)       MR_INIT_EXPORT(fn, "2")

/**
 * @def Module auto-init export
 */
#define MR_INIT_MODULE_EXPORT(fn)       MR_INIT_EXPORT(fn, "3")

/**
 * @def App auto-init export
 */
#define MR_INIT_APP_EXPORT(fn)          MR_INIT_EXPORT(fn, "4")

#else
#define MR_INIT_DRIVER_EXPORT(fn)
#define MR_INIT_DEVICE_EXPORT(fn)
#define MR_INIT_MODULE_EXPORT(fn)
#define MR_INIT_APP_EXPORT(fn)
#endif
/** @} */

/**
 * @struct Single list
 */
struct mr_slist
{
    struct mr_slist *next;                                          /* Point to next node */
};
typedef struct mr_slist *mr_slist_t;                                /* Type for slist */

/**
 * @struct Double list
 */
struct mr_list
{
    struct mr_list *next;                                           /* Point to next node */
    struct mr_list *prev;                                           /* Point to prev node */
};
typedef struct mr_list *mr_list_t;                                  /* Type for list */

/**
 * @struct AVL tree
 */
struct mr_avl
{
    mr_int32_t height;                                              /* Balance factor */
    mr_uint32_t value;                                              /* Key-hold */

    struct mr_avl *left_child;                                      /* Point to left-child node */
    struct mr_avl *right_child;                                     /* Point to right-child node */
};
typedef struct mr_avl *mr_avl_t;                                    /* Type for avl-tree */

/**
 * @struct Ring buffer
 */
struct mr_rb
{
    mr_uint8_t *buffer;                                             /* Buffer pool */
    mr_uint16_t size;                                               /* Buffer pool size */
    mr_uint16_t read_mirror: 1;                                     /* Read mirror flag */
    mr_uint16_t write_mirror: 1;                                    /* Write mirror flag */
    mr_uint16_t reserved: 14;                                       /* Reserved */
    mr_uint16_t read_index;                                         /* Read index */
    mr_uint16_t write_index;                                        /* Write index */
};
typedef struct mr_rb *mr_rb_t;                                      /* Type for ring buffer */

/**
 * @addtogroup Object
 * @{
 */

/**
 * @enum Object type
 */
enum mr_object_type
{
    Mr_Object_Type_None = 0,                                        /* None object */
    Mr_Object_Type_Device,                                          /* Device object */
    Mr_Object_Type_Module,                                          /* Module object */
};

/**
 * @def Object magic number
 */
#define MR_OBJECT_MAGIC                 0x6D72                      /* Mr object magic number */

/**
 * @struct Object container
 */
struct mr_object_container
{
    mr_uint16_t type;                                               /* Object type */
    mr_uint16_t magic;                                              /* Object magic number */
    struct mr_list list;                                            /* Container list */
};
typedef struct mr_object_container *mr_object_container_t;          /* Type for container */

/**
 * @struct Object
 */
struct mr_object
{
    char name[MR_CFG_OBJECT_NAME_SIZE];                             /* Object name */
    mr_uint16_t type;                                               /* Object type */
    mr_uint16_t magic;                                              /* Object magic number */
    struct mr_list list;                                            /* Object list */
};
typedef struct mr_object *mr_object_t;                              /* Type for object */
/** @} */

/**
 * @struct Mutex
 */
struct mr_mutex
{
    volatile mr_size_t hold;                                        /* Mutex hold count */
    volatile void *owner;                                           /* Mutex owner */
};
typedef struct mr_mutex *mr_mutex_t;                                /* Type for mutex */

/**
 * @addtogroup Device
 * @{
 */
#if (MR_CFG_DEVICE == MR_CFG_ENABLE)

/**
 * @enum Device type
 */
enum mr_device_type
{
    Mr_Device_Type_None = 0,                                        /* None device */
    Mr_Device_Type_Pin,                                             /* GPIO device */
    Mr_Device_Type_SPIBUS,                                          /* SPI-BUS device */
    Mr_Device_Type_SPI,                                             /* SPI device */
    Mr_Device_Type_I2CBUS,                                          /* I2C-BUS device */
    Mr_Device_Type_I2C,                                             /* I2C device */
    Mr_Device_Type_Serial,                                          /* UART device */
    Mr_Device_Type_ADC,                                             /* ADC device */
    Mr_Device_Type_DAC,                                             /* DAC device */
    Mr_Device_Type_PWM,                                             /* PWM device */
    Mr_Device_Type_Timer,                                           /* TIMER device */
    Mr_Device_Type_Flash,                                           /* FLASH device */
    Mr_Device_Type_MSGBUS,                                          /* MSG-BUS device */
    Mr_Device_Type_MSG,                                             /* MSG device */
    /* ... */
};

/**
 * @def Device open mode flags
 */
#define MR_DEVICE_OFLAG_CLOSED          0x00                        /* Closed */
#define MR_DEVICE_OFLAG_RDONLY          0x10                        /* Read only */
#define MR_DEVICE_OFLAG_WRONLY          0x20                        /* Write only */
#define MR_DEVICE_OFLAG_RDWR            0x30                        /* Read and write */
#define MR_DEVICE_OFLAG_NONBLOCKING     0x40                        /* Non-blocking */
#define MR_DEVICE_OFLAG_BUS             0x80                        /* Bus mode */
#define MR_DEVICE_OFLAG_MASK            0xf0                        /* Mask for open mode flag */

/**
 * @def Device control flags
 */
#define MR_DEVICE_CTRL_NONE             0x00000000                  /* None control */
#define MR_DEVICE_CTRL_SET_CONFIG       0x10000000                  /* Set configure */
#define MR_DEVICE_CTRL_GET_CONFIG       0x20000000                  /* Get configure */
#define MR_DEVICE_CTRL_SET_RX_CB        0x30000000                  /* Set receive callback */
#define MR_DEVICE_CTRL_SET_TX_CB        0x40000000                  /* Set transmit callback */
#define MR_DEVICE_CTRL_SET_RX_BUFSZ     0x50000000                  /* Set receive buffer size */
#define MR_DEVICE_CTRL_SET_TX_BUFSZ     0x60000000                  /* Set transmit buffer size */
#define MR_DEVICE_CTRL_CONNECT          0x70000000                  /* Connect device */

typedef struct mr_device *mr_device_t;                              /* Type for device */
typedef mr_err_t (*mr_device_cb_t)(mr_device_t device, void *args); /* Type for device callback */

/**
 * @struct Device operations
 */
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*read)(mr_device_t device, mr_off_t off, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_off_t off, const void *buffer, mr_size_t size);
};

/**
 * @struct Device
 */
struct mr_device
{
    struct mr_object object;                                        /* Device object */

    mr_uint8_t type;                                                /* Device type */
    mr_uint8_t sflags;                                              /* Support open mode flags */
    mr_uint8_t oflags;                                              /* Open mode flags */
    mr_uint8_t reserved;                                            /* Reserved */
    mr_size_t ref_count;                                            /* Number of references */
    mr_err_t (*rx_cb)(mr_device_t device, void *args);              /* Receive the completed callback */
    mr_err_t (*tx_cb)(mr_device_t device, void *args);              /* Send completion callback */

    const struct mr_device_ops *ops;                                /* Operations */
    void *data;                                                     /* Device data */
};

/**
 * @struct Device channel
 */
struct mr_device_channel
{
    union
    {
        struct
        {
            mr_uint32_t ch0: 1;
            mr_uint32_t ch1: 1;
            mr_uint32_t ch2: 1;
            mr_uint32_t ch3: 1;
            mr_uint32_t ch4: 1;
            mr_uint32_t ch5: 1;
            mr_uint32_t ch6: 1;
            mr_uint32_t ch7: 1;
            mr_uint32_t ch8: 1;
            mr_uint32_t ch9: 1;
            mr_uint32_t ch10: 1;
            mr_uint32_t ch11: 1;
            mr_uint32_t ch12: 1;
            mr_uint32_t ch13: 1;
            mr_uint32_t ch14: 1;
            mr_uint32_t ch15: 1;
            mr_uint32_t ch16: 1;
            mr_uint32_t ch17: 1;
            mr_uint32_t ch18: 1;
            mr_uint32_t ch19: 1;
            mr_uint32_t ch20: 1;
            mr_uint32_t ch21: 1;
            mr_uint32_t ch22: 1;
            mr_uint32_t ch23: 1;
            mr_uint32_t ch24: 1;
            mr_uint32_t ch25: 1;
            mr_uint32_t ch26: 1;
            mr_uint32_t ch27: 1;
            mr_uint32_t ch28: 1;
            mr_uint32_t ch29: 1;
            mr_uint32_t ch30: 1;
            mr_uint32_t ch31: 1;
        };
        mr_uint32_t _mask;
    };
};
typedef struct mr_device_channel *mr_device_channel_t;              /* Type for device channel */

#endif
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _MR_DEF_H_ */