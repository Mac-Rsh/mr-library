/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-22    MacRsh       First version
 */

#ifndef _MR_CAN_H_
#define _MR_CAN_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_CAN

/**
 * @addtogroup CAN
 * @{
 */

/**
 * @brief CAN mode.
 */
#define MR_CAN_MODE_NORMAL              (0)                         /**< Normal mode */
#define MR_CAN_MODE_LOOPBACK            (1)                         /**< Loopback mode */
#define MR_CAN_MODE_SILENT              (2)                         /**< Silent mode */
#define MR_CAN_MODE_SILENT_LOOPBACK     (3)                         /**< Silent loopback mode */

/**
 * @brief CAN default configuration.
 */
#define MR_CAN_CONFIG_DEFAULT           \
{                                       \
    500000,                             \
    MR_CAN_MODE_NORMAL,                 \
}

/**
 * @brief CAN configuration structure.
 */
struct mr_can_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
    int mode;                                                       /**< Mode */
};

/**
 * @brief CAN control command.
 */
#define MR_IOC_CAN_SET_CONFIG           MR_IOC_SCFG                 /**< Set configuration command */
#define MR_IOC_CAN_SET_RD_BUFSZ         MR_IOC_SRBSZ                /**< Set read buffer size command */
#define MR_IOC_CAN_CLR_RD_BUF           MR_IOC_CRBD                 /**< Clear read buffer command */
#define MR_IOC_CAN_SET_RD_CALL          MR_IOC_SRCB                 /**< Set read callback command */
#define MR_IOC_CAN_REMOTE_REQUEST       (0x01)                      /**< Remote request command */

#define MR_IOC_CAN_GET_CONFIG           MR_IOC_GCFG                 /**< Get configuration command */
#define MR_IOC_CAN_GET_RD_BUFSZ         MR_IOC_GRBSZ                /**< Get read buffer size command */
#define MR_IOC_CAN_GET_RD_DATASZ        MR_IOC_GRBDSZ               /**< Get read data size command */
#define MR_IOC_CAN_GET_RD_CALL          MR_IOC_GRCB                 /**< Get read callback command */

/**
 * @brief CAN data type.
 */
typedef uint8_t mr_can_data_t;                                      /**< CAN read/write data type */

/**
* @brief CAN ISR events.
*/
#define MR_ISR_CAN_RD_INT               (MR_ISR_RD | (0x01 << 8))   /**< Read interrupt event */

/**
 * @brief CAN bus structure.
 */
struct mr_can_bus
{
    struct mr_dev dev;                                              /**< Device */

    struct mr_can_config config;                                    /**< Configuration */
    volatile void *owner;                                           /**< Owner */
    volatile int hold;                                              /**< Owner hold */
};

/**
 * @brief CAN bus operations structure.
 */
struct mr_can_bus_ops
{
    int (*configure)(struct mr_can_bus *can_bus, struct mr_can_config *config);
    int (*filter_configure)(struct mr_can_bus *can_bus, int id, int ide, int state);
    int (*read)(struct mr_can_bus *can_bus, int *id, int *ide, int *rtr, uint8_t *buf, size_t size);
    ssize_t (*write)(struct mr_can_bus *can_bus, int id, int ide, const uint8_t *buf, size_t size);
    int (*remote_request)(struct mr_can_bus *can_bus, int id, int ide);
};

/**
 * @brief CAN ID type.
 */
#define MR_CAN_IDE_STD                  (0)                         /**< Standard ID */
#define MR_CAN_IDE_EXT                  (1)                         /**< Extended ID */

/**
 * @brief CAN device structure.
 */
struct mr_can_dev
{
    struct mr_dev dev;                                              /**< Device structure */

    struct mr_can_config config;                                    /**< Configuration */
    struct mr_ringbuf rd_fifo;                                      /**< Read FIFO */
    size_t rd_bufsz;                                                /**< Read buffer size */
    int id;                                                         /**< ID */
    int ide;                                                        /**< ID type */
};

int mr_can_bus_register(struct mr_can_bus *can_bus, const char *path, struct mr_drv *drv);
int mr_can_dev_register(struct mr_can_dev *can_dev, const char *path, int id, int ide);
/** @} */

#endif /* MR_USING_CAN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_CAN_H_ */
