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
#define mr_printf                       printf

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
#define MR_ERR_OK                       0                           /**< There is no error */
#define MR_ERR_GENERIC                  1                           /**< A generic error happens */
#define MR_ERR_NO_MEMORY                2                           /**< No memory */
#define MR_ERR_IO                       3                           /**< IO error */
#define MR_ERR_TIMEOUT                  4                           /**< Timed out */
#define MR_ERR_BUSY                     5                           /**< Busy */
#define MR_ERR_NOT_FOUND                6                           /**< Not found */
#define MR_ERR_UNSUPPORTED              7                           /**< Unsupported feature */
#define MR_ERR_INVALID                  8                           /**< Invalid parameter */

/* mr-library basic open flag definitions */
#define MR_OPEN_CLOSED                  0x0000                      /**< Closed */
#define MR_OPEN_RDONLY                  0x1000                      /**< Read only */
#define MR_OPEN_WRONLY                  0x2000                      /**< Write only */
#define MR_OPEN_RDWR                    0x3000                      /**< Read and write */
#define MR_OPEN_NONBLOCKING             0x4000                      /**< Non-blocking */
#define MR_OPEN_ACTIVE                  0x8000                      /**< Active */
#define _MR_OPEN_FLAG_MASK              0xf000                      /**< Mask for getting open flag */

/* mr-library basic control flag definitions */
#define MR_CTRL_NONE                    0x0000                      /**< No control */
#define MR_CTRL_CONFIG                  0x1000                      /**< Configure */
#define MR_CTRL_SET_RX_CB               0x2000                      /**< Set receive callback */
#define MR_CTRL_SET_TX_CB               0x3000                      /**< Set transmit callback */
#define MR_CTRL_ATTACH                  0x4000                      /**< Attach the bus */
#define MR_CTRL_REBOOT                  0x5000                      /**< Reboot */
#define _MR_CTRL_FLAG_MASK              0xf000                      /**< Mask for getting control flag */

/* mr-library basic data type definitions */
#define MR_NULL                         0                           /**< Null pointer */
#define MR_FALSE                        0                           /**< False */
#define MR_TRUE                         1                           /**< True */
#define MR_LOW                          0                           /**< Low level */
#define MR_HIGH                         1                           /**< High level */
#define MR_DISABLE                      0                           /**< Disable */
#define MR_ENABLE                       1                           /**< Enable */
#define MR_UNLOCK                       0                           /**< Unlock */
#define MR_LOCK                         1                           /**< Lock */

typedef signed char mr_int8_t;                                      /**< Type for 8bit integer */
typedef signed short mr_int16_t;                                    /**< Type for 16bit integer */
typedef signed int mr_int32_t;                                      /**< Type for 32bit integer */
typedef signed long long mr_int64_t;                                /**< Type for 64bit integer */
typedef unsigned char mr_uint8_t;                                   /**< Type for 8bit unsigned integer */
typedef unsigned short mr_uint16_t;                                 /**< Type for 16bit unsigned integer */
typedef unsigned int mr_uint32_t;                                   /**< Type for 32bit unsigned integer */
typedef unsigned long long mr_uint64_t;                             /**< Type for 64bit unsigned integer */
typedef float mr_fp32_t;                                            /**< Type for single-precision floating point */
typedef double mr_fp64_t;                                           /**< Type for double-precision floating point */

typedef signed int mr_base_t;                                       /**< Type for Nbit CPU related date */
typedef unsigned int mr_ubase_t;                                    /**< Type for Nbit unsigned CPU related data */
typedef mr_ubase_t mr_size_t;                                       /**< Type for size number */
typedef mr_base_t mr_ssize_t;                                       /**< Type for signed size number */
typedef mr_base_t mr_err_t;                                         /**< Type for error number */
typedef mr_base_t mr_off_t;                                         /**< Type for offset */
typedef mr_int8_t mr_bool_t;                                        /**< Type for boolean */
typedef mr_int8_t mr_level_t;                                       /**< Type for level */
typedef mr_int8_t mr_state_t;                                       /**< Type for state */
typedef mr_int8_t mr_lock_t;                                        /**< Type for lock */

/**
 *  Auto-Init
 */
typedef int (*init_fn_t)(void);
#define INIT_EXPORT(fn,level) \
    mr_used const init_fn_t _mr_init_##fn mr_section(".mri_fn."level) = fn

#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")
#define INIT_DEV_EXPORT(fn)          	INIT_EXPORT(fn, "2")
#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "3")
#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "4")

/**
 *  Double-list
 */
struct mr_list
{
	struct mr_list *next;                                           /**< Point to next node */
	struct mr_list *prev;                                           /**< Point to prev node */
};
typedef struct mr_list *mr_list_t;                                  /**< Type for list */

/**
 *  Avl-tree
 */
struct mr_avl
{
	mr_int8_t height;                                               /**< Balance factor */
	mr_uint32_t value;                                              /**< Key-value */

	struct mr_avl *left_child;                                      /**< Point to left-child node */
	struct mr_avl *right_child;                                     /**< Point to right-child node */
};
typedef struct mr_avl *mr_avl_t;                                    /**< Type for avl-tree */

/**
 *  Fifo
 */
struct mr_fifo
{
	mr_uint8_t *buffer;                                             /**< Buffer pool */

	mr_uint16_t read_mirror: 1;                                     /**< Read mirror flag */
	mr_uint16_t read_index: 15;                                     /**< Read index */
	mr_uint16_t write_mirror: 1;                                    /**< Write mirror flag */
	mr_uint16_t write_index: 15;                                    /**< Write index */

	mr_uint16_t size;                                               /**< Buffer pool size */
};
typedef struct mr_fifo *mr_fifo_t;                                  /**< Type for fifo */

/**
 *  Container
 */
enum mr_container_type
{
	MR_CONTAINER_TYPE_MISC,                                         /**< Miscellaneous container */
	MR_CONTAINER_TYPE_DEVICE,                                       /**< Device container */
	MR_CONTAINER_TYPE_MANAGER,                                      /**< Event container */
	_MR_CONTAINER_TYPE_MASK,                                        /**< Mask for getting container type */
};

struct mr_container
{
	struct mr_list list;                                            /**< Container list */

	enum mr_container_type type;                                    /**< Container flag */
};
typedef struct mr_container *mr_container_t;                        /**< Type for container */

/**
 *  Object
 */
#define _MR_OBJECT_TYPE_NONE           0x00
#define _MR_OBJECT_TYPE_REGISTER       0x10

struct mr_object
{
	struct mr_list list;                                            /**< Object list */

	char name[MR_CONF_NAME_MAX + 1];                                /**< Object name */
	mr_uint8_t flag;                                                /**< Object flag */
};
typedef struct mr_object *mr_object_t;                              /**< Type for object */

/**
 *  Mutex
 */
struct mr_mutex
{
	mr_object_t owner;                                              /**< Mutex owns the object */
	mr_lock_t lock;                                                 /**< Mutex lock state */
};
typedef struct mr_mutex *mr_mutex_t;                                /**< Type for mutex */

/**
 *  Device
 */
enum mr_device_type
{
	MR_DEVICE_TYPE_NONE,                                            /**< No device */
	MR_DEVICE_TYPE_PIN,                                             /**< GPIO device */
	MR_DEVICE_TYPE_SPI_BUS,                                         /**< SPI-BUS device */
	MR_DEVICE_TYPE_SPI,                                             /**< SPI device */
	MR_DEVICE_TYPE_I2C_BUS,                                         /**< I2C-BUS device */
	MR_DEVICE_TYPE_I2C,                                             /**< I2C device */
	MR_DEVICE_TYPE_SERIAL,                                          /**< UART device */
	MR_DEVICE_TYPE_ADC,                                             /**< ADC device */
	MR_DEVICE_TYPE_DAC,                                             /**< DAC device */
	MR_DEVICE_TYPE_PWM,                                             /**< PWM device */
	MR_DEVICE_TYPE_TIMER,                                           /**< TIMER device */
	MR_DEVICE_TYPE_FLASH,                                           /**< FLASH device */
	/* ... */
};

typedef struct mr_device *mr_device_t;                              /**< Type for device */
struct mr_device_ops
{
	mr_err_t (*open)(mr_device_t device);
	mr_err_t (*close)(mr_device_t device);
	mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
	mr_ssize_t (*read)(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
	mr_ssize_t (*write)(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
};

struct mr_device
{
	struct mr_object object;                                        /**< Device object */

	enum mr_device_type type;                                       /**< Device type */
	mr_uint16_t support_flag;                                       /**< Open mode supported by the device */
	mr_uint16_t open_flag;                                          /**< Opening mode of the device */
	mr_size_t ref_count;                                            /**< Number of device references */
	void *data;                                                     /**< Device data */

	mr_err_t (*rx_cb)(mr_device_t device, void *args);              /**< Device receives the baud function */
	mr_err_t (*tx_cb)(mr_device_t device, void *args);              /**< Device sends the baud function */

	const struct mr_device_ops *ops;                                /**< Operations of the device */
};

/**
 *  Manager
 */
enum mr_manager_type
{
	MR_MANAGER_TYPE_EVENT,                                          /**< Event manager */
	MR_MANAGER_TYPE_FSM,                                            /**< Finite state machine(FSM) manager */
	MR_MANAGER_TYPE_AT_PARSER,                                      /**< Attention(AT) Parser manager */
	MR_MANAGER_TYPE_CMD_PARSER,                                     /**< Command(CMD) Parser manager */
	/* ... */
};

enum mr_manager_at_parser_state
{
	MR_MANAGER_AT_STATE_NONE,                                       /**< No state */
	MR_MANAGER_AT_STATE_START,                                      /**< Start state */
	MR_MANAGER_AT_STATE_FLAG,                                       /**< Flag state */
	MR_MANAGER_AT_STATE_ID,                                        /**< Id state */
	MR_MANAGER_AT_STATE_CHECK,                                      /**< Check state */
	MR_MANAGER_AT_STATE_ARGS,                                       /**< Args state */
	MR_MANAGER_AT_STATE_STOP,                                       /**< Stop state */
	MR_MANAGER_AT_STATE_HANDLE,                                     /**< Handle state */
};

typedef struct mr_manager *mr_manager_t;                            /**< Type for manager */
struct mr_manager_ops
{
	mr_err_t (*add)(mr_manager_t manager);
	mr_err_t (*remove)(mr_manager_t manager);
	void (*handler)(mr_manager_t manage);
};

struct mr_manager
{
	struct mr_object object;                                        /**< Manager object */

	enum mr_manager_type type;                                      /**< Manager type */
	void *data;                                                     /**< Manager data */
	struct mr_fifo queue;                                           /**< Agent queue */

	mr_avl_t avl;                                                   /**< Manager list */

	const struct mr_manager_ops *ops;                               /**< Operations of the manager */
};

/**
 * 	Agent
 */
struct mr_agent
{
	struct mr_avl avl;                                              /**< Avl-tree and agent id */

	mr_err_t (*cb)(mr_manager_t manager, void *args);               /**< Agent occurrence callback function */
	void *args;                                                     /**< Callback function argument */
};
typedef struct mr_agent *mr_agent_t;                                /**< Type for agent */

#endif