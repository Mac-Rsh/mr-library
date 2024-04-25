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

#if defined(__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)                   @x
#define MR_USED                         __root
#define MR_WEAK                         __weak
#define MR_INLINE                       static inline
#else
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#endif /* __ARMCC_VERSION */

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
    uint32_t in_mirror: 1;                                  /**< Input mirror */
    uint32_t out_mirror: 1;                                 /**< Output mirror */
    uint32_t dynamic: 1;                                    /**< Dynamic mode */
    uint32_t _reserved: 29;                                 /**< Reserved */
    uint8_t *buf;                                           /**< Data buffer */
    uint32_t size;                                          /**< Buffer size */
};

/** @} */

/**
 * @addtogroup Device
 * @{
 */

#define MR_FLAG_RDONLY                  (0x01 << 24)        /**< Read only flag */
#define MR_FLAG_WRONLY                  (0x02 << 24)        /**< Write only flag */
#define MR_FLAG_RDWR                    (0x03 << 24)        /**< Read/write flag */
#define MR_FLAG_RDONLY_ASYNC            (0x04 << 24)        /**< Async read only flag */
#define MR_FLAG_WRONLY_ASYNC            (0x08 << 24)        /**< Async write only flag */
#define MR_FLAG_RDWR_ASYNC              (0x0c << 24)        /**< Async read/write flag */

#define MR_CTRL_SET(_cmd)               (_cmd)              /**< Set operation */
#define MR_CTRL_GET(_cmd)               (-(_cmd))           /**< Get operation */
#define MR_CTRL_NEW(_cmd)               MR_CTRL_SET(_cmd)   /**< New operation */
#define MR_CTRL_DEL(_cmd)               MR_CTRL_GET(_cmd)   /**< Delete operation */
#define MR_CTRL_CLR(_cmd)               (_cmd)              /**< Clear operation */

#define MR_CMD_POS                      (0x01 << 24)        /**< Position command */
#define MR_CMD_EVENT                    (0x02 << 24)        /**< Event command */
#define MR_CMD_CONFIG                   (0x03 << 24)        /**< Configuration command */

#define MR_EVENT_RD_COMPLETE            (0x01 << 24)        /**< Read complete event */
#define MR_EVENT_WR_COMPLETE            (0x02 << 24)        /**< Write complete event */

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
    MR_DEVICE_TYPE_FULL_DUPLEX = 0x80000000,                /**< Full duplex device */
};

struct mr_device;

/**
 * @brief Device operations structure.
 */
struct mr_device_ops
{
    int (*open)(struct mr_device *device);
    int (*close)(struct mr_device *device);
    ssize_t (*read)(struct mr_device *device, int pos, void *buf, size_t count);
    ssize_t (*write)(struct mr_device *device, int pos, const void *buf,
                     size_t count);
    ssize_t (*read_async)(struct mr_device *device, int pos, void *buf,
                          size_t count);
    ssize_t (*write_async)(struct mr_device *device, int pos, const void *buf,
                           size_t count);
    int (*ioctl)(struct mr_device *device, int pos, int cmd, void *args);
    int (*isr)(struct mr_device *device, uint32_t event, void *args);

    int (*attach)(struct mr_device *device, struct mr_device *source);
    int (*detach)(struct mr_device *device, struct mr_device *source);
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
    struct mr_list list;                                    /**< Same level device list */
    struct mr_list clist;                                   /**< Child device list */
    void *parent;                                           /**< Parent device */

    uint32_t type: 31;                                      /**< Type */
    uint32_t full_duplex: 1;                                /**< Full duplex */
    uint32_t flags;                                         /**< Flags */
    size_t ref_count;                                       /**< Reference count */
    volatile uint32_t lock;                                 /**< Operation lock */
    const struct mr_device_ops *ops;                        /**< Operations */
    const void *driver;                                     /**< Driver */
    struct mr_list event_list;                              /**< Event list */
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
 * @brief Device event structure.
 */
struct mr_device_event
{
    uint32_t event;                                         /**< Event */
    void (*callback)(int descriptor, uint32_t event,
                     void *args);                           /**< Callback function */
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
