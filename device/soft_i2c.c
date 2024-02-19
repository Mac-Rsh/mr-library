/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-13    MacRsh       First version
 */

#include "include/device/mr_soft_i2c.h"

#if defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C)

#ifdef MR_USING_PIN
#include "include/device/mr_pin.h"
int _mr_fast_pin_mode(int number, int mode);
uint8_t _mr_fast_pin_read(int number);
void _mr_fast_pin_write(int number, int value);
#else
#error "Please define MR_USING_PIN. Otherwise Soft-I2C will not work."
#endif /* MR_USING_PIN */

#define SOFT_I2C_LOW                    0
#define SOFT_I2C_HIGH                   1

MR_INLINE void soft_i2c_scl_set(struct mr_soft_i2c_bus *soft_i2c_bus, uint8_t value)
{
    _mr_fast_pin_write(soft_i2c_bus->scl_pin, value);
}

MR_INLINE void soft_i2c_bus_sda_set(struct mr_soft_i2c_bus *soft_i2c_bus, uint8_t value)
{
    _mr_fast_pin_write(soft_i2c_bus->sda_pin, value);
}

MR_INLINE uint8_t soft_i2c_sda_get(struct mr_soft_i2c_bus *soft_i2c_bus)
{
    return (uint8_t)_mr_fast_pin_read(soft_i2c_bus->sda_pin);
}

static int soft_i2c_bus_wait_ack(struct mr_i2c_bus *i2c_bus)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;
    int ret = MR_ETIMEOUT;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);

    if (soft_i2c_sda_get(soft_i2c_bus) == SOFT_I2C_LOW) {
        ret = MR_EOK;
    }
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    return ret;
}

static void soft_i2c_bus_send_ack(struct mr_i2c_bus *i2c_bus, int ack)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    if (ack == MR_ENABLE) {
        soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_LOW);
    } else {
        soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
    }

    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);
}

static int mr_soft_i2c_bus_configure(struct mr_i2c_bus *i2c_bus,
                                     struct mr_i2c_config *config,
                                     int addr,
                                     int addr_bits)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;
    int state = (config->baud_rate != 0) ? MR_ENABLE : MR_DISABLE;
    int mode = (state == MR_ENABLE) ? MR_PIN_MODE_OUTPUT_OD : MR_PIN_MODE_NONE;

    if (state == MR_ENABLE) {
        /* Soft I2C only support host mode */
        if (config->host_slave != MR_I2C_HOST) {
            return MR_ENOTSUP;
        }

        /* Calculate the delay time */
        soft_i2c_bus->delay = (1000000 / config->baud_rate) / 2;
    }

    /* Configure SCL and SDA */
    int ret = _mr_fast_pin_mode(soft_i2c_bus->scl_pin, mode);
    if (ret < 0) {
        return ret;
    }
    ret = _mr_fast_pin_mode(soft_i2c_bus->sda_pin, mode);
    if (ret < 0) {
        return ret;
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

static int mr_soft_i2c_bus_write(struct mr_i2c_bus *i2c_bus, uint8_t data);

static int mr_soft_i2c_bus_send_addr(struct mr_i2c_bus *i2c_bus, int addr, int addr_bits)
{
    if (addr_bits == MR_I2C_ADDR_BITS_10) {
        int ret = mr_soft_i2c_bus_write(i2c_bus, addr >> 8);
        if (ret < 0) {
            return ret;
        }
    }

    int ret = mr_soft_i2c_bus_write(i2c_bus, addr);
    if (ret < 0) {
        return ret;
    }
    return MR_EOK;
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
    mr_delay_us(soft_i2c_bus->delay);
}

static int mr_soft_i2c_bus_read(struct mr_i2c_bus *i2c_bus, uint8_t *data, int ack_state)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_sda_set(soft_i2c_bus, SOFT_I2C_HIGH);

    for (size_t bits = 0; bits < (sizeof(*data) * 8); bits++) {
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        *data <<= 1;
        if (soft_i2c_sda_get(soft_i2c_bus) == SOFT_I2C_HIGH) {
            *data |= 0x01;
        }
    }

    soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    mr_delay_us(soft_i2c_bus->delay);
    soft_i2c_bus_send_ack(i2c_bus, ack_state);
    return MR_EOK;
}

static int mr_soft_i2c_bus_write(struct mr_i2c_bus *i2c_bus, uint8_t data)
{
    struct mr_soft_i2c_bus *soft_i2c_bus = (struct mr_soft_i2c_bus *)i2c_bus;

    for (size_t bits = 0; bits < 8; bits++) {
        soft_i2c_bus_sda_set(soft_i2c_bus, (data & 0x80) ? SOFT_I2C_HIGH : SOFT_I2C_LOW);
        data <<= 1;

        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_HIGH);
        mr_delay_us(soft_i2c_bus->delay);
        soft_i2c_scl_set(soft_i2c_bus, SOFT_I2C_LOW);
    }
    return soft_i2c_bus_wait_ack(i2c_bus);
}

/**
 * @brief This function registers a soft-i2c-bus.
 *
 * @param soft_i2c_bus The soft-i2c-bus.
 * @param path The path of the soft-i2c-bus.
 * @param scl_pin The scl pin of the soft-i2c-bus.
 * @param sda_pin The sda pin of the soft-i2c-bus.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus,
                             const char *path,
                             int scl_pin,
                             int sda_pin)
{
    static struct mr_i2c_bus_ops ops = {mr_soft_i2c_bus_configure,
                                        mr_soft_i2c_bus_start,
                                        mr_soft_i2c_bus_send_addr,
                                        mr_soft_i2c_bus_stop,
                                        mr_soft_i2c_bus_read,
                                        mr_soft_i2c_bus_write};
    static struct mr_drv drv = {&ops, MR_NULL};

    MR_ASSERT(soft_i2c_bus != MR_NULL);
    MR_ASSERT(path != MR_NULL);

    /* Initialize the fields */
    soft_i2c_bus->delay = 0;
    soft_i2c_bus->scl_pin = scl_pin;
    soft_i2c_bus->sda_pin = sda_pin;

    /* Register the soft-i2c-bus */
    return mr_i2c_bus_register(&soft_i2c_bus->i2c_bus, path, &drv);
}

#endif /* defined(MR_USING_I2C) && defined(MR_USING_SOFT_I2C) */
