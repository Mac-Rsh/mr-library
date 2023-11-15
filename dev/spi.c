/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#include "spi.h"

#ifdef MR_USING_SPI

#ifdef MR_USING_GPIO
#include "gpio.h"
#else
#warning "Please define MR_USING_GPIO. Otherwise SPI-CS will not work."
#endif /* MR_USING_GPIO */

#define MR_SPI_RD                       (0)
#define MR_SPI_WR                       (1)
#define MR_SPI_RDWR                     (2)

static ssize_t spi_bus_transfer(struct mr_spi_bus *spi_bus,
                                void *rd_buf,
                                const void *wr_buf,
                                size_t size,
                                int rdwr)
{
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;
    size_t tf_size = 0;

    if (rdwr == MR_SPI_RD)
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                uint8_t *rd_data = (uint8_t *)rd_buf;
                mr_bits_clr(size, sizeof(*rd_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*rd_data))
                {
                    ops->write(spi_bus, 0);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_16:
            {
                uint16_t *rd_data = (uint16_t *)rd_buf;
                mr_bits_clr(size, sizeof(*rd_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*rd_data))
                {
                    ops->write(spi_bus, 0);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_32:
            {
                uint32_t *rd_data = (uint32_t *)rd_buf;
                mr_bits_clr(size, sizeof(*rd_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*rd_data))
                {
                    ops->write(spi_bus, 0);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                }
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }
    } else if (rdwr == MR_SPI_WR)
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                uint8_t *wr_data = (uint8_t *)wr_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    ops->read(spi_bus);
                    wr_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_16:
            {
                uint16_t *wr_data = (uint16_t *)wr_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    ops->read(spi_bus);
                    wr_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_32:
            {
                uint32_t *wr_data = (uint32_t *)wr_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    ops->read(spi_bus);
                    wr_data++;
                }
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }
    } else
    {
        switch (spi_bus->config.data_bits)
        {
            case MR_SPI_DATA_BITS_8:
            {
                uint8_t *rd_data = (uint8_t *)rd_buf;
                uint8_t *wr_data = (uint8_t *)wr_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                    wr_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_16:
            {
                uint16_t *wr_data = (uint16_t *)wr_buf;
                uint16_t *rd_data = (uint16_t *)rd_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                    wr_data++;
                }
                break;
            }
            case MR_SPI_DATA_BITS_32:
            {
                uint32_t *wr_data = (uint32_t *)wr_buf;
                uint32_t *rd_data = (uint32_t *)rd_buf;
                mr_bits_clr(size, sizeof(*wr_data) - 1);
                for (tf_size = 0; tf_size < size; tf_size += sizeof(*wr_data))
                {
                    ops->write(spi_bus, *wr_data);
                    *rd_data = ops->read(spi_bus);
                    rd_data++;
                    wr_data++;
                }
                break;
            }

            default:
            {
                return MR_EINVAL;
            }
        }
    }
    return (ssize_t)tf_size;
}

static int mr_spi_bus_open(struct mr_dev *dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;

    return ops->configure(spi_bus, &spi_bus->config);
}

static int mr_spi_bus_close(struct mr_dev *dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;
    struct mr_spi_config close_config = {0};

    return ops->configure(spi_bus, &close_config);
}

static ssize_t mr_spi_bus_read(struct mr_dev *dev, int off, void *buf, size_t size, int sync_or_async)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;

    if (off >= 0)
    {
        spi_bus_transfer(spi_bus, MR_NULL, buf, (spi_bus->config.off_bits >> 3), MR_SPI_WR);
    }
    return spi_bus_transfer(spi_bus, buf, MR_NULL, size, MR_SPI_RD);
}

static ssize_t mr_spi_bus_write(struct mr_dev *dev, int off, const void *buf, size_t size, int sync_or_async)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;

    if (off >= 0)
    {
        spi_bus_transfer(spi_bus, MR_NULL, buf, (spi_bus->config.off_bits >> 3), MR_SPI_WR);
    }
    return spi_bus_transfer(spi_bus, MR_NULL, buf, size, MR_SPI_WR);
}

static int mr_spi_bus_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    switch (cmd)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_config config = *(struct mr_spi_config *)args;

                /* The bus is held by another device */
                if (spi_bus->owner != MR_NULL)
                {
                    return MR_EBUSY;
                }

                /* The bus requires a device to be mounted to be configured in slave mode */
                if (config.host_slave == MR_SPI_SLAVE)
                {
                    return MR_EINVAL;
                }

                int ret = ops->configure(spi_bus, &config);
                if (ret == MR_EOK)
                {
                    spi_bus->config = config;
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_config *config = (struct mr_spi_config *)args;

                *config = spi_bus->config;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_SPI_TRANSFER:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_transfer transfer = *(struct mr_spi_transfer *)args;

                int ret =
                    (int)spi_bus_transfer(spi_bus, transfer.rd_buf, transfer.wr_buf, transfer.size, MR_SPI_RDWR);

                return ret;
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_spi_bus_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    switch (event)
    {
        case MR_ISR_EVENT_RD_INTER:
        {
            struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)spi_bus->owner;

            uint32_t data = ops->read(spi_bus);
#ifdef MR_USING_GPIO
            if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
            {
                uint8_t level = !spi_dev->cs_active;
                mr_dev_read(spi_dev->cs_desc, &level, sizeof(level));
                if (level != spi_dev->cs_active)
                {
                    return MR_ENOTSUP;
                }
            }
#endif /* MR_USING_GPIO */
            mr_ringbuf_write_force(&spi_dev->rd_fifo, &data, (spi_bus->config.data_bits >> 3));
            if (spi_dev->dev.rd_cb.cb != MR_NULL)
            {
                size_t size = (ssize_t)mr_ringbuf_get_data_size(&spi_dev->rd_fifo);
                spi_dev->dev.rd_cb.cb(spi_dev->dev.rd_cb.desc, &size);
            }
            return (ssize_t)mr_ringbuf_get_data_size(&spi_dev->rd_fifo);
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a spi-bus.
 *
 * @param spi_bus The spi-bus.
 * @param name The name of the spi-bus.
 * @param drv The driver of the spi-bus.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            mr_spi_bus_open,
            mr_spi_bus_close,
            mr_spi_bus_read,
            mr_spi_bus_write,
            mr_spi_bus_ioctl,
            mr_spi_bus_isr
        };
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    mr_assert(spi_bus != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Initialize the fields */
    spi_bus->config = default_config;
    spi_bus->owner = MR_NULL;

    /* Register the spi-bus */
    return mr_dev_register(&spi_bus->dev, name, Mr_Dev_Type_Spi, MR_SFLAG_RDWR, &ops, drv);
}

#ifdef MR_USING_GPIO
static void spi_dev_cs_configure(struct mr_spi_dev *spi_dev, int state)
{
    int desc = spi_dev->cs_desc;
    if (desc < 0)
    {
        return;
    }

    if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        mr_dev_ioctl(desc, MR_CTRL_SET_OFFSET, mr_make_local(int, spi_dev->cs_pin));

        if (state == MR_ENABLE)
        {
            int mode = MR_GPIO_MODE_NONE;

            if (spi_dev->config.host_slave == MR_SPI_HOST)
            {
                mode = MR_GPIO_MODE_OUTPUT;
            } else
            {
                if (spi_dev->cs_active == MR_SPI_CS_ACTIVE_LOW)
                {
                    mode = MR_GPIO_MODE_INPUT_UP;
                } else
                {
                    mode = MR_GPIO_MODE_INPUT_DOWN;
                }
            }
            mr_dev_ioctl(desc, MR_CTRL_GPIO_SET_PIN_MODE, &mode);
            mr_dev_write(desc, mr_make_local(uint8_t, !spi_dev->cs_active), sizeof(uint8_t));
        } else
        {
            mr_dev_ioctl(desc, MR_CTRL_GPIO_SET_PIN_MODE, mr_make_local(int, MR_GPIO_MODE_NONE));
        }
    }
}
#endif /* MR_USING_GPIO */

MR_INLINE int spi_dev_take_bus(struct mr_spi_dev *spi_dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    if ((spi_dev != spi_bus->owner) && (spi_bus->owner != MR_NULL))
    {
        return MR_EBUSY;
    }

    if (spi_dev != spi_bus->owner)
    {
        /* Reconfigure the bus */
        if (spi_dev->config.baud_rate != spi_bus->config.baud_rate
            || spi_dev->config.host_slave != spi_bus->config.host_slave
            || spi_dev->config.mode != spi_bus->config.mode
            || spi_dev->config.data_bits != spi_bus->config.data_bits
            || spi_dev->config.bit_order != spi_bus->config.bit_order)
        {
            int ret = ops->configure(spi_bus, &spi_dev->config);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }
        spi_bus->config = spi_dev->config;
        spi_bus->owner = spi_dev;
    }
    return MR_EOK;
}

MR_INLINE int spi_dev_release_bus(struct mr_spi_dev *spi_dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;

    if (spi_dev != spi_bus->owner)
    {
        return MR_EINVAL;
    }

    /* If it is a host, release the bus. The slave needs to hold the bus at all times */
    if (spi_dev->config.host_slave == MR_SPI_HOST)
    {
        spi_bus->owner = MR_NULL;
    }
    return MR_EOK;
}

MR_INLINE void spi_dev_cs_set(struct mr_spi_dev *spi_dev, int state)
{
#ifdef MR_USING_GPIO
    if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        mr_dev_write(spi_dev->cs_desc, mr_make_local(uint8_t, !(state ^ spi_dev->cs_active)), sizeof(uint8_t));
    }
#endif /* MR_USING_GPIO */
}

static int mr_spi_dev_open(struct mr_dev *dev)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

#ifdef MR_USING_GPIO
    if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        spi_dev->cs_desc = mr_dev_open("gpio", MR_OFLAG_RDWR);
        spi_dev_cs_configure(spi_dev, MR_ENABLE);
    }
#endif /* MR_USING_GPIO */

    /* Allocate FIFO buffers */
    return mr_ringbuf_allocate(&spi_dev->rd_fifo, spi_dev->rd_bufsz);
}

static int mr_spi_dev_close(struct mr_dev *dev)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

#ifdef MR_USING_GPIO
    if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        spi_dev_cs_configure(spi_dev, MR_DISABLE);
        mr_dev_close(spi_dev->cs_desc);
        spi_dev->cs_desc = -1;
    }
#endif /* MR_USING_GPIO */

    /* Free FIFO buffers */
    mr_ringbuf_free(&spi_dev->rd_fifo);
    return MR_EOK;
}

static ssize_t mr_spi_dev_read(struct mr_dev *dev, int off, void *buf, size_t size, int sync_or_async)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    ssize_t ret = spi_dev_take_bus(spi_dev);
    if (ret != MR_EOK)
    {
        return ret;
    }

    if (spi_dev->config.host_slave == MR_SPI_HOST)
    {
        spi_dev_cs_set(spi_dev, MR_ENABLE);
        ret = mr_spi_bus_read(dev->link, off, buf, size, sync_or_async);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else
    {
        if (mr_ringbuf_get_bufsz(&spi_dev->rd_fifo) == 0)
        {
            ret = mr_spi_bus_read(dev->link, -1, buf, size, sync_or_async);
        } else
        {
            ret = (ssize_t)mr_ringbuf_read(&spi_dev->rd_fifo, buf, size);
        }
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static ssize_t mr_spi_dev_write(struct mr_dev *dev, int off, const void *buf, size_t size, int sync_or_async)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    ssize_t ret = spi_dev_take_bus(spi_dev);
    if (ret != MR_EOK)
    {
        return ret;
    }

    if (spi_dev->config.host_slave == MR_SPI_HOST)
    {
        spi_dev_cs_set(spi_dev, MR_ENABLE);
        ret = mr_spi_bus_write(dev->link, off, buf, size, sync_or_async);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else
    {
        ret = mr_spi_bus_write(dev->link, -1, buf, size, sync_or_async);
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static int mr_spi_dev_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    switch (cmd)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev->link;
                struct mr_spi_config config = *(struct mr_spi_config *)args;

#ifdef MR_USING_GPIO
                /* Reconfigure CS */
                if (config.host_slave != spi_dev->config.host_slave)
                {
                    spi_dev_cs_configure(spi_dev, MR_ENABLE);
                }
#endif /* MR_USING_GPIO */
                /* Release the bus */
                if (spi_dev == spi_bus->owner)
                {
                    spi_bus->owner = MR_NULL;
                }

                spi_dev->config = config;
                if (config.host_slave == MR_SPI_SLAVE)
                {
                    /* Retry to take the bus */
                    int ret = spi_dev_take_bus(spi_dev);
                    if (ret != MR_EOK)
                    {
                        return ret;
                    }
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_SET_RD_BUFSZ:
        {
            if (args != MR_NULL)
            {
                size_t bufsz = *(size_t *)args;

                int ret = mr_ringbuf_allocate(&spi_dev->rd_fifo, bufsz);
                spi_dev->rd_bufsz = 0;
                if (ret == MR_EOK)
                {
                    spi_dev->rd_bufsz = bufsz;
                }
                return ret;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_config *config = (struct mr_spi_config *)args;

                *config = spi_dev->config;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTRL_GET_RD_BUFSZ:
        {
            if (args != MR_NULL)
            {
                *(size_t *)args = spi_dev->rd_bufsz;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_SPI_TRANSFER:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_transfer transfer = *(struct mr_spi_transfer *)args;

                int ret = spi_dev_take_bus(spi_dev);
                if (ret != MR_EOK)
                {
                    return ret;
                }

                if (spi_dev->config.host_slave == MR_SPI_HOST)
                {
                    spi_dev_cs_set(spi_dev, MR_ENABLE);
                    ret = (int)spi_bus_transfer(dev->link,
                                                transfer.rd_buf,
                                                transfer.wr_buf,
                                                transfer.size,
                                                MR_SPI_RDWR);
                    spi_dev_cs_set(spi_dev, MR_DISABLE);
                } else
                {
                    ret = (int)spi_bus_transfer(dev->link,
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

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a spi-device.
 *
 * @param spi_dev The spi-device.
 * @param name The name of the spi-device.
 * @param cs_pin The cs pin of the spi-device.
 * @param cs_active The cs active level of the spi-device.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *name, int cs_pin, int cs_active)
{
    static struct mr_dev_ops ops =
        {
            mr_spi_dev_open,
            mr_spi_dev_close,
            mr_spi_dev_read,
            mr_spi_dev_write,
            mr_spi_dev_ioctl
        };
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    mr_assert(spi_dev != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert((cs_active >= MR_SPI_CS_ACTIVE_LOW) && (cs_active <= MR_SPI_CS_ACTIVE_NONE));

    /* Initialize the fields */
    spi_dev->config = default_config;
    mr_ringbuf_init(&spi_dev->rd_fifo, MR_NULL, 0);
#ifndef MR_CFG_SPI_RD_BUFSZ_INIT
#define MR_CFG_SPI_RD_BUFSZ_INIT        (0)
#endif /* MR_CFG_SPI_RD_BUFSZ_INIT */
    spi_dev->rd_bufsz = MR_CFG_SPI_RD_BUFSZ_INIT;
    spi_dev->cs_pin = cs_pin;
    spi_dev->cs_active = (cs_pin >= 0) ? cs_active : MR_SPI_CS_ACTIVE_NONE;
    spi_dev->cs_desc = -1;

    /* Register the spi-device */
    return mr_dev_register(&spi_dev->dev, name, Mr_Dev_Type_Spi, MR_SFLAG_RDWR | MR_SFLAG_NONDRV, &ops, MR_NULL);
}

#endif /* MR_USING_SPI */
