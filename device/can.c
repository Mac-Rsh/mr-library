/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-22    MacRsh       First version
 */

#include "include/device/mr_can.h"

#ifdef MR_USING_CAN

static int mr_can_bus_open(struct mr_dev *dev)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)dev;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)dev->drv->ops;

    /* Reset the hold */
    can_bus->hold = MR_FALSE;

    return ops->configure(can_bus, &can_bus->config);
}

static int mr_can_bus_close(struct mr_dev *dev)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)dev;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)dev->drv->ops;
    struct mr_can_config close_config = {0};

    return ops->configure(can_bus, &close_config);
}

static ssize_t mr_can_bus_read(struct mr_dev *dev, void *buf, size_t count)
{
    return MR_EIO;
}

static ssize_t mr_can_bus_write(struct mr_dev *dev, const void *buf, size_t count)
{
    return MR_EIO;
}

static ssize_t mr_can_bus_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)dev;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)dev->drv->ops;

    switch (event) {
        case MR_ISR_CAN_RD_INT: {
            uint8_t data[8] = {0};
            int id, ide, rtr;

            /* Read id and data to FIFO */
            ssize_t ret = ops->read(can_bus, &id, &ide, &rtr, data, sizeof(data));
            if (ret < 0) {
                return ret;
            }
            id = (ide == MR_CAN_IDE_STD) ? (id & 0x7ff) : (id & 0x1fffffff);

            /* Search the matching device */
            for (struct mr_list *list = dev->clist.next; list != &dev->clist; list = list->next) {
                struct mr_can_dev *can_dev = (struct mr_can_dev *)MR_CONTAINER_OF(list,
                                                                                  struct mr_dev,
                                                                                  list);

                /* Check id is valid */
                if (can_dev->id == id) {
                    mr_ringbuf_write_force(&can_dev->rd_fifo, data, ret);

                    return mr_dev_isr(&can_dev->dev, event, &rtr);
                }
            }
            return MR_ENOTFOUND;
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a can-bus.
 *
 * @param can_dev The can-device.
 * @param path The path of the can-bus.
 * @param drv The driver of the can-bus.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_can_bus_register(struct mr_can_bus *can_bus, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops = {mr_can_bus_open,
                                    mr_can_bus_close,
                                    mr_can_bus_read,
                                    mr_can_bus_write,
                                    MR_NULL,
                                    mr_can_bus_isr};
    struct mr_can_config default_config = MR_CAN_CONFIG_DEFAULT;

    MR_ASSERT(can_bus != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    can_bus->config = default_config;
    can_bus->owner = MR_NULL;

    /* Register the can-bus */
    return mr_dev_register(&can_bus->dev, path, MR_DEV_TYPE_CAN, MR_O_RDWR, &ops, drv);
}

MR_INLINE int can_dev_filter_configure(struct mr_can_dev *can_dev, int id, int ide, int state)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)can_dev->dev.parent;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)can_bus->dev.drv->ops;

    return ops->filter_configure(can_bus, id, ide, state);
}

MR_INLINE int can_dev_take_bus(struct mr_can_dev *can_dev)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)can_dev->dev.parent;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)can_bus->dev.drv->ops;

    if ((can_bus->hold == MR_TRUE) && (can_dev != can_bus->owner)) {
        return MR_EBUSY;
    }

    /* If the owner changes, recheck the configuration */
    if (can_dev != can_bus->owner) {
        if (can_dev->config.baud_rate != can_bus->config.baud_rate) {
            int ret = ops->configure(can_bus, &can_dev->config);
            if (ret < 0) {
                return ret;
            }
        }
        can_bus->config = can_dev->config;
        can_bus->owner = can_dev;
    }
    can_bus->hold = MR_TRUE;
    return MR_EOK;
}

MR_INLINE int can_dev_release_bus(struct mr_can_dev *can_dev)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)can_dev->dev.parent;

    if (can_dev != can_bus->owner) {
        return MR_EINVAL;
    }

    can_bus->hold = MR_FALSE;
    return MR_EOK;
}

MR_INLINE ssize_t can_dev_write(struct mr_can_dev *can_dev,
                                int id,
                                int ide,
                                const uint8_t *buf,
                                size_t count)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)can_dev->dev.parent;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)can_bus->dev.drv->ops;

    return ops->write(can_bus, id, ide, buf, count);
}

MR_INLINE int can_dev_request(struct mr_can_dev *can_dev, int id, int ide)
{
    struct mr_can_bus *can_bus = (struct mr_can_bus *)can_dev->dev.parent;
    struct mr_can_bus_ops *ops = (struct mr_can_bus_ops *)can_bus->dev.drv->ops;

    return ops->remote_request(can_bus, id, ide);
}

static int mr_can_dev_open(struct mr_dev *dev)
{
    struct mr_can_dev *can_dev = (struct mr_can_dev *)dev;

    int ret = mr_ringbuf_allocate(&can_dev->rd_fifo, can_dev->rd_bufsz);
    if (ret < 0) {
        return ret;
    }

    return can_dev_filter_configure(can_dev, can_dev->id, can_dev->ide, MR_ENABLE);
}

static int mr_can_dev_close(struct mr_dev *dev)
{
    struct mr_can_dev *can_dev = (struct mr_can_dev *)dev;

    mr_ringbuf_free(&can_dev->rd_fifo);

    return can_dev_filter_configure(can_dev, can_dev->id, can_dev->ide, MR_DISABLE);
}

static ssize_t mr_can_dev_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_can_dev *can_dev = (struct mr_can_dev *)dev;

    ssize_t ret = can_dev_take_bus(can_dev);
    if (ret < 0) {
        return ret;
    }

    ret = (ssize_t)mr_ringbuf_read(&can_dev->rd_fifo, buf, count);

    can_dev_release_bus(can_dev);
    return ret;
}

static ssize_t mr_can_dev_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_can_dev *can_dev = (struct mr_can_dev *)dev;

    ssize_t ret = can_dev_take_bus(can_dev);
    if (ret < 0) {
        return ret;
    }

    ret = can_dev_write(can_dev, can_dev->id, can_dev->ide, (uint8_t *)buf, count);

    can_dev_release_bus(can_dev);
    return ret;
}

static int mr_can_dev_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_can_dev *can_dev = (struct mr_can_dev *)dev;

    switch (cmd) {
        case MR_IOC_CAN_SET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_can_bus *can_bus = (struct mr_can_bus *)dev->parent;
                struct mr_can_config config = *(struct mr_can_config *)args;

                /* If holding the bus, release it */
                if (can_dev == can_bus->owner) {
                    can_bus->hold = MR_FALSE;
                    can_bus->owner = MR_NULL;
                }
                can_dev->config = config;
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_CAN_SET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&can_dev->rd_fifo, bufsz);
                can_dev->rd_bufsz = 0;
                if (ret < 0) {
                    return ret;
                }
                can_dev->rd_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_CAN_CLR_RD_BUF: {
            mr_ringbuf_reset(&can_dev->rd_fifo);
            return MR_EOK;
        }
        case MR_IOC_CAN_REMOTE_REQUEST: {
            return can_dev_request(can_dev, can_dev->id, can_dev->ide);
        }
        case MR_IOC_CAN_GET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_can_config *config = (struct mr_can_config *)args;

                *config = can_dev->config;
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_CAN_GET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = can_dev->rd_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_CAN_GET_RD_DATASZ: {
            if (args != MR_NULL) {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_data_size(&can_dev->rd_fifo);
                return sizeof(*datasz);
            }
            return MR_EINVAL;
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a can-device.
 *
 * @param can_dev The can-device.
 * @param path The path of the can-device.
 * @param id The id of the can-device.
 * @param ide The id identifier of the can-device.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_can_dev_register(struct mr_can_dev *can_dev, const char *path, int id, int ide)
{
    static struct mr_dev_ops ops = {mr_can_dev_open,
                                    mr_can_dev_close,
                                    mr_can_dev_read,
                                    mr_can_dev_write,
                                    mr_can_dev_ioctl,
                                    MR_NULL};
    struct mr_can_config default_config = MR_CAN_CONFIG_DEFAULT;

    MR_ASSERT(can_dev != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT((ide == MR_CAN_IDE_STD) || (ide == MR_CAN_IDE_EXT));
    MR_ASSERT((ide != MR_CAN_IDE_STD) || ((id >= 0) && (id <= 0x7ff)));
    MR_ASSERT((ide != MR_CAN_IDE_EXT) || ((id >= 0) && (id <= 0x1fffffff)));

    /* Initialize the fields */
    can_dev->config = default_config;
    mr_ringbuf_init(&can_dev->rd_fifo, MR_NULL, 0);
#ifndef MR_CFG_CAN_RD_BUFSZ
#define MR_CFG_CAN_RD_BUFSZ             (0)
#endif /* MR_CFG_CAN_RD_BUFSZ */
    can_dev->rd_bufsz = MR_CFG_CAN_RD_BUFSZ;
    can_dev->id = id;
    can_dev->ide = ide;

    /* Register the can-device */
    return mr_dev_register(&can_dev->dev, path, MR_DEV_TYPE_CAN, MR_O_RDWR, &ops, MR_NULL);
}

#endif /* MR_USING_CAN */
