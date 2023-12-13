/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#include "include/device/spi.h"

#ifdef MR_USING_SPI

#ifdef MR_USING_PIN
#include "include/device/pin.h"
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
    spi_bus->cs_desc = mr_dev_open("pin", MR_OFLAG_RDWR);
#endif /* MR_USING_PIN */

    return ops->configure(spi_bus, &spi_bus->config);
}

static int mr_spi_bus_close(struct mr_dev *dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)dev->drv->ops;
    struct mr_spi_config close_config = {0};

#ifdef MR_USING_PIN
    if (spi_bus->cs_desc >= 0)
    {
        mr_dev_close(spi_bus->cs_desc);
        spi_bus->cs_desc = -1;
    }
#endif /* MR_USING_PIN */

    return ops->configure(spi_bus, &close_config);
}

static ssize_t mr_spi_bus_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    return MR_EIO;
}

static ssize_t mr_spi_bus_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    return MR_EIO;
}

static ssize_t mr_spi_bus_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    switch (event)
    {
        case MR_ISR_SPI_RD_INT:
        {
            struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)spi_bus->owner;
            uint32_t data = ops->read(spi_bus);

#ifdef MR_USING_PIN
            /* Check if CS is active */
            if ((spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE) && (spi_bus->cs_desc >= 0))
            {
                uint8_t level = !spi_dev->cs_active;
                mr_dev_read(spi_bus->cs_desc, &level, sizeof(level));
                if (level != spi_dev->cs_active)
                {
                    return MR_EINVAL;
                }
            }
#endif /* MR_USING_PIN */

            /* Read data to FIFO. if callback is set, call it */
            mr_ringbuf_write_force(&spi_dev->rd_fifo, &data, (spi_bus->config.data_bits >> 3));
            if (spi_dev->dev.rd_call.call != MR_NULL)
            {
                ssize_t size = (ssize_t)mr_ringbuf_get_data_size(&spi_dev->rd_fifo);
                spi_dev->dev.rd_call.call(spi_dev->dev.rd_call.desc, &size);
            }
            return MR_EOK;
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
            MR_NULL,
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
    spi_bus->hold = MR_FALSE;
    spi_bus->cs_desc = -1;

    /* Register the spi-bus */
    return mr_dev_register(&spi_bus->dev, name, Mr_Dev_Type_SPI, MR_SFLAG_RDWR, &ops, drv);
}

#ifdef MR_USING_PIN
static void spi_dev_cs_configure(struct mr_spi_dev *spi_dev, int state)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;
    int desc = spi_bus->cs_desc;

    /* Check the descriptor is valid */
    if (desc < 0)
    {
        return;
    }

    if (spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE)
    {
        int old_number = -1;

        /* Temporarily store the old number */
        mr_dev_ioctl(desc, MR_CTL_PIN_GET_NUMBER, &old_number);

        /* Set the new number */
        mr_dev_ioctl(desc, MR_CTL_PIN_SET_NUMBER, mr_make_local(int, spi_dev->cs_pin));
        if (state == MR_ENABLE)
        {
            int mode = MR_PIN_MODE_NONE;

            if (spi_dev->config.host_slave == MR_SPI_HOST)
            {
                mode = MR_PIN_MODE_OUTPUT;
            } else
            {
                if (spi_dev->cs_active == MR_SPI_CS_ACTIVE_LOW)
                {
                    mode = MR_PIN_MODE_INPUT_UP;
                } else
                {
                    mode = MR_PIN_MODE_INPUT_DOWN;
                }
            }
            mr_dev_ioctl(desc, MR_CTL_PIN_SET_MODE, &mode);
            mr_dev_write(desc, mr_make_local(uint8_t, !spi_dev->cs_active), sizeof(uint8_t));
        } else
        {
            mr_dev_ioctl(desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_NONE));
        }

        /* Restore the old number */
        mr_dev_ioctl(desc, MR_CTL_PIN_SET_NUMBER, &old_number);
    }
}
#endif /* MR_USING_PIN */

MR_INLINE void spi_dev_cs_set(struct mr_spi_dev *spi_dev, int state)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;

#ifdef MR_USING_PIN
    if ((spi_dev->cs_active != MR_SPI_CS_ACTIVE_NONE) && (spi_bus->cs_desc >= 0))
    {
        mr_dev_write(spi_bus->cs_desc, mr_make_local(uint8_t, !(state ^ spi_dev->cs_active)), sizeof(uint8_t));
    }
#endif /* MR_USING_PIN */
}

MR_INLINE int spi_dev_take_bus(struct mr_spi_dev *spi_dev)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;

    /* Check if the bus is busy */
    if ((spi_bus->hold == MR_TRUE) && (spi_dev != spi_bus->owner))
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
#ifdef MR_USING_PIN
        if (spi_bus->cs_desc >= 0)
        {
            mr_dev_ioctl(spi_bus->cs_desc, MR_CTL_PIN_SET_NUMBER, mr_make_local(int, spi_dev->cs_pin));
        }
#endif /* MR_USING_PIN */
    }
    spi_bus->hold = MR_TRUE;
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
        spi_bus->hold = MR_FALSE;
    }
    return MR_EOK;
}

#define MR_SPI_RD                       (0)
#define MR_SPI_WR                       (1)
#define MR_SPI_RDWR                     (2)

static ssize_t spi_dev_transfer(struct mr_spi_dev *spi_dev, void *rd_buf, const void *wr_buf, size_t size, int rdwr)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)spi_dev->dev.link;
    struct mr_spi_bus_ops *ops = (struct mr_spi_bus_ops *)spi_bus->dev.drv->ops;
    size_t tf_size = 0;

    if (rdwr == MR_SPI_RD)
    {
        switch (spi_dev->config.data_bits)
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
        switch (spi_dev->config.data_bits)
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
        switch (spi_dev->config.data_bits)
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

static ssize_t mr_spi_dev_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
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
        if (off >= 0)
        {
            /* Send the address of the register that needs to be read */
            spi_dev_transfer(spi_dev, MR_NULL, &off, (spi_dev->config.reg_bits >> 3), MR_SPI_WR);
        }

        ret = spi_dev_transfer(spi_dev, buf, MR_NULL, size, MR_SPI_RD);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else
    {
        if (mr_ringbuf_get_bufsz(&spi_dev->rd_fifo) == 0)
        {
            ret = spi_dev_transfer(spi_dev, buf, MR_NULL, size, MR_SPI_RD);
        } else
        {
            ret = (ssize_t)mr_ringbuf_read(&spi_dev->rd_fifo, buf, size);
        }
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static ssize_t mr_spi_dev_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
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
        if (off >= 0)
        {
            /* Send the address of the register that needs to be written */
            spi_dev_transfer(spi_dev, MR_NULL, &off, (spi_dev->config.reg_bits >> 3), MR_SPI_WR);
        }

        ret = spi_dev_transfer(spi_dev, MR_NULL, buf, size, MR_SPI_WR);
        spi_dev_cs_set(spi_dev, MR_DISABLE);
    } else
    {
        ret = spi_dev_transfer(spi_dev, MR_NULL, buf, size, MR_SPI_WR);
    }

    spi_dev_release_bus(spi_dev);
    return ret;
}

static int mr_spi_dev_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_spi_dev *spi_dev = (struct mr_spi_dev *)dev;

    switch (cmd)
    {
        case MR_CTL_SPI_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)dev->link;
                struct mr_spi_config config = *(struct mr_spi_config *)args;

#ifdef MR_USING_PIN
                /* Reconfigure CS */
                if (config.host_slave != spi_dev->config.host_slave)
                {
                    spi_dev->config = config;
                    spi_dev_cs_configure(spi_dev, MR_ENABLE);
                }
#endif /* MR_USING_PIN */

                /* If holding the bus, release it */
                if (spi_dev == spi_bus->owner)
                {
                    spi_bus->hold = MR_FALSE;
                    spi_bus->owner = MR_NULL;
                }

                /* Update the configuration and try again to get the bus */
                spi_dev->config = config;
                if (config.host_slave == MR_SPI_SLAVE)
                {
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
        case MR_CTL_SPI_SET_RD_BUFSZ:
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
        case MR_CTL_SPI_CLR_RD_BUF:
        {
            mr_ringbuf_reset(&spi_dev->rd_fifo);
            return MR_EOK;
        }
        case MR_CTL_SPI_TRANSFER:
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
                    ret = (int)spi_dev_transfer(dev->link,
                                                transfer.rd_buf,
                                                transfer.wr_buf,
                                                transfer.size,
                                                MR_SPI_RDWR);
                    spi_dev_cs_set(spi_dev, MR_DISABLE);
                } else
                {
                    ret = (int)spi_dev_transfer(dev->link,
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

        case MR_CTL_SPI_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_spi_config *config = (struct mr_spi_config *)args;

                *config = spi_dev->config;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_SPI_GET_RD_BUFSZ:
        {
            if (args != MR_NULL)
            {
                *(size_t *)args = spi_dev->rd_bufsz;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_SPI_GET_RD_DATASZ:
        {
            if (args != MR_NULL)
            {
                size_t *datasz = (size_t *)args;

                *datasz = mr_ringbuf_get_data_size(&spi_dev->rd_fifo);
                return MR_EOK;
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
            mr_spi_dev_ioctl,
            MR_NULL
        };
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    mr_assert(spi_dev != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert((cs_active >= MR_SPI_CS_ACTIVE_LOW) && (cs_active <= MR_SPI_CS_ACTIVE_NONE));

    /* Initialize the fields */
    spi_dev->config = default_config;
    mr_ringbuf_init(&spi_dev->rd_fifo, MR_NULL, 0);
#ifndef MR_CFG_SPI_RD_BUFSZ
#define MR_CFG_SPI_RD_BUFSZ             (0)
#endif /* MR_CFG_SPI_RD_BUFSZ */
    spi_dev->rd_bufsz = MR_CFG_SPI_RD_BUFSZ;
    spi_dev->cs_pin = cs_pin;
    spi_dev->cs_active = (cs_pin >= 0) ? cs_active : MR_SPI_CS_ACTIVE_NONE;

    /* Register the spi-device */
    return mr_dev_register(&spi_dev->dev, name, Mr_Dev_Type_SPI, MR_SFLAG_RDWR | MR_SFLAG_NONDRV, &ops, MR_NULL);
}

#endif /* MR_USING_SPI */
