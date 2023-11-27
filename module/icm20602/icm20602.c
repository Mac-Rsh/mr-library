/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#include "icm20602.h"

#ifdef MR_USING_ICM20602

#define ICM20602_SMPLRT_DIV             (0x19)
#define ICM20602_CONFIG                 (0x1A)
#define ICM20602_GYRO_CONFIG            (0x1B)
#define ICM20602_ACCEL_CONFIG           (0x1C)
#define ICM20602_ACCEL_CONFIG_2         (0x1D)
#define ICM20602_ACCEL_XOUT_H           (0x3B)
#define ICM20602_GYRO_XOUT_H            (0x43)
#define ICM20602_PWR_MGMT_1             (0x6B)
#define ICM20602_PWR_MGMT_2             (0x6C)
#define ICM20602_WHO_AM_I               (0x75)

static void icm20602_write_reg(struct mr_icm20602 *icm20602, uint8_t reg, uint8_t data)
{
    mr_dev_ioctl(icm20602->desc, MR_CTL_SET_OFFSET, mr_make_local(int, reg));
    mr_dev_write(icm20602->desc, &data, sizeof(data));
}

static uint8_t icm20602_read_reg(struct mr_icm20602 *icm20602, uint8_t reg)
{
    uint8_t data = 0;

    mr_dev_ioctl(icm20602->desc, MR_CTL_SET_OFFSET, mr_make_local(int, reg | 0x80));
    mr_dev_read(icm20602->desc, &data, sizeof(data));
    return data;
}

static ssize_t icm20602_read_regs(struct mr_icm20602 *icm20602, uint8_t reg, uint8_t *buf, size_t size)
{
    mr_dev_ioctl(icm20602->desc, MR_CTL_SET_OFFSET, mr_make_local(int, reg | 0x80));
    return mr_dev_read(icm20602->desc, buf, size);
}

static int icm20602_self_check(struct mr_icm20602 *icm20602)
{
    size_t count = 0;

    for (count = 0; count < 200; count++)
    {
        if (icm20602_read_reg(icm20602, ICM20602_WHO_AM_I) == 0x12)
        {
            return MR_TRUE;
        }
    }
    return MR_FALSE;
}

static int icm20602_config(struct mr_icm20602 *icm20602, struct mr_icm20602_config *config)
{
    switch (config->acc_range)
    {
        case MR_ICM20602_ACC_RANGE_2G:
        {
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x00);
            break;
        }
        case MR_ICM20602_ACC_RANGE_4G:
        {
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x08);
            break;
        }
        case MR_ICM20602_ACC_RANGE_8G:
        {
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x10);
            break;
        }
        case MR_ICM20602_ACC_RANGE_16G:
        {
            icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG, 0x18);
            break;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }

    switch (config->gyro_range)
    {
        case MR_ICM20602_GYRO_RANGE_250DPS:
        {
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x00);
            break;
        }
        case MR_ICM20602_GYRO_RANGE_500DPS:
        {
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x08);
            break;
        }
        case MR_ICM20602_GYRO_RANGE_1000DPS:
        {
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x10);
            break;
        }
        case MR_ICM20602_GYRO_RANGE_2000DPS:
        {
            icm20602_write_reg(icm20602, ICM20602_GYRO_CONFIG, 0x18);
            break;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
    icm20602->config = *config;
    return MR_EOK;
}

static int mr_icm20602_open(struct mr_dev *dev)
{
    struct mr_icm20602 *icm20602 = (struct mr_icm20602 *)dev;
    char full_name[MR_CFG_NAME_MAX * 2 + 1] = {0};
    struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;
    size_t i = 0;

    /* Open the icm20602 spi-device */
    int ret = mr_dev_get_full_name(dev, full_name, sizeof(full_name));
    if (ret != MR_EOK)
    {
        return ret;
    }
    icm20602->desc = mr_dev_open(full_name, MR_OFLAG_RDWR);
    if (icm20602->desc < 0)
    {
        return icm20602->desc;
    }
    config.baud_rate = 10 * 1000 * 1000;
    mr_dev_ioctl(icm20602->desc, MR_CTL_SET_CONFIG, &config);

    /* Self check */
    if (icm20602_self_check(icm20602) == MR_FALSE)
    {
        mr_log("%s self check failed", dev->name);
        return MR_ENOTFOUND;
    }

    /* Restart */
    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x80);
    mr_delay_ms(10);
    for (i = 0; i < 200; i++)
    {
        if (icm20602_read_reg(icm20602, ICM20602_PWR_MGMT_1) == 0x41)
        {
            break;
        }
    }
    if (i == 200)
    {
        mr_log("%s restart failed\r\n", dev->name);
        return MR_ENOTFOUND;
    }

    /* Init config */
    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_1, 0x01);
    icm20602_write_reg(icm20602, ICM20602_PWR_MGMT_2, 0x00);
    icm20602_write_reg(icm20602, ICM20602_CONFIG, 0x01);
    icm20602_write_reg(icm20602, ICM20602_SMPLRT_DIV, 0x07);

    ret = icm20602_config(icm20602, &icm20602->config);
    if (ret != MR_EOK)
    {
        mr_log("%s init config failed\r\n", dev->name);
        return ret;
    }
    icm20602_write_reg(icm20602, ICM20602_ACCEL_CONFIG_2, 0x03);

    return MR_EOK;
}

static int mr_icm20602_close(struct mr_dev *dev)
{
    struct mr_icm20602 *icm20602 = (struct mr_icm20602 *)dev;

    /* Close the icm20602 spi-device */
    mr_dev_close(icm20602->desc);
    icm20602->desc = -1;

    return MR_EOK;
}

static ssize_t mr_icm20602_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_icm20602 *icm20602 = (struct mr_icm20602 *)dev;
    struct mr_icm20602_data *rd_buf = (struct mr_icm20602_data *)buf;
    uint8_t reg = (off == MR_ICM20602_ACC_OFFSET) ? ICM20602_ACCEL_XOUT_H : ICM20602_GYRO_XOUT_H;
    ssize_t rd_size = 0;

    if ((off != MR_ICM20602_ACC_OFFSET) && (off != MR_ICM20602_GYRO_OFFSET))
    {
        return MR_EINVAL;
    }

    mr_bits_clr(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        uint8_t axis_buf[6] = {0};
        icm20602_read_regs(icm20602, reg, axis_buf, sizeof(axis_buf));
        rd_buf->x = (int16_t)((axis_buf[0] << 8) | axis_buf[1]);
        rd_buf->y = (int16_t)((axis_buf[2] << 8) | axis_buf[3]);
        rd_buf->z = (int16_t)((axis_buf[4] << 8) | axis_buf[5]);
    }
    return rd_size;
}

static int mr_icm20602_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_icm20602 *icm20602 = (struct mr_icm20602 *)dev;

    switch (cmd)
    {
        case MR_CTL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_icm20602_config *config = (struct mr_icm20602_config *)args;

                return icm20602_config(icm20602, config);
            }
            return MR_EINVAL;
        }

        case MR_CTL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_icm20602_config *config = (struct mr_icm20602_config *)args;

                *config = icm20602->config;
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
 * @brief This function register a icm20602 module.
 *
 * @param icm20602 The icm20602 module.
 * @param name The name of the icm20602 module.
 * @param cs_pin The cs pin of the icm20602 module.
 * @param bus_name The spi-bus name of the icm20602 module.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_icm20602_register(struct mr_icm20602 *icm20602, const char *name, int cs_pin, const char *bus_name)
{
    static struct mr_dev_ops ops =
        {
            mr_icm20602_open,
            mr_icm20602_close,
            mr_icm20602_read,
            MR_NULL,
            mr_icm20602_ioctl,
            MR_NULL
        };
    struct mr_icm20602_config default_config = MR_ICM20602_CONFIG_DEFAULT;
    char cat_name[MR_CFG_NAME_MAX + MR_CFG_NAME_MAX + 1] = {0};
    int ret = 0;

    mr_assert(icm20602 != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(cs_pin >= 0);
    mr_assert(bus_name != MR_NULL);

    /* Register the spi-device */
    sprintf(cat_name, "%s/%s", bus_name, name);
    ret = mr_spi_dev_register(&icm20602->spi_dev, cat_name, cs_pin, MR_SPI_CS_ACTIVE_LOW);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Initialize the fields */
    icm20602->config = default_config;
    icm20602->desc = -1;

    /* Register the icm20602 */
    return mr_dev_register(&icm20602->dev, name, Mr_Dev_Type_Sensor, MR_SFLAG_RDONLY | MR_SFLAG_NONDRV, &ops, MR_NULL);
}

#endif /* MR_USING_ICM20602 */