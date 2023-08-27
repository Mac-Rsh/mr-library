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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @version mr-library version
 */
#define MR_LIBRARY_VERSION              "0.0.3"

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
typedef mr_base_t mr_pos_t;                                         /* Type for position */
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
#define MR_NULL                         (void *)0                           /* Null pointer */

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
#if(MR_CFG_AUTO_INIT == MR_CFG_ENABLE)

typedef int (*init_fn_t)(void);

/**
 * @def Auto-init export
 */
#define MR_INIT_EXPORT(fn, level) \
    MR_USED const init_fn_t _mr_auto_init_##fn MR_SECTION(".auto_init."level) = fn

/**
 * @def Device auto-init export
 */
#define MR_INIT_DEVICE_EXPORT(fn)       MR_INIT_EXPORT(fn, "1")     /* Device auto-init export */

/**
 * @def Module auto-init export
 */
#define MR_INIT_MODULE_EXPORT(fn)       MR_INIT_EXPORT(fn, "2")     /* Module auto-init export */

/**
 * @def App auto-init export
 */
#define MR_INIT_APP_EXPORT(fn)          MR_INIT_EXPORT(fn, "3")     /* App auto-init export */

#else
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
    mr_int8_t height;                                               /* Balance factor */
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
    mr_uint16_t read_mirror: 1;                                     /* Read mirror flag */
    mr_uint16_t read_index: 15;                                     /* Read index */
    mr_uint16_t write_mirror: 1;                                    /* Write mirror flag */
    mr_uint16_t write_index: 15;                                    /* Write index */

    mr_uint16_t size;                                               /* Buffer pool size */
    mr_uint8_t *buffer;                                             /* Buffer pool */
};
typedef struct mr_rb *mr_rb_t;                                      /* Type for fifo */

/**
 * @struct Transfer
 */
struct mr_transfer
{
    void *data;                                                     /* Transfer data */
    mr_size_t size;                                                 /* Transfer size */

    struct mr_transfer *next;                                       /* Point to next transfer */
};
typedef struct mr_transfer *mr_transfer_t;                          /* Type for transfer */

/**
 * @addtogroup Object
 * @{
 */

/**
 * @enum Object type
 */
enum mr_object_type
{
    Mr_Object_Type_None,                                            /* No object */
    Mr_Object_Type_Device,                                          /* Device object */
    Mr_Object_Type_Fsm,                                             /* FSM object */
    Mr_Object_Type_Event,                                           /* Event object */
    Mr_Object_Type_SoftTimer,                                       /* Soft timer object */
    Mr_Object_Type_Module,                                          /* Module object */
};

/**
 * @struct Object container
 */
struct mr_object_container
{
    enum mr_object_type type;                                       /* Object type */
    struct mr_list list;                                            /* Container list */
};
typedef struct mr_object_container *mr_object_container_t;          /* Type for container */

/**
 * @struct Object
 */
struct mr_object
{
    char name[MR_CFG_OBJECT_NAME_SIZE];                             /* Object name */
    enum mr_object_type type;                                       /* Object type */
    struct mr_list list;                                            /* Object list */
};
typedef struct mr_object *mr_object_t;                              /* Type for object */
/** @} */

/**
 * @struct Mutex
 */
struct mr_mutex
{
    mr_size_t hold;                                                 /* Mutex hold count */
    void *owner;                                                    /* Mutex owner */
};
typedef struct mr_mutex *mr_mutex_t;                                /* Type for mutex */

/**
 * @addtogroup Finite State Machine
 * @{
 */
#if (MR_CFG_FSM == MR_CFG_ENABLE)

typedef struct mr_fsm_table *mr_fsm_table_t;                        /* Type for fsm table */

/**
 * @struct FSM
 */
struct mr_fsm
{
    struct mr_object object;                                        /* FSM object */

    mr_fsm_table_t table;                                           /* State table */
    mr_size_t table_size;                                           /* State table size */
    mr_uint32_t current_state;                                      /* Current state */
    mr_uint32_t next_state;                                         /* Next state */
};
typedef struct mr_fsm *mr_fsm_t;                                    /* Type for fsm */

/**
 * @struct FSM table
 */
struct mr_fsm_table
{
    mr_err_t (*cb)(mr_fsm_t fsm, void *args);                       /* State callback */
    void *args;                                                     /* State arguments */
    mr_err_t (*signal)(mr_fsm_t fsm, mr_uint32_t signal);           /* State signal */
};

#endif
/** @} */

/**
 * @addtogroup Event
 * @{
 */
#if (MR_CFG_EVENT == MR_CFG_ENABLE)

typedef struct mr_event_table *mr_event_table_t;                    /* Type for event */

/**
 * @struct Event
 */
struct mr_event
{
    struct mr_object object;                                        /* Event object */

    mr_event_table_t table;                                         /* Event table */
    mr_size_t table_size;                                           /* Event table size */
    struct mr_rb queue;                                             /* Event queue */
};
typedef struct mr_event *mr_event_t;                                /* Type for event */

/**
 * @struct Event table
 */
struct mr_event_table
{
    mr_err_t (*cb)(mr_event_t event, void *args);                    /* Event callback */
    void *args;                                                      /* Event arguments */
};

#endif
/** @} */

/**
 * @addtogroup Soft-timer
 * @{
 */
#if (MR_CFG_SOFT_TIMER == MR_CFG_ENABLE)

/**
 * @enum Soft-timer type
 */
enum mr_soft_timer_type
{
    Mr_Soft_Timer_Type_Period,                                      /* Periodic timer */
    Mr_Soft_Timer_Type_Oneshot,                                     /* One-shot timer */
};

/**
 * @struct Soft-timer data
 */
struct mr_soft_timer_data
{
    mr_uint32_t timeout;                                            /* Timeout time */
    struct mr_list timeout_list;                                    /* Timeout list */
};
typedef struct mr_soft_timer_data *mr_soft_timer_data_t;            /* Type for soft-timer data */

typedef struct mr_soft_timer_table *mr_soft_timer_table_t;          /* Type for soft-timer table */

/**
 * @struct Soft-timer
 */
struct mr_soft_timer
{
    struct mr_object object;                                        /* Soft-timer object */

    mr_soft_timer_table_t table;                                    /* Timer table */
    mr_size_t table_size;                                           /* Timer table size */
    mr_uint32_t current_time;                                       /* Current time */
    struct mr_list timeout_list;                                    /* Timeout list */
    mr_soft_timer_data_t data;                                      /* Timer data */
};
typedef struct mr_soft_timer *mr_soft_timer_t;                      /* Type for soft-timer */

/**
 * @struct Soft-timer table
 */
struct mr_soft_timer_table
{
    mr_uint32_t time;                                               /* Timer interval time */
    enum mr_soft_timer_type type;                                   /* Timer type */
    mr_err_t (*cb)(mr_soft_timer_t timer, void *args);              /* Timer callback */
    void *args;                                                     /* Timer arguments */
};

#endif
/** @} */

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
    Mr_Device_Type_None,                                            /* No device */
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
    /* ... */
};

/**
 * @def Device open mode flags
 */
#define MR_OPEN_CLOSED                  0x0000                      /* Closed */
#define MR_OPEN_RDONLY                  0x1000                      /* Read only */
#define MR_OPEN_WRONLY                  0x2000                      /* Write only */
#define MR_OPEN_RDWR                    0x3000                      /* Read and write */
#define MR_OPEN_NONBLOCKING             0x4000                      /* Non-blocking */
#define MR_OPEN_FLAG_MASK               0xf000                      /* Mask for open mode flag */

/**
 * @def Device control flags
 */
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
#define MR_CTRL_FLAG_MASK               0xf000                      /* Mask for control flag */

typedef struct mr_device *mr_device_t;                              /* Type for device */

/**
 * @struct Device operations
 */
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*read)(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
};

/**
 * @struct Device
 */
struct mr_device
{
    struct mr_object object;                                        /* Device object */

    enum mr_device_type type;                                       /* Device type */
    mr_uint16_t support_flag;                                       /* Open mode supported */
    mr_uint16_t open_flag;                                          /* Open mode */
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
            mr_pos_t ch0: 1;
            mr_pos_t ch1: 1;
            mr_pos_t ch2: 1;
            mr_pos_t ch3: 1;
            mr_pos_t ch4: 1;
            mr_pos_t ch5: 1;
            mr_pos_t ch6: 1;
            mr_pos_t ch7: 1;
            mr_pos_t ch8: 1;
            mr_pos_t ch9: 1;
            mr_pos_t ch10: 1;
            mr_pos_t ch11: 1;
            mr_pos_t ch12: 1;
            mr_pos_t ch13: 1;
            mr_pos_t ch14: 1;
            mr_pos_t ch15: 1;
            mr_pos_t ch16: 1;
            mr_pos_t ch17: 1;
            mr_pos_t ch18: 1;
            mr_pos_t ch19: 1;
            mr_pos_t ch20: 1;
            mr_pos_t ch21: 1;
            mr_pos_t ch22: 1;
            mr_pos_t ch23: 1;
            mr_pos_t ch24: 1;
            mr_pos_t ch25: 1;
            mr_pos_t ch26: 1;
            mr_pos_t ch27: 1;
            mr_pos_t ch28: 1;
            mr_pos_t ch29: 1;
            mr_pos_t ch30: 1;
            mr_pos_t ch31: 1;
        };
        mr_pos_t mask;
    };
};
typedef struct mr_device_channel *mr_device_channel_t;              /* Type for device channel */

#endif
/** @} */

#endif /* _MR_DEF_H_ */