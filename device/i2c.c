/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-09    MacRsh       First version
 */

#include "include/device/mr_i2c.h"

#ifdef MR_USING_I2C

static int mr_i2c_bus_open(struct mr_dev *dev)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)dev;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)dev->drv->ops;

    /* Reset the hold */
    i2c_bus->hold = MR_FALSE;

    /* Default address is 0x00 and 7-bit */
    return ops->configure(i2c_bus, &i2c_bus->config, 0x00, MR_I2C_ADDR_BITS_7);
}

static int mr_i2c_bus_close(struct mr_dev *dev)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)dev;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)dev->drv->ops;
    struct mr_i2c_config close_config = {0};

    return ops->configure(i2c_bus, &close_config, 0x00, MR_I2C_ADDR_BITS_7);
}

static ssize_t mr_i2c_bus_read(struct mr_dev *dev, void *buf, size_t count)
{
    return MR_ENOTSUP;
}

static ssize_t mr_i2c_bus_write(struct mr_dev *dev, const void *buf, size_t count)
{
    return MR_ENOTSUP;
}

static ssize_t mr_i2c_bus_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)dev;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)dev->drv->ops;

    switch (event) {
        case MR_ISR_I2C_RD_INT: {
            struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)i2c_bus->owner;
            uint8_t data;

            /* Read data to FIFO */
            int ret = ops->read(i2c_bus, &data, MR_ENABLE);
            if (ret < 0) {
                return ret;
            }
            mr_ringbuf_push_force(&i2c_dev->rd_fifo, data);

            /* Call the i2c-dev ISR */
            return mr_dev_isr(&i2c_dev->dev, event, MR_NULL);
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a i2c-bus.
 *
 * @param i2c_bus The i2c-bus.
 * @param path The path of the i2c-bus.
 * @param drv The driver of the i2c-bus.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_i2c_bus_register(struct mr_i2c_bus *i2c_bus, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops = {mr_i2c_bus_open,
                                    mr_i2c_bus_close,
                                    mr_i2c_bus_read,
                                    mr_i2c_bus_write,
                                    MR_NULL,
                                    mr_i2c_bus_isr};
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    MR_ASSERT(i2c_bus != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    i2c_bus->config = default_config;
    i2c_bus->owner = MR_NULL;
    i2c_bus->hold = MR_FALSE;

    /* Register the i2c-bus */
    return mr_dev_register(&i2c_bus->dev, path, MR_DEV_TYPE_I2C, MR_O_RDWR, &ops, drv);
}

MR_INLINE int i2c_dev_take_bus(struct mr_i2c_dev *i2c_dev)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)i2c_bus->dev.drv->ops;

    /* Check if the bus is busy */
    if ((i2c_bus->hold == MR_TRUE) && (i2c_dev != i2c_bus->owner)) {
        return MR_EBUSY;
    }

    /* If the owner changes, recheck the configuration */
    if (i2c_dev != i2c_bus->owner) {
        if (i2c_dev->config.baud_rate != i2c_bus->config.baud_rate ||
            i2c_dev->config.host_slave != i2c_bus->config.host_slave) {
            int addr = (i2c_dev->config.host_slave == MR_I2C_HOST) ? 0x00 : i2c_dev->addr;

            int ret = ops->configure(i2c_bus, &i2c_dev->config, addr, i2c_dev->addr_bits);
            if (ret < 0) {
                return ret;
            }
        }
        i2c_bus->config = i2c_dev->config;
        i2c_bus->owner = i2c_dev;
    }
    i2c_bus->hold = MR_TRUE;
    return MR_EOK;
}

MR_INLINE int i2c_dev_release_bus(struct mr_i2c_dev *i2c_dev)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;

    if (i2c_dev != i2c_bus->owner) {
        return MR_EINVAL;
    }

    /* If it is a host, release the bus. The slave needs to hold the bus at all times */
    if (i2c_dev->config.host_slave == MR_I2C_HOST) {
        i2c_bus->hold = MR_FALSE;
    }
    return MR_EOK;
}

#define MR_I2C_RD                       (0)
#define MR_I2C_WR                       (1)

MR_INLINE int i2c_dev_send_addr(struct mr_i2c_dev *i2c_dev, int rdwr)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)i2c_bus->dev.drv->ops;
    int addr, addr_bits;

    /* Get the address, otherwise use the 0x00 */
    addr = i2c_dev->addr;
    addr_bits = i2c_dev->addr_bits;

    /* Set the read command */
    addr = (0xf000 | ((addr >> 8) & 0x03) << 9) | (addr & 0xff);
    if (rdwr == MR_I2C_RD) {
        addr |= (addr_bits == MR_I2C_ADDR_BITS_7) ? 0x01 : 0x10;
    }

    ops->start(i2c_bus);
    int ret = ops->send_addr(i2c_bus, addr, addr_bits);
    if (ret < 0) {
        ops->stop(i2c_bus);
    }
    return ret;
}

MR_INLINE void i2c_dev_send_stop(struct mr_i2c_dev *i2c_dev)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)i2c_bus->dev.drv->ops;

    ops->stop(i2c_bus);
}

MR_INLINE ssize_t i2c_dev_read(struct mr_i2c_dev *i2c_dev, uint8_t *buf, size_t count)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)i2c_bus->dev.drv->ops;
    ssize_t rd_size;

    for (rd_size = 0; rd_size < count; rd_size += sizeof(*buf)) {
        int ack = ((count - rd_size) != sizeof(*buf));

        int ret = ops->read(i2c_bus, buf, ack);
        if (ret < 0) {
            return (rd_size == 0) ? ret : rd_size;
        }
        buf++;
    }
    return rd_size;
}

MR_INLINE ssize_t i2c_dev_write(struct mr_i2c_dev *i2c_dev, const uint8_t *buf, size_t count)
{
    struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)i2c_dev->dev.parent;
    struct mr_i2c_bus_ops *ops = (struct mr_i2c_bus_ops *)i2c_bus->dev.drv->ops;
    ssize_t wr_size;

    for (wr_size = 0; wr_size < count; wr_size += sizeof(*buf)) {
        int ret = ops->write(i2c_bus, *buf);
        if (ret < 0) {
            return (wr_size == 0) ? ret : wr_size;
        }
        buf++;
    }
    return wr_size;
}

static int mr_i2c_dev_open(struct mr_dev *dev)
{
    struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)dev;

    return mr_ringbuf_allocate(&i2c_dev->rd_fifo, i2c_dev->rd_bufsz);
}

static int mr_i2c_dev_close(struct mr_dev *dev)
{
    struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)dev;

    mr_ringbuf_free(&i2c_dev->rd_fifo);
    return MR_EOK;
}

static ssize_t mr_i2c_dev_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)dev;

    ssize_t ret = i2c_dev_take_bus(i2c_dev);
    if (ret < 0) {
        return ret;
    }

    if (i2c_dev->config.host_slave == MR_I2C_HOST) {
        /* Send the address of the register that needs to be read */
        if (dev->position >= 0) {
            ret = i2c_dev_send_addr(i2c_dev, MR_I2C_WR);
            if (ret < 0) {
                goto release_bus;
            }

            ret = i2c_dev_write(i2c_dev,
                                (uint8_t *)&dev->position,
                                (i2c_dev->config.reg_bits >> 3));
            if (ret < 0) {
                goto release_bus;
            }
        }

        ret = i2c_dev_send_addr(i2c_dev, MR_I2C_RD);
        if (ret < 0) {
            goto release_bus;
        }
        ret = i2c_dev_read(i2c_dev, (uint8_t *)buf, count);
        i2c_dev_send_stop(i2c_dev);
    } else {
        ret = (ssize_t)mr_ringbuf_read(&i2c_dev->rd_fifo, buf, count);
    }

    release_bus:
    i2c_dev_release_bus(i2c_dev);
    return ret;
}

static ssize_t mr_i2c_dev_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)dev;

    ssize_t ret = i2c_dev_take_bus(i2c_dev);
    if (ret < 0) {
        return ret;
    }

    if (i2c_dev->config.host_slave == MR_I2C_HOST) {
        ret = i2c_dev_send_addr(i2c_dev, MR_I2C_WR);
        if (ret < 0) {
            goto release_bus;
        }

        /* Send the address of the register that needs to be written */
        if (dev->position >= 0) {
            ret = i2c_dev_write(i2c_dev,
                                (uint8_t *)&dev->position,
                                (i2c_dev->config.reg_bits >> 3));
            if (ret < 0) {
                goto release_bus;
            }
        }

        ret = i2c_dev_write(i2c_dev, (uint8_t *)buf, count);
        i2c_dev_send_stop(i2c_dev);
    } else {
        ret = i2c_dev_write(i2c_dev, (uint8_t *)buf, count);
    }

    release_bus:
    i2c_dev_release_bus(i2c_dev);
    return ret;
}

static int mr_i2c_dev_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_i2c_dev *i2c_dev = (struct mr_i2c_dev *)dev;

    switch (cmd) {
        case MR_IOC_I2C_SET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_i2c_bus *i2c_bus = (struct mr_i2c_bus *)dev->parent;
                struct mr_i2c_config config = *(struct mr_i2c_config *)args;

                /* If holding the bus, release it */
                if (i2c_dev == i2c_bus->owner) {
                    i2c_bus->hold = MR_FALSE;
                    i2c_bus->owner = MR_NULL;
                }

                /* Update the configuration and try again to get the bus */
                i2c_dev->config = config;
                if (config.host_slave == MR_I2C_SLAVE) {
                    int ret = i2c_dev_take_bus(i2c_dev);
                    if (ret < 0) {
                        return ret;
                    }
                }
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_I2C_SET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&i2c_dev->rd_fifo, bufsz);
                i2c_dev->rd_bufsz = 0;
                if (ret < 0) {
                    return ret;
                }
                i2c_dev->rd_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_I2C_CLR_RD_BUF: {
            mr_ringbuf_reset(&i2c_dev->rd_fifo);
            return MR_EOK;
        }
        case MR_IOC_I2C_GET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_i2c_config *config = (struct mr_i2c_config *)args;

                *config = i2c_dev->config;
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_I2C_GET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = i2c_dev->rd_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_I2C_GET_RD_DATASZ: {
            if (args != MR_NULL) {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_bufsz(&i2c_dev->rd_fifo);
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
 * @brief This function registers a i2c-device.
 *
 * @param i2c_dev The i2c-device.
 * @param path The path of the i2c-device.
 * @param addr The address of the i2c-device.
 * @param addr_bits The number of address bits of the i2c-device.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_i2c_dev_register(struct mr_i2c_dev *i2c_dev, const char *path, int addr, int addr_bits)
{
    static struct mr_dev_ops ops = {mr_i2c_dev_open,
                                    mr_i2c_dev_close,
                                    mr_i2c_dev_read,
                                    mr_i2c_dev_write,
                                    mr_i2c_dev_ioctl,
                                    MR_NULL};
    struct mr_i2c_config default_config = MR_I2C_CONFIG_DEFAULT;

    MR_ASSERT(i2c_dev != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT((addr_bits == MR_I2C_ADDR_BITS_7) || (addr_bits == MR_I2C_ADDR_BITS_10));
    MR_ASSERT((addr_bits != MR_I2C_ADDR_BITS_7) || (addr >= 0 && addr <= 0x7f));
    MR_ASSERT((addr_bits != MR_I2C_ADDR_BITS_10) || (addr >= 0 && addr <= 0x3ff));

    /* Initialize the fields */
    i2c_dev->config = default_config;
    mr_ringbuf_init(&i2c_dev->rd_fifo, MR_NULL, 0);
#ifndef MR_CFG_I2C_RD_BUFSZ
#define MR_CFG_I2C_RD_BUFSZ             (0)
#endif /* MR_CFG_I2C_RD_BUFSZ */
    i2c_dev->rd_bufsz = MR_CFG_I2C_RD_BUFSZ;
    i2c_dev->addr = addr;
    i2c_dev->addr_bits = addr_bits;

    /* Register the i2c-device */
    return mr_dev_register(&i2c_dev->dev, path, MR_DEV_TYPE_I2C, MR_O_RDWR, &ops, MR_NULL);
}

#endif /* MR_USING_I2C */
