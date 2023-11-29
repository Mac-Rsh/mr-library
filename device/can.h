/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-22    MacRsh       First version
 */

#ifndef _CAN_H_
#define _CAN_H_

#include "mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_CAN

/**
 * @brief CAN default configuration.
 */
#define MR_CAN_CONFIG_DEFAULT           \
{                                       \
    500000,                             \
}

/**
 * @brief CAN configuration structure.
 */
struct mr_can_config
{
    uint32_t baud_rate;                                             /**< Baud rate */
};

/**
 * @brief CAN identifier.
 */
#define MR_CAN_IDE_STD                  (0 << 29)
#define MR_CAN_IDE_EXT                  (1 << 29)

/**
 * @brief CAN remote transmission request.
 */
#define MR_CAN_RTR_DATA                 (0 << 30)
#define MR_CAN_RTR_REMOTE               (1 << 30)

/**
 * @brief Help to set id.
 */
#define MR_CAN_ID(id, ide, rtr)         ((id) | (ide) | (rtr))      /**< Set id-ide-rtr */

/**
 * @brief CAN control command.
 */
#define MR_CTL_CAN_SET_ID               MR_CTL_SET_OFFSET           /**< Set id */
#define MR_CTL_CAN_SET_RD_CALL          MR_CTL_SET_RD_CALL          /**< Set read call */
#define MR_CTL_CAN_GET_ID               MR_CTL_GET_OFFSET           /**< Get id */
#define MR_CTL_CAN_GET_RD_CALL          MR_CTL_GET_RD_CALL          /**< Get read call */

/**
 * @brief CAN data type.
 */
typedef uint8_t mr_can_data_t;                                      /**< CAN read/write data type */

/**
* @brief CAN ISR events.
*/
#define MR_ISR_CAN_RD_INT               (MR_ISR_RD | (0x01 << 16))     /**< Read interrupt */

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
    int (*get_id)(struct mr_can_bus *can_bus);
    ssize_t (*read)(struct mr_can_bus *can_bus, uint8_t *buf, size_t size);
    ssize_t (*write)(struct mr_can_bus *can_bus, int id, int ide, int rtr, const uint8_t *buf, size_t size);
};

/**
 * @brief CAN id identifier.
 */
#define MR_CAN_ID_STD                   (0)                         /**< Standard identifier */
#define MR_CAN_ID_EXT                   (1)                         /**< Extended identifier */

/**
 * @brief CAN device structure.
 */
struct mr_can_dev
{
    struct mr_dev dev;

    struct mr_can_config config;
    struct mr_ringbuf rd_fifo;
    size_t rd_bufsz;
    uint32_t id: 29;
    uint32_t ide: 1;
    uint32_t reserved: 2;
};

/**
 * @addtogroup CAN.
 * @{
 */
int mr_can_bus_register(struct mr_can_bus *can_bus, const char *name, struct mr_drv *drv);
int mr_can_dev_register(struct mr_can_dev *can_dev, const char *name, int id, int ide);
/** @} */
#endif /* MR_USING_CAN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CAN_H_ */
