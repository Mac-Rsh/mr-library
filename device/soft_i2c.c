/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-13    MacRsh       First version
 */

#include "include/device/soft_i2c.h"

#if defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C)

#ifdef MR_USING_PIN
#include "include/device/pin.h"
#else
#warning "Please define MR_USING_PIN. Otherwise Soft-I2C will not work."
#endif /* MR_USING_PIN */

#define SOFT_I2C_LOW                    0
#define SOFT_I2C_HIGH                   1

MR_INLINE void soft_i2c_scl_set(struct mr_soft_i2c_bus *soft_i2c_bus, uint8_t value)
{
    mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->scl_pin);
    mr_dev_write(soft_i2c_bus->desc, &value, sizeof(value));
}

MR_INLINE void soft_i2c_bus_sda_set(struct mr_soft_i2c_bus *soft_i2c_bus, uint8_t value)
{
    mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->sda_pin);
    mr_dev_write(soft_i2c_bus->desc, &value, sizeof(value));
}

MR_INLINE uint8_t soft_i2c_sda_get(struct mr_soft_i2c_bus *soft_i2c_bus)
{
    uint8_t value = 0;
    mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->sda_pin);
    mr_dev_read(soft_i2c_bus->desc, &value, sizeof(value));
    return value;
}

static void soft_i2c_bus_wait_ack(struct mr_i2c_bus *i2c_bus)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    soft_i2c_sda_get(soft_i2c_bus);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
}

static void soft_i2c_bus_send_ack(struct mr_i2c_bus *i2c_bus, int ack)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    if (ack == MR_ENABLE)
    {
        soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_LOW);
    } else
    {
        soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
    }

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
}

static int mr_soft_i2c_bus_configure(struct mr_i2c_bus *i2c_bus, struct mr_i2c_config *config, int addr, int addr_bits)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;
    int state = (config->baud_rate != 0) ? MR_ENABLE : MR_DISABLE;

    if (state == MR_ENABLE)
    {
        if (soft_i2c_bus->desc < 0)
        {
            soft_i2c_bus->desc = mr_dev_open("pin", MR_OFLAG_RDWR);
            if (soft_i2c_bus->desc < 0)
            {
                return soft_i2c_bus->desc;
            }
        }

        /* Configure SCL pin */
        mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->scl_pin);
        int ret = mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_OUTPUT_OD));
        if (ret < 0)
        {
            return ret;
        }

        /* Configure SDA pin */
        mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->sda_pin);
        ret = mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_OUTPUT_OD));
        if (ret < 0)
        {
            return ret;
        }
    } else
    {
        if (soft_i2c_bus->desc >= 0)
        {
            /* Reconfigure SCL pin */
            mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->scl_pin);
            int ret = mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_NONE));
            if (ret < 0)
            {
                return ret;
            }

            /* Reconfigure SDA pin */
            mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_NUMBER, &soft_i2c_bus->sda_pin);
            ret = mr_dev_ioctl(soft_i2c_bus->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_NONE));
            if (ret < 0)
            {
                return ret;
            }

            mr_dev_close(soft_i2c_bus->desc);
            soft_i2c_bus->desc = -1;
        }
    }
    return MR_EOK;
}

static void mr_soft_i2c_bus_start(struct mr_i2c_bus *i2c_bus)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);

    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
}

static void mr_soft_i2c_bus_write(struct mr_i2c_bus *i2c_bus, uint8_t data);

static void mr_soft_i2c_bus_send_addr(struct mr_i2c_bus *i2c_bus, int addr, int addr_bits)
{
    mr_soft_i2c_bus_write(i2c_bus, addr);
    if (addr_bits == MR_I2C_ADDR_BITS_10)
    {
        mr_soft_i2c_bus_write(i2c_bus, (addr >> 8));
    }
}

static void mr_soft_i2c_bus_stop(struct mr_i2c_bus *i2c_bus)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_LOW);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);

    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
}

static uint8_t mr_soft_i2c_bus_read(struct mr_i2c_bus *i2c_bus, int ack_state)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;
    uint8_t data = 0;
    size_t bits = 0;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);

    for (bits = 0; bits < 8; bits++)
    {
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        data <<= 1;
        if (soft_i2c_sda_get(soft_i2c_bus) == SOFT_I2C_HIGH)
        {
            data |= 0x01;
        }
    }

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_send_ack(i2c_bus, ack_state);
    return data;
}

static void mr_soft_i2c_bus_write(struct mr_i2c_bus *i2c_bus, uint8_t data)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;
    size_t bits = 0;

    for (bits = 0; bits < 8; bits++)
    {
        if (data & 0x80)
        {
            soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
        } else
        {
            soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_LOW);
        }
        data <<= 1;

        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    }
    soft_i2c_bus_wait_ack(i2c_bus);
}

/**
 * @brief This function registers a soft-i2c-bus.
 *
 * @param soft_i2c_bus The soft-i2c-bus.
 * @param name The name of the soft-i2c-bus.
 * @param scl_pin The scl pin of the soft-i2c-bus.
 * @param sda_pin The sda pin of the soft-i2c-bus.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus, const char *name, int scl_pin, int sda_pin)
{
    static struct mr_i2c_bus_ops ops =
        {
            mr_soft_i2c_bus_configure,
            mr_soft_i2c_bus_start,
            mr_soft_i2c_bus_send_addr,
            mr_soft_i2c_bus_stop,
            mr_soft_i2c_bus_read,
            mr_soft_i2c_bus_write
        };
    static struct mr_drv drv =
        {
            Mr_Drv_Type_I2C,
            &ops,
            MR_NULL
        };

    mr_assert(soft_i2c_bus != MR_NULL);
    mr_assert(name != MR_NULL);

    /* Initialize the fields */
    soft_i2c_bus->delay = 0;
    soft_i2c_bus->scl_pin = scl_pin;
    soft_i2c_bus->sda_pin = sda_pin;
    soft_i2c_bus->desc = -1;

    return mr_i2c_bus_register(&soft_i2c_bus->i2c_bus, name, &drv);
}

#endif /* defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C) */
