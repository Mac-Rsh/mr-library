/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-18    MacRsh       First version
 */

#ifndef _MR_DEF_H_
#define _MR_DEF_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mr_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief MR version.
 */
#define MR_VERSION                      "0.0.6"

/**
 * @brief Compiler related.
 */
#if defined(__ARMCC_VERSION)
#define MR_SECTION(x)              	    __attribute__((section(x)))
#define MR_USED                    	    __attribute__((used))
#define MR_WEAK                    	    __attribute__((weak))
#define MR_INLINE                  	    static __inline
#elif defined (__IAR_SYSTEMS_ICC__)
#define MR_SECTION(x)               	@ x
#define MR_USED                     	__root
#define MR_WEAK                     	__weak
#define MR_INLINE                   	static inline
#elif defined (__GNUC__)
#define MR_SECTION(x)                   __attribute__((section(x)))
#define MR_USED                         __attribute__((used))
#define MR_WEAK                         __attribute__((weak))
#define MR_INLINE                       static __inline
#elif defined (__ADSPBLACKFIN__)
#define MR_SECTION(x)               	__attribute__((section(x)))
#define MR_USED                     	__attribute__((used))
#define MR_WEAK                     	__attribute__((weak))
#define MR_INLINE                   	static inline
#elif defined (_MSC_VER)
#define MR_SECTION(x)
#define MR_USED
#define MR_WEAK
#define MR_INLINE                   	static __inline
#elif defined (__TASKING__)
#define MR_SECTION(x)               	__attribute__((section(x)))
#define MR_USED                     	__attribute__((used, protect))
#define MR_WEAK                     	__attribute__((weak))
#define MR_INLINE                   	static inline
#endif /* __ARMCC_VERSION */

typedef int (*mr_init_fn_t)(void);

/**
 * @brief Exports an auto initialization function.
 */
#define MR_INIT_EXPORT(fn, level) \
    MR_USED const mr_init_fn_t _mr_auto_init_##fn MR_SECTION(".auto_init."level) = fn

/**
 * @brief Exports a board auto initialization function.
 */
#define MR_BOARD_EXPORT(fn)             MR_INIT_EXPORT(fn, "1")

/**
 * @brief Exports a driver auto initialization function.
 */
#define MR_DRV_EXPORT(fn)               MR_INIT_EXPORT(fn, "2")

/**
 * @brief Exports a device auto initialization function.
 */
#define MR_DEV_EXPORT(fn)               MR_INIT_EXPORT(fn, "3")

/**
 * @brief Exports a app auto initialization function.
 */
#define MR_APP_EXPORT(fn)               MR_INIT_EXPORT(fn, "4")

/**
 * @brief Error code.
 */
#define MR_EOK                          (0)                         /**< No error */
#define MR_ENOMEM                       (-1)                        /**< No enough memory */
#define MR_EIO                          (-2)                        /**< I/O error */
#define MR_ENOTFOUND                    (-3)                        /**< Not found */
#define MR_EBUSY                        (-4)                        /**< Resource busy */
#define MR_EEXIST                       (-5)                        /**< Exists */
#define MR_ENOTSUP                      (-6)                        /**< Operation not supported */
#define MR_EINVAL                       (-7)                        /**< Invalid argument */

/**
 * @brief Null pointer.
 */
#define MR_NULL                         ((void *)0)

/**
 * @brief Disable/enable.
 */
#define MR_DISABLE                      (0)                         /**< Disable */
#define MR_ENABLE                       (1)                         /**< Enable */

/**
 * @brief True/false.
 */
#define MR_FALSE                        (0)                         /**< False */
#define MR_TRUE                         (1)                         /**< True */

/**
 * @brief Double linked list structure.
 */
struct mr_list
{
    struct mr_list *next;                                           /**< Point to next node */
    struct mr_list *prev;                                           /**< Point to prev node */
};

/**
 * @brief Ring buffer structure.
 */
struct mr_ringbuf
{
    uint8_t *buffer;                                                /**< Buffer pool */
    uint16_t size;                                                  /**< Buffer pool size */
    uint16_t read_mirror: 1;                                        /**< Read mirror flag */
    uint16_t write_mirror: 1;                                       /**< Write mirror flag */
    uint16_t reserved: 14;                                          /**< Reserved */
    uint16_t read_index;                                            /**< Read index */
    uint16_t write_index;                                           /**< Write index */
};

/**
 * @brief AVL tree structure.
 */
struct mr_avl
{
    int32_t height;                                                 /**< Balance factor */
    uint32_t value;                                                 /**< Key-hold */
    struct mr_avl *left_child;                                      /**< Point to left-child node */
    struct mr_avl *right_child;                                     /**< Point to right-child node */
};

/**
 * @brief Driver types.
 */
enum mr_drv_type
{
    Mr_Drv_Type_ADC,                                                /**< ADC */
    Mr_Drv_Type_CAN,                                                /**< CAN */
    Mr_Drv_Type_DAC,                                                /**< DAC */
    Mr_Drv_Type_I2C,                                                /**< I2C */
    Mr_Drv_Type_Pin,                                                /**< PIN */
    Mr_Drv_Type_Serial,                                             /**< SERIAL */
    Mr_Drv_Type_SPI,                                                /**< SPI */
    Mr_Drv_Type_Timer,                                              /**< Timer */
};

/**
 * @brief Driver structure.
 */
struct mr_drv
{
    uint32_t type;                                                  /**< Driver type */
    void *ops;                                                      /**< Driver operations */
    void *data;                                                     /**< Driver data */
};

/**
 * @brief Device types.
 */
enum mr_dev_type
{
    Mr_Dev_Type_ADC = Mr_Drv_Type_ADC,                              /**< ADC */
    Mr_Dev_Type_CAN = Mr_Drv_Type_CAN,                              /**< CAN */
    Mr_Dev_Type_DAC = Mr_Drv_Type_DAC,                              /**< DAC */
    Mr_Dev_Type_I2C = Mr_Drv_Type_I2C,                              /**< I2C */
    Mr_Dev_Type_Pin = Mr_Drv_Type_Pin,                              /**< PIN */
    Mr_Dev_Type_Serial = Mr_Drv_Type_Serial,                        /**< SERIAL */
    Mr_Dev_Type_SPI = Mr_Drv_Type_SPI,                              /**< SPI */
    Mr_Dev_Type_Timer = Mr_Drv_Type_Timer,                          /**< Timer */
    Mr_Dev_Type_Sensor,                                             /**< Sensor */
};

struct mr_dev;

/**
 * @brief Synchronous/asynchronous operation flag.
 */
#define MR_SYNC                         (0)                         /**< Synchronous */
#define MR_ASYNC                        (1)                         /**< Asynchronous */

/**
 * @brief Descriptor control command.
 */
#define MR_CTL_SET_OFFSET               ((0x01|0x80) << 24)         /**< Set offset */
#define MR_CTL_SET_RD_CALL              ((0x02|0x80) << 24)         /**< Set read callback */
#define MR_CTL_SET_WR_CALL              ((0x03|0x80) << 24)         /**< Set write callback */
#define MR_CTL_SET_SLEEP                ((0x04|0x80) << 24)         /**< Set sleep */
#define MR_CTL_SET_WAKEUP               ((0x05|0x80) << 24)         /**< Set wakeup */
#define MR_CTL_SET_CONFIG               ((0x06|0x80) << 24)         /**< Set configuration */
#define MR_CTL_SET_RD_BUFSZ             ((0x07|0x80) << 24)         /**< Set read buffer size */
#define MR_CTL_SET_WR_BUFSZ             ((0x08|0x80) << 24)         /**< Set write buffer size */

#define MR_CTL_GET_OFFSET               ((0x01|0x00) << 24)         /**< Get offset */
#define MR_CTL_GET_RD_CALL              ((0x02|0x00) << 24)         /**< Get read callback */
#define MR_CTL_GET_WR_CALL              ((0x03|0x00) << 24)         /**< Get write callback */
#define MR_CTL_GET_SLEEP                ((0x04|0x00) << 24)         /**< Get sleep (reserved) */
#define MR_CTL_GET_WAKEUP               ((0x05|0x00) << 24)         /**< Get wakeup (reserved) */
#define MR_CTL_GET_CONFIG               ((0x06|0x00) << 24)         /**< Get configuration */
#define MR_CTL_GET_RD_BUFSZ             ((0x07|0x00) << 24)         /**< Get read buffer size */
#define MR_CTL_GET_WR_BUFSZ             ((0x08|0x00) << 24)         /**< Get write buffer size */

/**
 * @brief ISR event.
 */
#define MR_ISR_RD                       ((0x01) << 24)              /**< Read interrupt */
#define MR_ISR_WR                       ((0x02) << 24)              /**< Write interrupt */
#define MR_ISR_MASK                     ((0xff) << 24)              /**< Interrupt mask */

/**
 * @brief Device operations structure.
 */
struct mr_dev_ops
{
    int (*open)(struct mr_dev *dev);
    int (*close)(struct mr_dev *dev);
    ssize_t (*read)(struct mr_dev *dev, int off, void *buf, size_t size, int async);
    ssize_t (*write)(struct mr_dev *dev, int off, const void *buf, size_t size, int async);
    int (*ioctl)(struct mr_dev *dev, int off, int cmd, void *args);
    ssize_t (*isr)(struct mr_dev *dev, int event, void *args);
};

/**
 * @brief Magic number.
 */
#define MR_MAGIC_NUMBER                 (0xdeadbeef)

/**
 * @brief Lock flags.
 */
#define MR_LFLAG_RD                     ((0x01) << 24)              /**< Read lock */
#define MR_LFLAG_WR                     ((0x02) << 24)              /**< Write lock */
#define MR_LFLAG_RDWR                   ((0x03) << 24)              /**< Read/write lock */
#define MR_LFLAG_NONBLOCK               ((0x04) << 24)              /**< Non-blocking lock */
#define MR_LFLAG_SLEEP                  ((0x08) << 24)              /**< Sleep lock */

/**
 * @brief Open flags.
 */
#define MR_OFLAG_CLOSED                 (0)                         /**< Closed */
#define MR_OFLAG_RDONLY                 ((0x01) << 24)              /**< Read only */
#define MR_OFLAG_WRONLY                 ((0x02) << 24)              /**< Write only */
#define MR_OFLAG_RDWR                   ((0x03) << 24)              /**< Read/write */
#define MR_OFLAG_NONBLOCK               ((0x04) << 24)              /**< Non-blocking */
#define MR_OFLAG_DMA                    ((0x08) << 24)              /**< DMA */

/**
 * @brief Support flags.
 */
#define MR_SFLAG_NONRDWR                MR_OFLAG_CLOSED             /**< Non-read/write */
#define MR_SFLAG_RDONLY                 MR_OFLAG_RDONLY             /**< Read only */
#define MR_SFLAG_WRONLY                 MR_OFLAG_WRONLY             /**< Write only */
#define MR_SFLAG_RDWR                   MR_OFLAG_RDWR               /**< Read/write */
#define MR_SFLAG_NONBLOCK               MR_OFLAG_NONBLOCK           /**< Non-blocking */
#define MR_SFLAG_DMA                    MR_OFLAG_DMA                /**< DMA */
#define MR_SFLAG_NONDRV                 ((0x10) << 24)              /**< Non-driver */
#define MR_SFLAG_ONLY                   ((0x20) << 24)              /**< Only */

/**
 * @brief Device structure.
 */
struct mr_dev
{
    uint32_t magic;                                                 /**< Magic number */
#ifndef MR_CFG_NAME_MAX
#define MR_CFG_NAME_MAX                 (8)
#endif /* MR_CFG_NAME_MAX */
    char name[MR_CFG_NAME_MAX];                                     /**< Name */
    struct mr_list list;                                            /**< List */
    struct mr_list slist;                                           /**< Slave list */
    void *link;                                                     /**< Link */

    uint32_t type;                                                  /**< Device type */
    size_t ref_count;                                               /**< Reference count */
#ifdef MR_USING_RDWR_CTL
    uint32_t sflags;                                                /**< Support flags */
    volatile uint32_t lflags;                                       /**< Lock flags */
#endif /* MR_USING_RDWR_CTL */

    struct
    {
        int desc;                                                   /**< Device descriptor */
        int (*call)(int desc, void *args);                          /**< Callback function */
    } rd_call, wr_call;                                             /**< Read/write callback */

    const struct mr_dev_ops *ops;                                   /**< Device operations */
    const struct mr_drv *drv;                                       /**< Driver */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_DEF_H_ */
