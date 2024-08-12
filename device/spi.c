/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#include "include/device/mr_spi.h"

#ifdef MR_USING_SPI

#ifdef MR_USING_PIN
#include "include/device/mr_pin.h"
#else
#warning "Please define MR_USING_PIN. Otherwise SPI-CS will not work."
#endif /* MR_USING_PIN */

static int mr_spi_bus_open(struct mr_dev *dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;

    /* Reset the hold */
    spi_bus->hold = MR_FALSE;
#ifdef MR_USING_PIN
    spi_bus->cs_desc = mr_dev_open("pin", MR_O_RDWR);
#endif /* MR_USING_PIN */
    return ops->configure(spi_bus, &spi_bus->config);
}

static int mr_spi_bus_close(struct mr_dev *dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;
    struct mr_spi_config close_config = {0};

#ifdef MR_USING_PIN
    if (spi_bus->cs_desc >= 0) {
        mr_dev_close(spi_bus->cs_desc);
        spi_bus->cs_desc = -1;
    }
#endif /* MR_USING_PIN */
    return ops->configure(spi_bus, &close_config);
}

static ssize_t mr_spi_bus_read(struct mr_dev *dev, void *buf, size_t count)
{
    return MR_EIO;
}

static ssize_t mr_spi_bus_write(struct mr_dev *dev, const void *buf, size_t count)
{
    return MR_EIO;
}

static ssize_t mr_spi_bus_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;

    switch (event) {
        case MR_ISR_SPI_RD_INT: {
            struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)spi_bus->owner;
            uint8_t data;

            /* Read data to FIFO */
            int ret = ops->read(spi_bus, &data);
            if (ret < 0) {
                return ret;
            }
            mr_ringbuf_write_force(&spi_dev->rd_fifo, &data, sizeof(data));

            /* Call the spi-dev ISR */
            return mr_dev_isr(&spi_dev->dev, event, MR_NULL);
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a spi-bus.
 *
 * @param spi_bus The spi-bus.
 * @param path The path of the spi-bus.
 * @param drv The driver of the spi-bus.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops = {mr_spi_bus_open,
                                    mr_spi_bus_close,
                                    mr_spi_bus_read,
                                    mr_spi_bus_write,
                                    MR_NULL,
                                    mr_spi_bus_isr};
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    MR_ASSERT(spi_bus != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    spi_bus->config = default_config;
    spi_bus->owner = MR_NULL;
    spi_bus->hold = MR_FALSE;
    spi_bus->cs_desc = -1;

    /* Register the spi-bus */
    return mr_dev_register(&spi_bus->dev, path, MR_DEV_TYPE_SPI, MR_O_RDWR, &ops, drv);
}

#ifdef MR_USING_PIN
static void spi_dev_cs_configure(struct mr_spi_dev *spi_dev, int state)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.parent;
    int desc = spi_bus->cs_desc, old_number = -1;

    if (desc < 0) {
        return;
    }

    if (spi_dev->cs_pin != -1) {
        /* Temporarily store the old number */
        mr_dev_ioctl(desc, MR_IOC_GPOS, &old_number);

        /* Set the new number */
        mr_dev_ioctl(desc, MR_IOC_SPOS, MR_MAKE_LOCAL(int, spi_dev->cs_pin));
        if (state == MR_ENABLE) {
            if (spi_dev->config.host_slave == MR_SPI_HOST) {
                mr_dev_ioctl(desc, MR_IOC_SCFG, MR_MAKE_LOCAL(int, MR_PIN_MODE_OUTPUT));
                mr_dev_write(desc, MR_MAKE_LOCAL(uint8_t, !spi_dev->cs_active), sizeof(uint8_t));
            } else {
                mr_dev_ioctl(desc, MR_IOC_SCFG, MR_MAKE_LOCAL(int, MR_PIN_MODE_NONE));
            }
        } else {
            mr_dev_ioctl(desc, MR_IOC_SCFG, MR_MAKE_LOCAL(int, MR_PIN_MODE_NONE));
        }

        /* Restore the old number */
        mr_dev_ioctl(desc, MR_IOC_SPOS, &old_number);
    }
}
#endif /* MR_USING_PIN */

MR_INLINE void spi_dev_cs_set(struct mr_spi_dev *spi_dev, int state)
{
#ifdef MR_USING_PIN
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.parent;

    if (spi_bus->cs_desc < 0) {
        return;
    }

    /* Set the new state */
    mr_dev_write(spi_bus->cs_desc,
                 MR_MAKE_LOCAL(uint8_t, !(state ^ spi_dev->cs_active)),
                 sizeof(uint8_t));
#endif /* MR_USING_PIN */
}

MR_INLINE int spi_dev_take_bus(struct mr_spi_dev *spi_dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.parent;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    /* Check if the bus is busy */
    if ((spi_bus->hold == MR_TRUE) && (spi_dev != spi_bus->owner)) {
        return MR_EBUSY;
    }

    if (spi_dev != spi_bus->owner) {
        /* Reconfigure the bus */
        if (spi_dev->config.baud_rate != spi_bus->config.baud_rate ||
            spi_dev->config.host_slave != spi_bus->config.host_slave ||
            spi_dev->config.mode != spi_bus->config.mode ||
            spi_dev->config.bit_order != spi_bus->config.bit_order) {
            int ret = ops->configure(spi_bus, &spi_dev->config);
            if (ret < 0) {
                return ret;
            }
        }
        spi_bus->config = spi_dev->config;
        spi_bus->owner = spi_dev;
#ifdef MR_USING_PIN
        if ((spi_bus->cs_desc >= 0) && (spi_bus->config.host_slave == MR_SPI_HOST)) {
            mr_dev_ioctl(spi_bus->cs_desc, MR_IOC_SPOS, MR_MAKE_LOCAL(int, spi_dev->cs_pin));
        }
#endif /* MR_USING_PIN */
    }
    spi_bus->hold = MR_TRUE;
    return MR_EOK;
}

MR_INLINE int spi_dev_release_bus(struct mr_spi_dev *spi_dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.parent;

    if (spi_dev != spi_bus->owner) {
        return MR_EINVAL;
    }

    /* If it is a host, release the bus. The slave needs to hold the bus at all times */
    if (spi_dev->config.host_slave == MR_SPI_HOST) {
        spi_bus->hold = MR_FALSE;
    }
    return MR_EOK;
}

#define MR_SPI_RD                       (0)
#define MR_SPI_WR                       (1)
#define MR_SPI_RDWR                     (2)

static ssize_t spi_dev_transfer(struct mr_spi_dev *spi_dev,
                                uint8_t *rd_buf,
                                const uint8_t *wr_buf,
                                size_t size,
                                int rdwr)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.parent;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;
    ssize_t tf_size;

    if (rdwr == MR_SPI_RD) {
        for (tf_size = 0; tf_size < size; tf_size += sizeof(*rd_buf)) {
            ops->write(spi_bus, 0);
            int ret = ops->read(spi_bus, rd_buf);
            if (ret < 0) {
                return (tf_size == 0) ? ret : tf_size;
            }
            rd_buf++;
        }
    } else if (rdwr == MR_SPI_WR) {
        for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_buf)) {
            int ret = ops->write(spi_bus, *wr_buf);
            if (ret < 0) {
                return (tf_size == 0) ? ret : tf_size;
            }
            ops->read(spi_bus, MR_MAKE_LOCAL(uint8_t, 0));
            wr_buf++;
        }
    } else {
        for (tf_size = 0;
             tf_size < MR_ALIGN_DOWN(size, sizeof(*wr_buf));
             tf_size += sizeof(*wr_buf)) {
            int ret = ops->write(spi_bus, *wr_buf);
            if (ret < 0) {
                return (tf_size == 0) ? ret : tf_size;
            }

            ret = ops->read(spi_bus, rd_buf);
            if (ret < 0) {
                return (tf_size == 0) ? ret : tf_size;
            }
            rd_buf++;
            wr_buf++;
        }
    }
    return (ssize_t)tf_size;
}

static int mr_spi_dev_open(struct mr_dev *dev)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

#ifdef MR_USING_PIN
    spi_dev_cs_configure(spi_dev, MR_ENABLE);
#endif /* MR_USING_PIN */

    /* Allocate FIFO buffers */
    return mr_ringbuf_allocate(&spi_dev->rd_fifo, spi_dev->rd_bufsz);
}

static int mr_spi_dev_close(struct mr_dev *dev)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

#ifdef MR_USING_PIN
    spi_dev_cs_configure(spi_dev, MR_DISABLE);
#endif /* MR_USING_PIN */

    /* Free FIFO buffers */
    mr_ringbuf_free(&spi_dev->rd_fifo);
    return MR_EOK;
}

static ssize_t mr_spi_dev_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    ssize_t ret = spi_dev_take_bus(spi_dev);
    if (ret < 0) {
        return ret;
    }

    if (spi_dev->config.host_slave == MR_SPI_HOST) {
        spi_dev_cs_set(spi_dev, MR_ENABLE);

        /* Send the address of the register that needs to be read */
        if (dev->position >= 0) {
            ret = spi_dev_transfer(spi_dev,
                                   MR_NULL,
                                   (uint8_t *)&dev->position,
                                   (spi_dev->config.reg_bits >> 3),
                                   MR_SPI_WR);
            if (ret < 0) {
                spi_dev_cs_set(spi_dev, MR_DISABLE);
                spi_dev_release_bus(spi_dev);
                return ret;
            }
        }

        ret = spi_dev_transfer(spi_dev, buf, MR_NULL, count, MR_SPI_RD);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else {
        ret = (ssize_t)mr_ringbuf_read(&spi_dev->rd_fifo, buf, count);
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static ssize_t mr_spi_dev_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    ssize_t ret = spi_dev_take_bus(spi_dev);
    if (ret < 0) {
        return ret;
    }

    if (spi_dev->config.host_slave == MR_SPI_HOST) {
        spi_dev_cs_set(spi_dev, MR_ENABLE);

        /* Send the address of the register that needs to be written */
        if (dev->position >= 0) {
            ret = spi_dev_transfer(spi_dev,
                                   MR_NULL,
                                   (uint8_t *)&dev->position,
                                   (spi_dev->config.reg_bits >> 3),
                                   MR_SPI_WR);
            if (ret < 0) {
                spi_dev_cs_set(spi_dev, MR_DISABLE);
                spi_dev_release_bus(spi_dev);
                return ret;
            }
        }

        ret = spi_dev_transfer(spi_dev, MR_NULL, buf, count, MR_SPI_WR);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else {
        ret = spi_dev_transfer(spi_dev, MR_NULL, buf, count, MR_SPI_WR);
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static int mr_spi_dev_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    switch (cmd) {
        case MR_IOC_SPI_SET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev->parent;
                struct mr_spi_config config = *(struct mr_spi_config *)args;

#ifdef MR_USING_PIN
                /* Reconfigure CS */
                if (config.host_slave != spi_dev->config.host_slave) {
                    spi_dev->config = config;
                    spi_dev_cs_configure(spi_dev, MR_ENABLE);
                }
#endif /* MR_USING_PIN */

                /* If holding the bus, release it */
                if (spi_dev == spi_bus->owner) {
                    spi_bus->hold = MR_FALSE;
                    spi_bus->owner = MR_NULL;
                }

                /* Update the configuration and try again to get the bus */
                spi_dev->config = config;
                if (config.host_slave == MR_SPI_SLAVE) {
                    int ret = spi_dev_take_bus(spi_dev);
                    if (ret < 0) {
                        return ret;
                    }
                }
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SPI_SET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&spi_dev->rd_fifo, bufsz);
                spi_dev->rd_bufsz = 0;
                if (ret < 0) {
                    return ret;
                }
                spi_dev->rd_bufsz = bufsz;
                return sizeof(bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SPI_CLR_RD_BUF: {
            mr_ringbuf_reset(&spi_dev->rd_fifo);
            return MR_EOK;
        }
        case MR_IOC_SPI_TRANSFER: {
            if (args != MR_NULL) {
                struct mr_spi_transfer transfer = *(struct mr_spi_transfer *)args;

                int ret = spi_dev_take_bus(spi_dev);
                if (ret < 0) {
                    return ret;
                }

                if (spi_dev->config.host_slave == MR_SPI_HOST) {
                    spi_dev_cs_set(spi_dev, MR_ENABLE);
                    ret = (int)spi_dev_transfer(spi_dev,
                                                transfer.rd_buf,
                                                transfer.wr_buf,
                                                transfer.size,
                                                MR_SPI_RDWR);
                    spi_dev_cs_set(spi_dev, MR_DISABLE);
                } else {
                    ret = (int)spi_dev_transfer(spi_dev,
                                                transfer.rd_buf,
                                                transfer.wr_buf,
                                                transfer.size,
                                                MR_SPI_RDWR);
                }

                spi_dev_release_bus(spi_dev);
                return ret;
            }
            return MR_EINVAL;
        }
        case MR_IOC_SPI_GET_CONFIG: {
            if (args != MR_NULL) {
                struct mr_spi_config *config = (struct mr_spi_config *)args;

                *config = spi_dev->config;
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SPI_GET_RD_BUFSZ: {
            if (args != MR_NULL) {
                size_t *bufsz = (size_t *)args;

                *bufsz = spi_dev->rd_bufsz;
                return sizeof(*bufsz);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SPI_GET_RD_DATASZ: {
            if (args != MR_NULL) {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_data_size(&spi_dev->rd_fifo);
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
 * @brief This function registers a spi-device.
 *
 * @param spi_dev The spi-device.
 * @param path The path of the spi-device.
 * @param cs_pin The cs pin of the spi-device.
 * @param cs_active The cs active level of the spi-device.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *path, int cs_pin, int cs_active)
{
    static struct mr_dev_ops ops = {mr_spi_dev_open,
                                    mr_spi_dev_close,
                                    mr_spi_dev_read,
                                    mr_spi_dev_write,
                                    mr_spi_dev_ioctl,
                                    MR_NULL};
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    MR_ASSERT(spi_dev != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT((cs_active >= MR_SPI_CS_ACTIVE_LOW) && (cs_active <= MR_SPI_CS_ACTIVE_HARDWARE));

    /* Initialize the fields */
    spi_dev->config = default_config;
    mr_ringbuf_init(&spi_dev->rd_fifo, MR_NULL, 0);
#ifndef MR_CFG_SPI_RD_BUFSZ
#define MR_CFG_SPI_RD_BUFSZ             (0)
#endif /* MR_CFG_SPI_RD_BUFSZ */
    spi_dev->rd_bufsz = MR_CFG_SPI_RD_BUFSZ;
    spi_dev->cs_pin = (cs_active != MR_SPI_CS_ACTIVE_HARDWARE) ? cs_pin : -1;
    spi_dev->cs_active = cs_active;

    /* Register the spi-device */
    return mr_dev_register(&spi_dev->dev, path, MR_DEV_TYPE_SPI, MR_O_RDWR, &ops, MR_NULL);
}

#endif /* MR_USING_SPI */
