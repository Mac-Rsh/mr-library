/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-18    MacRsh       First version
 * @date 2024-03-24    MacRsh       Updated v1.0.0
 */

#ifndef _MR_DEF_H_
#define _MR_DEF_H_

#include "../mr-library/include/mr_config.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Version
 * @{
 */

#define MR_VERSION_MAJOR                (1)                 /**< Major version (X.y.z) */
#define MR_VERSION_MINOR                (0)                 /**< Minor version (x.Y.z) */
#define MR_VERSION_PATCH                (0)                 /**< Patch version (x.y.Z) */
/** Version (X.Y.Z) */
#define MR_VERSION                                                             \
    (MR_VERSION_MAJOR << 16 | MR_VERSION_MINOR << 8 | MR_VERSION_PATCH)

/** @} */

/**
 * @addtogroup Compiler
 * @{
 */

#if defined(__GNUC__) || defined(__clang__)
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       __inline
#elif defined(__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)                   @x
#define MR_USED                         __root
#define MR_WEAK                         __weak
#define MR_INLINE                       static inline
#else
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#endif /* __GNUC__ */

/** @} */

/**
 * @addtogroup Auto-Init
 * @{
 */

/** Auto-initialization functions */
typedef void (*mr_init_fn_t)(void);
/** Exports an initialization function by level */
#define MR_INIT_EXPORT(_fn, _level)                                            \
    MR_USED MR_SECTION("mr_auto_init." _level)                                 \
        const mr_init_fn_t _mr_auto_init_##_fn = _fn
/** Exports a Board initialization function */
#define MR_INIT_BOARD_EXPORT(_fn)       MR_INIT_EXPORT(_fn, "1")
/** Exports a Driver initialization function */
#define MR_INIT_DRIVER_EXPORT(_fn)      MR_INIT_EXPORT(_fn, "2")
/** Exports a Device initialization function */
#define MR_INIT_DEVICE_EXPORT(_fn)      MR_INIT_EXPORT(_fn, "3")
/** Exports an App initialization function */
#define MR_INIT_APP_EXPORT(_fn)         MR_INIT_EXPORT(_fn, "4")

/** @} */

/**
 * @addtogroup Error
 * @{
 */

#define MR_EOK                          (0)                 /** No error */
#define MR_EPERM                        (-1)                /** Operation not permitted */
#define MR_ENOENT                       (-2)                /** No such file or directory */
#define MR_EIO                          (-3)                /** I/O error */
#define MR_ENOMEM                       (-4)                /**< Out of memory */
#define MR_EACCES                       (-5)                /**< Permission denied */
#define MR_EBUSY                        (-6)                /**< Resource busy */
#define MR_EEXIST                       (-7)                /**< Resource exists */
#define MR_EINVAL                       (-8)                /**< Invalid argument */
#define MR_ETIMEOUT                     (-9)                /**< Operation timed */

/** @} */

/**
 * @addtogroup Memory
 * @{
 */

/**
 * @brief Heap block structure.
 */
struct mr_heap_block
{
    struct mr_heap_block *next;                             /**< Point to next block */
    uint32_t size: 31;                                      /**< Size of this block */
    uint32_t allocated: 1;                                  /**< Allocated flag */
};

/** @} */

/**
 * @addtogroup List
 * @{
 */

/**
 * @brief List structure.
 */
struct mr_list
{
    struct mr_list *next;                                   /**< Point to next node */
    struct mr_list *prev;                                   /**< Point to prev node */
};

/** @} */

/**
 * @addtogroup FIFO
 * @{
 */

/**
 * @brief FIFO structure.
 */
struct mr_fifo
{
    uint32_t in;                                            /**< Input index */
    uint32_t out;                                           /**< Output index */
    uint8_t *buf;                                           /**< Data buffer */
    uint32_t size: 31;                                      /**< Buffer size */
    uint32_t dynamic: 1;                                    /**< Dynamic mode */
};

/** @} */

/**
 * @addtogroup Device
 * @{
 */

#define MR_FLAG_RDONLY                  (0x01 << 24)        /**< Read only flag */
#define MR_FLAG_WRONLY                  (0x02 << 24)        /**< Write only flag */
#define MR_FLAG_RDWR                    (0x03 << 24)        /**< Read/write flag */

#define MR_CMD_POS                      (0x01 << 24)        /**< Position command */
#define MR_CMD_EVENT                    (0x02 << 24)        /**< Event command */
#define MR_CMD_CONFIG                   (0x03 << 24)        /**< Configuration command */

#define MR_EVENT_DATA                   (0x01 << 24)        /**< Data event */
#define MR_EVENT_RD                     (0x10 << 24)        /**< Read event */
#define MR_EVENT_WR                     (0x20 << 24)        /**< Write event */
#define _MR_EVENT_MASK                  (0xff << 24)        /**< Event mask */

#define _MR_OPERATE_MASK_RD             (0xffff0000)        /**< Read lock mask */
#define _MR_OPERATE_MASK_WR             (0x0000ffff)        /**< Write lock mask */
#define _MR_OPERATE_MASK_ALL            (0xffffffff)        /**< Read/write lock mask */

/**
 * @brief Device types.
 */
enum mr_device_type
{
    MR_DEVICE_TYPE_ADC,                                     /**< ADC device */
    MR_DEVICE_TYPE_CAN,                                     /**< CAN device */
    MR_DEVICE_TYPE_DAC,                                     /**< DAC device */
    MR_DEVICE_TYPE_I2C,                                     /**< I2C device */
    MR_DEVICE_TYPE_PIN,                                     /**< PIN device */
    MR_DEVICE_TYPE_SERIAL,                                  /**< Serial device */
    MR_DEVICE_TYPE_SPI,                                     /**< SPI device */
    MR_DEVICE_TYPE_TIMER,                                   /**< Timer device */
    MR_DEVICE_TYPE_PWM,                                     /**< PWM device */
    MR_DEVICE_TYPE_FDX = 0x80000000,                        /**< Full duplex device */
};

struct mr_device;

/**
 * @brief Device operations structure.
 */
struct mr_device_ops
{
    int (*attach)(struct mr_device *device, struct mr_device *source);
    int (*detach)(struct mr_device *device, struct mr_device *source);
    int (*open)(struct mr_device *device);
    int (*close)(struct mr_device *device);
    ssize_t (*read)(struct mr_device *device, int pos, void *buf, size_t count);
    ssize_t (*write)(struct mr_device *device, int pos, const void *buf,
                     size_t count);
    int (*ioctl)(struct mr_device *device, int pos, int cmd, void *args);
    int (*isr)(struct mr_device *device, uint32_t event, void *args);
#ifdef MR_USE_AIO_EXT
    ssize_t (*aread)(struct mr_device *device, int pos, void *buf,
                     size_t count);
    ssize_t (*awrite)(struct mr_device *device, int pos, const void *buf,
                      size_t count);
#endif /* MR_USE_AIO_EXT */
};

/**
 * @brief Device structure.
 */
struct mr_device
{
    uint32_t magic;                                         /**< Magic number */
#ifndef MR_CFG_DEVICE_NAME_MAX
#define MR_CFG_DEVICE_NAME_MAX          (12)
#endif /* MR_CFG_DEVICE_NAME_MAX */
    char name[MR_CFG_DEVICE_NAME_MAX];                      /**< Name */
    struct mr_list list;                                    /**< Same level list */
    struct mr_list clist;                                   /**< Children list */
    void *parent;                                           /**< Parent device */

    uint32_t type: 31;                                      /**< Type */
    uint32_t fdx: 1;                                        /**< Full duplex */
    uint32_t flags;                                         /**< Flags */
    size_t ref_count;                                       /**< Reference count */
    volatile uint32_t lock;                                 /**< Operation lock */
    const struct mr_device_ops *ops;                        /**< Operations */
    struct mr_list elist;                                   /**< Event list */

    const void *driver;                                     /**< Driver */
};

/**
 * @brief Device descriptor structure.
 */
struct mr_descriptor
{
    struct mr_device *device;                               /**< Device */
    uint32_t flags;                                         /**< Open flags */
    int pos;                                                /**< Current position */
};

/**
 * @brief Event structure.
 */
struct mr_event
{
    uint32_t event: 31;                                     /**< Event */
    uint32_t self: 1;                                       /**< Self-defined event */
    void (*callback)(int descriptor, uint32_t event,
                     void *args, void *op_data);            /**< Callback function */
    void *op_data;                                          /**< Operator data */
};

/** @} */

/**
 * @addtogroup Driver
 * @{
 */

/**
 * @brief Driver structure.
 */
struct mr_driver
{
    const void *ops;                                        /**< Operations */
    void *data;                                             /**< Data */
};

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DEF_H_ */
