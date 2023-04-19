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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <malloc.h>
#include <string.h>

#define mr_malloc                  malloc
#define mr_free                    free
#define mr_strncmp                 strncmp
#define mr_strncpy                 strncpy
#define mr_memset                  memset
#define mr_memcpy                  memcpy
#define mr_printf                  printf

/* Compiler Related Definitions */
#if defined(__ARMCC_VERSION)
#define MR_SECTION(x)              __attribute__((section(x)))
#define MR_USED                    __attribute__((used))
#define MR_ALIGN(n)                __attribute__((aligned(n)))
#define MR_WEAK                    __attribute__((weak))
#define MR_INLINE                  static __inline

#elif defined (__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)               @ x
#define MR_USED                     __root
#define PRAGMA(x)                   _Pragma(#x)
#define MR_ALIGN(n)                 PRAGMA(data_alignment=n)
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

/* mr-library version information */
#define MR_LIBRARY_VERSION         "0.0.2"

#define MR_NULL                    0

/* mr-library error code definitions */
#define MR_ERR_OK                  0                           /**< There is no error */
#define MR_ERR_GENERIC             1                           /**< A generic error happens */
#define MR_ERR_NO_MEMORY           2                           /**< No memory */
#define MR_ERR_IO                  3                           /**< IO error */
#define MR_ERR_TIMEOUT             4                           /**< Timed out */
#define MR_ERR_BUSY                5                           /**< Busy */
#define MR_ERR_NOT_FOUND           6                           /**< Not found */
#define MR_ERR_UNSUPPORTED         7                           /**< Unsupported feature */

/* mr-library basic open flag definitions */
#define MR_OPEN_CLOSED             0x0000                      /**< Closed */
#define MR_OPEN_RDONLY             0x1000                      /**< Read only */
#define MR_OPEN_WRONLY             0x2000                      /**< Write only */
#define MR_OPEN_RDWR               0x3000                      /**< Read and write */
#define MR_OPEN_NONBLOCKING        0x4000                      /**< Non-blocking */
#define MR_OPEN_ACTIVE             0x8000                      /**< Active */
#define _MR_OPEN_FLAG_MASK         0xf000                      /**< Mask for getting open flag */

/* mr-library basic command definitions */
#define MR_CMD_NULL                0x0000                      /**< Null command */
#define MR_CMD_CONFIG              0x1000                      /**< Configure command */
#define MR_CMD_SET_RX_CALLBACK     0x2000                      /**< Set rx callback command */
#define MR_CMD_SET_TX_CALLBACK     0x3000                      /**< Set tx callback command */
#define MR_CMD_ATTACH              0x4000                      /**< Attach command */
#define MR_CMD_TRANSFER            0x5000                      /**< Transfer command */
#define MR_CMD_REBOOT              0x6000                      /**< Reboot command */
#define MR_CMD_STOP                0x7000                      /**< Stop command */
#define _MR_CMD_MASK               0xf000                      /**< Mask for getting command */

/* mr-library basic data_p flag definitions */
typedef signed char mr_int8_t;                                 /**< Type for 8bit integer */
typedef signed short mr_int16_t;                               /**< Type for 16bit integer */
typedef signed int mr_int32_t;                                 /**< Type for 32bit integer */
typedef unsigned char mr_uint8_t;                              /**< Type for 8bit unsigned integer */
typedef unsigned short mr_uint16_t;                            /**< Type for 16bit unsigned integer */
typedef unsigned int mr_uint32_t;                              /**< Type for 32bit unsigned integer */
typedef float mr_float_t;                                      /**< Type for single-precision floating point */
typedef double mr_double_t;                                    /**< Type for double-precision floating point */

typedef signed long long mr_int64_t;                           /**< Type for 64bit integer */
typedef unsigned long long mr_uint64_t;                        /**< Type for 64bit unsigned integer */

typedef unsigned int mr_size_t;                                /**< Type for size number */
typedef signed int mr_base_t;                                  /**< Type for Nbit CPU related date */
typedef unsigned int mr_ubase_t;                               /**< Type for Nbit unsigned CPU related data */

typedef mr_base_t mr_err_t;                                    /**< Type for error number */
typedef mr_base_t mr_off_t;                                    /**< Type for offset */

typedef enum mr_bool
{
	MR_FALSE = 0,                                              /**< Boolean fails */
	MR_TRUE = 1                                                /**< Boolean true */
} mr_bool_t;                                                   /**< Type for boolean */

typedef enum mr_level
{
	MR_LOW = 0,                                                /**< Level low */
	MR_HIGH = 1                                                /**< Level high */
} mr_level_t;                                                  /**< Type for level */

typedef enum mr_state
{
	MR_DISABLE = 0,                                            /**< State disable */
	MR_ENABLE = 1                                              /**< State enable */
} mr_state_t;                                                  /**< Type for state */

typedef enum mr_lock
{
	MR_UNLOCK = 0,                                             /**< Unlock*/
	MR_LOCK = 1                                                /**< Lock */
} mr_lock_t;                                                   /**< Type for lock */

/**
 *  Double-list
 */
struct mr_list
{
	struct mr_list *next;                                      /**< Point to next node */
	struct mr_list *prev;                                      /**< Point to prev node */
};
typedef struct mr_list *mr_list_t;                             /**< Type for list */

/**
 *  Avl-tree
 */
struct mr_avl
{
	mr_int8_t height;                                          /**< Balance factor */
	mr_uint32_t value;                                         /**< Key-value */

	struct mr_avl *left_child;                                 /**< Point to left-child node */
	struct mr_avl *right_child;                                /**< Point to right-child node */
};
typedef struct mr_avl *mr_avl_t;                               /**< Type for avl-tree */

/**
 *  Fifo
 */
enum mr_fifo_state
{
	MR_FIFO_EMPTY,                                       	   /**< Empty fifo state */
	MR_FIFO_FULL,                                        	   /**< Full fifo state */
	MR_FIFO_HALF_FULL,                                   	   /**< Half-full fifo state */
};

struct mr_fifo
{
	mr_uint8_t *buffer;                                        /**< Buffer pool */

	mr_uint16_t read_mirror: 1;                                /**< Read mirror flag */
	mr_uint16_t read_index: 15;                                /**< Read index */
	mr_uint16_t write_mirror: 1;                               /**< Write mirror flag */
	mr_uint16_t write_index: 15;                               /**< Write index */

	mr_uint16_t size;                                          /**< Buffer pool size */
};
typedef struct mr_fifo *mr_fifo_t;                 			   /**< Type for fifo */

/**
 *  Container
 */
enum mr_container_type
{
	MR_CONTAINER_TYPE_MISC,                                    /**< Miscellaneous container */
	MR_CONTAINER_TYPE_DEVICE,                                  /**< Device container */
	MR_CONTAINER_TYPE_EVENT,                                   /**< Event container */
	_MR_CONTAINER_TYPE_MASK,                                   /**< Mask for getting container type */
};

struct mr_container
{
	enum mr_container_type type;                               /**< Container flag */
	struct mr_list list;                                       /**< Container list */
};
typedef struct mr_container *mr_container_t;                   /**< Type for container */

/**
 *  Object
 */
#define MR_OBJECT_TYPE_NULL         0x00
#define MR_OBJECT_TYPE_REGISTER     0x10

struct mr_object
{
	char name[MR_NAME_MAX + 1];                                /**< Object name */
	mr_uint8_t flag;                                           /**< Object flag */
	struct mr_list list;                                       /**< Object list */
};
typedef struct mr_object *mr_object_t;                         /**< Type for object */

/**
 *  Mutex
 */
struct mr_mutex
{
	mr_object_t owner;                                         /**< Mutex owns the object */
	mr_lock_t lock;                                            /**< Mutex lock state */
};
typedef struct mr_mutex *mr_mutex_t;                           /**< Type for mutex */

/**
 *  Device
 */
enum mr_device_type
{
	MR_DEVICE_TYPE_NULL,                                       /**< Null-type device */
	MR_DEVICE_TYPE_PIN,                                        /**< GPIO device */
	MR_DEVICE_TYPE_SPI_BUS,                                    /**< SPI-BUS device */
	MR_DEVICE_TYPE_SPI,                                        /**< SPI device */
	MR_DEVICE_TYPE_I2C_BUS,                                    /**< I2C-BUS device */
	MR_DEVICE_TYPE_I2C,                                        /**< I2C device */
	MR_DEVICE_TYPE_SERIAL,                                     /**< UART device */
	MR_DEVICE_TYPE_ADC,                                        /**< ADC device */
	MR_DEVICE_TYPE_DAC,                                        /**< DAC device */
	MR_DEVICE_TYPE_PWM,                                        /**< PWM device */
	MR_DEVICE_TYPE_TIMER,                                      /**< TIMER device */
	MR_DEVICE_TYPE_FLASH,                                      /**< FLASH device */
	MR_DEVICE_TYPE_SDRAM,                                      /**< SDRAM device */
	/* ... */
};

typedef struct mr_device *mr_device_t;                         /**< Type for device */
struct mr_device_ops
{
	mr_err_t (*open)(mr_device_t device);
	mr_err_t (*close)(mr_device_t device);
	mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
	mr_size_t (*read)(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
	mr_size_t (*write)(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
};

struct mr_device
{
	struct mr_object object;                                   /**< Device object */

	enum mr_device_type type;                                  /**< Device type */
	mr_uint16_t support_flag;                                  /**< Open mode supported by the device */
	mr_uint16_t open_flag;                                     /**< Opening mode of the device */
	mr_uint8_t ref_count;                                      /**< Number of device references */
	void *data;                                                /**< Device data */

	mr_err_t (*rx_callback)(mr_device_t device, void *args);   /**< Device receives the callback function */
	mr_err_t (*tx_callback)(mr_device_t device, void *args);   /**< Device sends the callback function */

	const struct mr_device_ops *ops;                           /**< Operations of the device */
};

/**
 *  Event
 */
enum mr_event_manager_type
{
	MR_EVENT_MANAGER_TYPE_GENERAL,                             /**< General event-manager */
	MR_EVENT_MANAGER_TYPE_FSM,                                 /**< Finite state machine(FSM) event-manager */
};

struct mr_event_manager
{
	struct mr_object object;                                   /**< Event-manager object */

	enum mr_event_manager_type type;                           /**< Event-manager type */
	struct mr_fifo queue;                                /**< Event queue */
	mr_avl_t avl;                                              /**< Event-manager list */
};
typedef struct mr_event_manager *mr_event_manager_t;           /**< Type for event-manager */

typedef struct mr_event *mr_event_t;                           /**< Type for event */
struct mr_event
{
	struct mr_avl avl;                                         /**< Avl-tree and event value */

	mr_err_t (*callback)(mr_event_manager_t manager,           /**< Event occurrence callback function */
						 void *args);
	void *args;                                                /**< Callback function argument */
};

/**
 *  Finite state machine(FSM)
 */
typedef struct mr_fsm_manager *mr_fsm_manager_t;               /**< Type for finite state machine(FSM) */
struct mr_fsm_manager
{
	struct mr_event_manager manager;                           /**< Fsm event-manager */
	mr_uint8_t pool[4];                                        /**< Event buffer pool */

	mr_uint32_t state;                                         /**< Fsm now state */
};

#endif