/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-01    MacRsh       First version
 */

#include "../mr-library/device/include/mr_spi.h"

#ifdef MR_USE_SPI

#ifdef MR_USE_PIN
#include "../mr-library/device/include/mr_pin.h"
#endif /* MR_USE_PIN */

static int spi_isr(struct mr_device *device, uint32_t event, void *args)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)device;
    struct mr_spi_device *spi_device = (struct mr_spi_device *)spi_bus->owner;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    switch (event)
    {
        case MR_EVENT_SPI_RD_COMPLETE_INT:
        {
            int count = 1;

            if (spi_device == NULL)
            {
                return MR_EINVAL;
            }

            if (args != NULL)
            {
                /* Hardware FIFO is considered to be used */
                count = *((int *)args);
            }

            /* If FIFO is empty, the read operation is abandoned */
            if (mr_fifo_size_get(&spi_device->rfifo) == 0)
            {
                return MR_EOK;
            }

            /* Read all data from hardware FIFO */
            uint32_t data_sizeof = spi_device->config.data_bits / 8;
            for (size_t rcount = 0; rcount < count; rcount++)
            {
                uint32_t data;

                /* Read data from serial */
                int ret = ops->receive(driver, &data);
                if (ret < 0)
                {
                    return ret;
                }

                /* Force write data to FIFO */
                mr_fifo_write_force(&spi_device->rfifo, &data, data_sizeof);
            }
            return MR_EOK;
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

static int spi_bus_attach(struct mr_device *device, struct mr_device *source)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Only allow attaching if the source device is an SPI device */
    if (source->type != MR_DEVICE_TYPE_SPI)
    {
        return MR_EPERM;
    }

    /* In the first attaching, if the driver does not support chip selection
     * configuration or level setting, try using a pin device */
    if ((mr_list_is_empty(&device->clist) == true) &&
        ((ops->cs_configure == NULL) || (ops->cs_set == NULL)))
    {
#ifdef MR_USE_PIN
#ifndef MR_CFG_SPI_PIN_NAME
#define MR_CFG_SPI_PIN_NAME ("pin")
#endif /* MR_CFG_SPI_PIN_NAME */
        spi_bus->pin_descriptor =
            mr_device_open(MR_CFG_SPI_PIN_NAME, MR_FLAG_RDWR);
        if (spi_bus->pin_descriptor < 0)
        {
            return spi_bus->pin_descriptor;
        }
#else
        return MR_EIO;
#endif /* MR_USE_PIN */
    }

    /* Allow attaching */
    return MR_EOK;
}

static int spi_bus_detach(struct mr_device *device, struct mr_device *source)
{
    struct mr_spi_bus *spi_bus = (struct mr_spi_bus *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    struct mr_spi_config null_config = {0};

    /* In the last detaching, try to release the resource */
    if (mr_list_is_empty(&device->clist) == true)
    {
#ifdef MR_USE_PIN
        /* Close the pin device if it is open */
        if (spi_bus->pin_descriptor >= 0)
        {
            int ret = mr_device_close(spi_bus->pin_descriptor);
            if (ret < 0)
            {
                return ret;
            }

            /* Reset the pin descriptor */
            spi_bus->pin_descriptor = -1;
        }
#endif /* MR_USE_PIN */

        /* Reset the SPI bus */
        _MR_DEVICE_OPERATOR_CLR((struct mr_device *)spi_bus);
        spi_bus->config = null_config;
        spi_bus->owner = NULL;

        /* Configure the driver as disabled */
        return ops->configure(driver, false, &spi_bus->config);
    }

    /* Allow detaching */
    return MR_EOK;
}

/**
 * @brief This function register a SPI bus.
 *
 * @param spi_bus The SPI bus.
 * @param path The path of the SPI bus.
 * @param driver The SPI bus driver.
 *
 * @return The error code.
 */
int mr_spi_bus_register(struct mr_spi_bus *spi_bus, const char *path,
                        struct mr_driver *driver)
{
    MR_ASSERT(spi_bus != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL));

    static struct mr_device_ops ops = {.isr = spi_isr,
                                       .attach = spi_bus_attach,
                                       .detach = spi_bus_detach};
    struct mr_spi_config null_config = {0};

    /* Initialize the SPI bus */
    spi_bus->config = null_config;
    spi_bus->owner = NULL;
#ifdef MR_USE_PIN
    spi_bus->pin_descriptor = -1;
#endif /* MR_USE_PIN */

    /* Register the SPI bus */
    return mr_device_register((struct mr_device *)spi_bus, path,
                              MR_DEVICE_TYPE_SPI, &ops, driver);
}

static int _spi_device_cs_configure(struct mr_spi_device *spi_device,
                                    bool enable)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint8_t level = !spi_device->cs_active;
    uint32_t mode;
    int ret;

    /* Check whether it is necessary to configure the chip select pin */
    if (spi_device->cs_active == MR_SPI_CS_ACTIVE_NONE)
    {
        return MR_EOK;
    }

    if (enable == true)
    {
        /* Configure the chip to select the pin mode according to the role */
        if (spi_device->config.role == MR_SPI_ROLE_MASTER)
        {
            mode = MR_SPI_CS_MODE_OUTPUT;
        } else if (spi_device->cs_active == MR_SPI_CS_ACTIVE_LOW)
        {
            mode = MR_SPI_CS_MODE_INPUT_UP;
        } else
        {
            mode = MR_SPI_CS_MODE_INPUT_DOWN;
        }
    } else
    {
        mode = MR_SPI_CS_MODE_NONE;
    }

    /* Configure the chip select pin */
    if (ops->cs_configure != NULL)
    {
        /* Configure mode */
        ret = ops->cs_configure(driver, spi_device->cs_pin, mode);
        if (ret < 0)
        {
            return ret;
        }

        /* If the mode is none, no need to set the level */
        if (mode == MR_SPI_CS_MODE_NONE)
        {
            return MR_EOK;
        }

        /* Set the inactive level */
        return ops->cs_set(driver, spi_device->cs_pin, level);
    }
#ifdef MR_USE_PIN
    else if (spi_bus->pin_descriptor >= 0)
    {
        /* Configure mode */
        mr_device_ioctl(spi_bus->pin_descriptor, MR_CTRL_SET(MR_CMD_POS),
                        &spi_device->cs_pin);
        ret = mr_device_ioctl(spi_bus->pin_descriptor,
                              MR_CTRL_SET(MR_CMD_CONFIG), &mode);
        if (ret < 0)
        {
            return ret;
        }

        /* If the mode is none, no need to set the level */
        if (mode == MR_SPI_CS_MODE_NONE)
        {
            return MR_EOK;
        }

        /* Set the inactive level */
        return (int)mr_device_write(spi_bus->pin_descriptor, &level,
                                    sizeof(level));
    }
#endif /* MR_USE_PIN */
    return MR_EIO;
}

static int _spi_device_cs_set(struct mr_spi_device *spi_device, bool enable)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint8_t level = spi_device->cs_active;
    int ret;

    /* Check whether it is necessary to configure the chip select pin */
    if (spi_device->cs_active == MR_SPI_CS_ACTIVE_NONE)
    {
        return MR_EOK;
    }

    if (enable == true)
    {
        /* Delay before setting the chip select pin */
        mr_delay_us(spi_device->config.cs_delay);
    } else
    {
        /* Set the inactive level */
        level = !level;
    }

    /* Set the chip select pin */
    if (ops->cs_set != NULL)
    {
        ret = ops->cs_set(driver, spi_device->cs_pin, level);
        if (ret < 0)
        {
            return ret;
        }
    }
#ifdef MR_USE_PIN
    else
    {
        ret = (int)mr_device_write(spi_bus->pin_descriptor, &level,
                                   sizeof(level));
        if (ret < 0)
        {
            return ret;
        }
    }
#else
    else
    {
        return MR_EIO;
    }
#endif /* MR_USE_PIN */

    if (enable == false)
    {
        /* Delay after setting the chip select pin */
        mr_delay_us(spi_device->config.cs_delay);
    }
    return MR_EOK;
}

MR_INLINE int _spi_device_take_bus(struct mr_spi_device *spi_device)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    int spi_bus_operator, spi_device_operator;

    /* Critical section enter */
    mr_critical_enter();

    /* Check whether the SPI bus is already in use */
    spi_bus_operator = _MR_DEVICE_OPERATOR_GET((struct mr_device *)spi_bus);
    if ((spi_bus_operator != -1) && (spi_device != spi_bus->owner))
    {
        /* Critical section exit */
        mr_critical_exit();
        return MR_EBUSY;
    }

    /* Set the SPI device operator as the SPI bus operator */
    spi_device_operator =
        _MR_DEVICE_OPERATOR_GET((struct mr_device *)spi_device);
    _MR_DEVICE_OPERATOR_SET((struct mr_device *)spi_bus, spi_device_operator);

    /* Critical section exit */
    mr_critical_exit();

    /* Reconfigure the driver if the configuration is changed or an owner
     * changed */
    if (spi_device != spi_bus->owner)
    {
        if ((spi_device->config.baud_rate != spi_bus->config.baud_rate) ||
            (spi_device->config.role != spi_bus->config.role) ||
            (spi_device->config.mode != spi_bus->config.mode) ||
            (spi_device->config.bit_order != spi_bus->config.bit_order) ||
            (spi_device->config.data_bits != spi_bus->config.data_bits))
        {
            int ret = ops->configure(driver, true, &spi_device->config);
            if (ret < 0)
            {
                /* Clear the SPI bus operator */
                _MR_DEVICE_OPERATOR_CLR((struct mr_device *)spi_bus);
                return ret;
            }

            /* Update the spi-bus configuration */
            spi_bus->config = spi_device->config;
            spi_bus->owner = spi_device;
        }
    }

#ifdef MR_USE_PIN
    /* Switch to the SPI bus owner's chip select pin */
    if (spi_bus->pin_descriptor >= 0)
    {
        mr_device_ioctl(spi_bus->pin_descriptor, MR_CTRL_SET(MR_CMD_POS),
                        &spi_device->cs_pin);
    }
#endif /* MR_USE_PIN */
    return MR_EOK;
}

MR_INLINE int _spi_device_release_bus(struct mr_spi_device *spi_device)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);

    /* Release the SPI bus if its role is not slave */
    if (spi_bus->config.role != MR_SPI_ROLE_SLAVE)
    {
        _MR_DEVICE_OPERATOR_SET((struct mr_device *)spi_bus, -1);
    }
    return MR_EOK;
}

MR_INLINE ssize_t _spi_device_master_read(struct mr_spi_device *spi_device,
                                          int pos, uint8_t *buf, size_t count)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t data_sizeof = spi_device->config.data_bits / 8;
    ssize_t rcount;

    /* Send the address of the register that needs to be read */
    if (spi_device->config.reg_bits > 0)
    {
        for (size_t reg_count = 0;
             reg_count < (spi_device->config.reg_bits / 8);
             reg_count += data_sizeof)
        {
            uint32_t data;

            memcpy(&data, ((uint8_t *)&pos) + reg_count, data_sizeof);
            int ret = ops->send(driver, data);
            if (ret < 0)
            {
                /* Return the error code */
                return ret;
            }
        }
    }

    /* Receive data */
    for (rcount = 0; rcount < count; rcount += data_sizeof)
    {
        uint32_t data;

        int ret = ops->receive(driver, &data);
        if (ret < 0)
        {
            /* If no data received, return the error code */
            return (rcount == 0) ? ret : rcount;
        }

        memcpy(buf, &data, data_sizeof);
        buf += data_sizeof;
    }

    /* Return the number of bytes read */
    return rcount;
}

MR_INLINE ssize_t _spi_device_slave_read(struct mr_spi_device *spi_device,
                                         int pos, uint8_t *buf, size_t count)
{
    return (ssize_t)mr_fifo_read(&spi_device->rfifo, buf, count);
}

MR_INLINE ssize_t _spi_device_master_write(struct mr_spi_device *spi_device,
                                           int pos, const uint8_t *buf,
                                           size_t count)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t data_sizeof = spi_device->config.data_bits / 8;
    ssize_t wcount;

    /* Send the address of the register that needs to be written */
    if (spi_device->config.reg_bits > 0)
    {
        for (size_t reg_count = 0;
             reg_count < (spi_device->config.reg_bits / 8);
             reg_count += data_sizeof)
        {
            uint32_t data;

            memcpy(&data, ((uint8_t *)&pos) + reg_count, data_sizeof);
            int ret = ops->send(driver, data);
            if (ret < 0)
            {
                /* Return the error code */
                return ret;
            }
        }
    }

    /* Send data */
    for (wcount = 0; wcount < count; wcount += data_sizeof)
    {
        uint32_t data;

        memcpy(&data, buf, data_sizeof);
        int ret = ops->send(driver, data);
        if (ret < 0)
        {
            /* If no data sent, return the error code */
            return (wcount == 0) ? ret : wcount;
        }
        buf += data_sizeof;
    }

    /* Return the number of bytes sent */
    return wcount;
}

MR_INLINE ssize_t _spi_device_slave_write(struct mr_spi_device *spi_device,
                                          int pos, const uint8_t *buf,
                                          size_t count)
{
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);
    struct mr_driver *driver =
        _MR_DEVICE_DRIVER_GET((struct mr_device *)spi_bus);
    struct mr_spi_bus_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t data_sizeof = spi_device->config.data_bits / 8;
    ssize_t wcount;

    /* Send data */
    for (wcount = 0; wcount < count; wcount += data_sizeof)
    {
        uint32_t data;

        memcpy(&data, buf, data_sizeof);
        int ret = ops->send(driver, data);
        if (ret < 0)
        {
            /* If no data sent, return the error code */
            return (wcount == 0) ? ret : wcount;
        }
        buf += data_sizeof;
    }

    /* Return the number of bytes sent */
    return wcount;
}

MR_INLINE int _spi_device_fifo_update(struct mr_fifo *fifo, size_t *size)
{
    /* Allocate new buffer for FIFO */
    int ret = mr_fifo_allocate(fifo, *size);
    if (ret < 0)
    {
        /* Old buffer has been released */
        *size = 0;
        return ret;
    }
    return MR_EOK;
}

static int spi_device_open(struct mr_device *device)
{
    struct mr_spi_device *spi_device = (struct mr_spi_device *)device;
    int ret;

    /* Enable the spi-device chip select pin */
    ret = _spi_device_cs_configure(spi_device, true);
    if (ret < 0)
    {
        return ret;
    }

    /* Allocate FIFO */
    mr_fifo_allocate(&spi_device->rfifo, spi_device->rfifo_size);
    return MR_EOK;
}

static int spi_device_close(struct mr_device *device)
{
    struct mr_spi_device *spi_device = (struct mr_spi_device *)device;

    /* Disable the spi-device chip select pin */
    _spi_device_cs_configure(spi_device, false);

    /* Free FIFO */
    mr_fifo_free(&spi_device->rfifo);
    return MR_EOK;
}

static ssize_t spi_device_read(struct mr_device *device, int pos, void *buf,
                               size_t count)
{
    struct mr_spi_device *spi_device = (struct mr_spi_device *)device;

    /* Take the SPI bus */
    ssize_t ret = _spi_device_take_bus(spi_device);
    if (ret < 0)
    {
        return ret;
    }

    if (spi_device->config.role == MR_SPI_ROLE_MASTER)
    {
        /* Set the chip select pin to active */
        _spi_device_cs_set(spi_device, true);

        /* Polling SPI bus as master */
        ret = _spi_device_master_read(spi_device, pos, buf, count);

        /* Set the chip select pin to inactive */
        _spi_device_cs_set(spi_device, false);
    } else
    {
        /* Polling SPI bus as a slave */
        ret = _spi_device_slave_read(spi_device, pos, buf, count);
    }

    /* Release the SPI bus */
    _spi_device_release_bus(spi_device);

    /* Return the number of bytes read or error code */
    return ret;
}

static ssize_t spi_device_write(struct mr_device *device, int pos,
                                const void *buf, size_t count)
{
    struct mr_spi_device *spi_device = (struct mr_spi_device *)device;

    /* Take the SPI bus */
    ssize_t ret = _spi_device_take_bus(spi_device);
    if (ret < 0)
    {
        return ret;
    }

    if (spi_device->config.role == MR_SPI_ROLE_MASTER)
    {
        /* Set the chip select pin to active */
        _spi_device_cs_set(spi_device, true);

        /* Polling SPI bus as master */
        ret = _spi_device_master_write(spi_device, pos, buf, count);

        /* Set the chip select pin to inactive */
        _spi_device_cs_set(spi_device, false);
    } else
    {
        /* Polling SPI bus as a slave */
        ret = _spi_device_slave_write(spi_device, pos, buf, count);
    }

    /* Release the SPI bus */
    _spi_device_release_bus(spi_device);

    /* Return the number of bytes written or error code */
    return ret;
}

static int spi_device_ioctl(struct mr_device *device, int pos, int cmd,
                            void *args)
{
    struct mr_spi_device *spi_device = (struct mr_spi_device *)device;
    struct mr_spi_bus *spi_bus =
        _MR_DEVICE_PARENT_GET((struct mr_device *)spi_device);

    switch (cmd)
    {
        case MR_CTRL_SET(MR_CMD_SPI_CONFIG):
        {
            struct mr_spi_config old_config = spi_device->config;
            struct mr_spi_config *config = (struct mr_spi_config *)args;
            int ret;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Validate the configuration */
            if ((config->reg_bits > config->data_bits) ||
                (config->data_bits == 0) || ((config->data_bits % 8) != 0) ||
                (config->data_bits > 32) || ((config->reg_bits % 8) != 0))
            {
                return MR_EINVAL;
            }

            /* Reconfigure CS if SPI device role is changed */
            if (config->role != spi_device->config.role)
            {
                spi_device->config = *config;
                ret = _spi_device_cs_configure(spi_device, true);
                if (ret < 0)
                {
                    spi_device->config = old_config;
                    return ret;
                }
            }

            /* If holding the bus, release it */
            if (spi_device == spi_bus->owner)
            {
                _MR_DEVICE_OPERATOR_SET((struct mr_device *)spi_bus, -1);
                spi_bus->owner = NULL;
            }

            /* If the role is slave, try to take the bus */
            if (config->role == MR_SPI_ROLE_SLAVE)
            {
                spi_device->config = *config;
                ret = _spi_device_take_bus(spi_device);
                if (ret < 0)
                {
                    return ret;
                }
            }

            /* Update configuration */
            spi_device->config = *config;
            return sizeof(*config);
        }
        case MR_CTRL_SET(MR_CMD_SPI_RD_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Update FIFO size */
            spi_device->rfifo_size = *fifo_size;
            int ret = _spi_device_fifo_update(&spi_device->rfifo,
                                              &spi_device->rfifo_size);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*fifo_size);
        }
        case MR_CTRL_CLR(MR_CMD_SPI_RD_FIFO_DATA):
        {
            /* Reset FIFO */
            mr_fifo_reset(&spi_device->rfifo);
            return MR_EOK;
        }
        case MR_CTRL_GET(MR_CMD_SPI_CONFIG):
        {
            struct mr_spi_config *config = (struct mr_spi_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get configuration */
            *config = spi_device->config;
            return sizeof(*config);
        }
        case MR_CTRL_GET(MR_CMD_SPI_RD_FIFO_SIZE):
        {
            size_t *fifo_size = (size_t *)args;

            if (fifo_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get FIFO size */
            *fifo_size = spi_device->rfifo_size;
            return sizeof(*fifo_size);
        }
        case MR_CTRL_GET(MR_CMD_SPI_RD_FIFO_DATA):
        {
            size_t *data_size = (size_t *)args;

            if (data_size == NULL)
            {
                return MR_EINVAL;
            }

            /* Get data size */
            *data_size = mr_fifo_used_get(&spi_device->rfifo);
            return sizeof(*data_size);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function register a SPI device.
 *
 * @param spi_device The SPI device.
 * @param path The device path.
 * @param cs_pin The chip select pin number.
 * @param cs_active The chip select pin active level.
 * @param spi_bus_name The spi-bus name.
 *
 * @return The error code.
 */
int mr_spi_device_register(struct mr_spi_device *spi_device, const char *path,
                           int cs_pin, int cs_active, const char *spi_bus_name)
{
    MR_ASSERT(spi_device != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT((cs_active >= MR_SPI_CS_ACTIVE_LOW) &&
              (cs_active <= MR_SPI_CS_ACTIVE_NONE));

    static struct mr_device_ops ops = {.open = spi_device_open,
                                       .close = spi_device_close,
                                       .read = spi_device_read,
                                       .write = spi_device_write,
                                       .ioctl = spi_device_ioctl};
    struct mr_spi_config default_config = MR_SPI_CONFIG_DEFAULT;

    /* Initialize the spi-device */
    spi_device->config = default_config;
    mr_fifo_init(&spi_device->rfifo, NULL, 0);
#ifndef MR_CFG_SPI_RD_FIFO_SIZE
#define MR_CFG_SPI_RD_FIFO_SIZE (0)
#endif /* MR_CFG_SPI_RD_FIFO_SIZE */
    spi_device->rfifo_size = MR_CFG_SPI_RD_FIFO_SIZE;
    spi_device->cs_pin = (cs_active != MR_SPI_CS_ACTIVE_NONE) ? cs_pin : -1;
    spi_device->cs_active = (cs_pin >= 0) ? cs_active : MR_SPI_CS_ACTIVE_NONE;

    /* Register the spi-device to the spi-bus */
    return mr_device_register_to((struct mr_device *)spi_device, path,
                                 MR_DEVICE_TYPE_SPI, &ops, NULL, spi_bus_name);
}

/**
 * @brief This function unregister a SPI device.
 *
 * @param spi_device The SPI device.
 *
 * @return The error code.
 */
int mr_spi_device_unregister(struct mr_spi_device *spi_device)
{
    MR_ASSERT(spi_device != MR_NULL);

    return mr_device_unregister((struct mr_device *)spi_device);
}

#endif /* MR_USE_SPI */
