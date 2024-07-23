/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-18    MacRsh       First version
 * @date 2024-03-24    MacRsh       Updated v1.0.0
 */

#ifndef __MR_DEF_H__
#define __MR_DEF_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <include/mr_config.h>
#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Version
 * @{
 */

#define MR_VERSION                          ("1.0.0")                                                   /** Version (X.Y.Z) */

/** @} */

/**
 * @addtogroup Compiler
 * @{
 */

#if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#define MR_SECTION(x)                       __attribute__((section(x)))
#define MR_UNUSED                           __attribute__((unused))
#define MR_USED                             __attribute__((used))
#define MR_WEAK                             __attribute__((weak))
#define MR_INLINE                           static __inline
#elif defined(__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)                       @x
#define MR_UNUSED
#define MR_USED                             __root
#define MR_WEAK                             __weak
#define MR_INLINE                           static inline
#else
#define MR_SECTION(x)                       __attribute__((section(x)))
#define MR_UNUSED                           __attribute__((unused))
#define MR_USED                             __attribute__((used))
#define MR_WEAK                             __attribute__((weak))
#define MR_INLINE                           static __inline
#endif /* __GNUC__ */

/** @} */

/**
 * @addtogroup Error
 * @{
 */

#define MR_EOK                              (0)                                                         /**< No error */
#define MR_EPERM                            (-1)                                                        /**< Operation not permitted */
#define MR_ENOENT                           (-2)                                                        /**< No such file or directory */
#define MR_EIO                              (-5)                                                        /**< I/O error */
#define MR_EAGAIN                           (-11)                                                       /**< Resource temporarily unavailable */
#define MR_ENOMEM                           (-12)                                                       /**< Out of memory */
#define MR_EACCES                           (-13)                                                       /**< Permission denied */
#define MR_EBUSY                            (-16)                                                       /**< Resource busy */
#define MR_EEXIST                           (-17)                                                       /**< Resource exists */
#define MR_EINVAL                           (-22)                                                       /**< Invalid argument */
#define MR_ENOSYS                           (-38)                                                       /**< Not supported */
#define MR_ETIMEOUT                         (-110)                                                      /**< Operation timed */

/** @} */

/**
 * @addtogroup Initialization
 * @{
 */

/**
 * @brief Initialization item structure.
 * 
 */
typedef struct mr_init_item
{
    const char *name;                                                                                   /**< Name */
    int (*fn)(void);                                                                                    /**< Function */
} mr_init_item_t;

#define __MR_INIT_EXPORT(_name, _fn, _level)                                                       \
    MR_USED MR_SECTION("mr_init." _level) const mr_init_item_t _mr_init_item_##_fn                 \
        = {(_name), (_fn)}                                                                              /**< Initialization item export */
#define MR_INIT_BOARD_EXPORT(_name, _fn)    __MR_INIT_EXPORT(_name, _fn, "1")                           /**< Board initialization */
#define MR_INIT_SYSTEM_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "2")                           /**< System initialization */
#define MR_INIT_METADATA_EXPORT(_name, _fn) __MR_INIT_EXPORT(_name, _fn, "3")                           /**< Metadata initialization */
#define MR_INIT_DRIVER_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "4")                           /**< Driver initialization */
#define MR_INIT_DEVICE_EXPORT(_name, _fn)   __MR_INIT_EXPORT(_name, _fn, "5")                           /**< Device initialization */
#define MR_INIT_APP_EXPORT(_name, _fn)      __MR_INIT_EXPORT(_name, _fn, "6")                           /**< Application initialization */

/** @} */

/**
 * @addtogroup Atomic
 * @{
 */

typedef int                                 mr_atomic_t;                                                /**< Atomic type */

/** @} */

/**
 * @addtogroup Spinlock
 * @{
 */

/**
 * @brief Spinlock structure.
 */
typedef struct mr_spinlock
{
    mr_atomic_t lock;                                                                                   /**< Lock */
} mr_spinlock_t;

/** @} */

/**
 * @addtogroup Memory
 * @{
 */

/**
 * @brief Heap block structure.
 */
typedef struct mr_heap_block
{
    struct mr_heap_block *next;                                                                         /**< Point to next block */
    uint32_t size: 31;                                                                                  /**< Size of this block */
    uint32_t allocated: 1;                                                                              /**< Allocated flag */
} mr_heap_block_t;

/** @} */

/**
 * @addtogroup List
 * @{
 */

/**
 * @brief Double list structure.
 */
typedef struct mr_list
{
    struct mr_list *next;                                                                               /**< Point to next nod */
    struct mr_list *prev;                                                                               /**< Point to prev nod */
} mr_list_t;

/** @} */

/**
 * @addtogroup FIFO
 * @{
 */

/**
 * @brief FIFO structure.
 */
typedef struct mr_fifo
{
    uint8_t *buf;                                                                                       /**< Data buffer */
    uint32_t in;                                                                                        /**< Input index */
    uint32_t out;                                                                                       /**< Output index */
    uint32_t size: 31;                                                                                  /**< Buffer size */
    uint32_t dynamic: 1;                                                                                /**< Dynamic mode */
} mr_fifo_t;

/** @} */

/**
 * @addtogroup Reference
 * @{
 */

/**
 * @brief Reference structure.
 */
typedef struct mr_ref
{
    mr_atomic_t count;                                                                                  /**< Reference count */
} mr_ref_t;

/** @} */

/**
 * @addtogroup Class
 * @{
 */

/**
 * @brief Class structure.
 */
typedef struct mr_class
{
    const char *name;                                                                                   /**< Name */
    struct mr_class *parent;                                                                            /**< Parent */
    mr_list_t list, clist;                                                                              /**< List */
    mr_spinlock_t lock;                                                                                 /**< List lock */
    void *privdata;                                                                                     /**< Private data */
    size_t privsize;                                                                                    /**< Private data size */
    mr_ref_t refcount;                                                                                  /**< Reference count */
    void (*release)(struct mr_class *);                                                                 /**< Release function */
    void *methods;                                                                                      /**< Methods */
} mr_class_t;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_DEF_H__ */
