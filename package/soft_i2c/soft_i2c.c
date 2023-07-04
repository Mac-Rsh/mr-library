/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#include "soft_i2c.h"

#define i2c_delay(x) for(volatile size_t temp = x; temp--;)

static void i2c_bus_start(i2c_bus_t bus)
{
    bus->ops->scl_write(bus, 1);
    bus->ops->sda_write(bus, 1);

    i2c_delay(I2C_DELAY);
    bus->ops->sda_write(bus, 0);
    i2c_delay(I2C_DELAY);
    bus->ops->scl_write(bus, 0);
}

static void i2c_bus_stop(i2c_bus_t bus)
{
    bus->ops->sda_write(bus, 0);
    bus->ops->scl_write(bus, 0);

    i2c_delay(I2C_DELAY);
    bus->ops->scl_write(bus, 1);
    i2c_delay(I2C_DELAY);
    bus->ops->sda_write(bus, 1);
}

static void i2c_bus_send_ack(i2c_bus_t bus, uint8_t ack)
{
    bus->ops->scl_write(bus, 0);

    i2c_delay(I2C_DELAY);
    if (ack)
    {
        bus->ops->sda_write(bus, 1);
    } else
    {
        bus->ops->sda_write(bus, 0);
    }

    bus->ops->scl_write(bus, 1);
    i2c_delay(I2C_DELAY);
    bus->ops->scl_write(bus, 0);
    i2c_delay(I2C_DELAY);
}

static uint8_t i2c_bus_wait_ack(i2c_bus_t bus)
{
    uint8_t ack;

    bus->ops->scl_write(bus, 0);
    i2c_delay(I2C_DELAY);
    bus->ops->scl_write(bus, 1);
    i2c_delay(I2C_DELAY);

    ack = bus->ops->sda_read(bus);

    bus->ops->scl_write(bus, 0);
    i2c_delay(I2C_DELAY);

    return ack;
}

static void i2c_bus_write(i2c_bus_t bus, uint8_t data)
{
    uint8_t count = 8;

    while (count--)
    {
        if (data & 0x80)
        {
            bus->ops->sda_write(bus, 1);
        } else
        {
            bus->ops->sda_write(bus, 0);
        }
        data = data << 1;

        i2c_delay(I2C_DELAY);
        bus->ops->scl_write(bus, 1);
        i2c_delay(I2C_DELAY);
        bus->ops->scl_write(bus, 0);
    }

    i2c_bus_wait_ack(bus);
}

static uint8_t i2c_bus_read(i2c_bus_t bus, uint8_t ack)
{
    uint8_t data = 0;
    uint8_t count = 8;

    bus->ops->scl_write(bus, 0);
    i2c_delay(I2C_DELAY);
    bus->ops->sda_write(bus, 1);

    while (count--)
    {
        i2c_delay(I2C_DELAY);
        bus->ops->scl_write(bus, 0);
        i2c_delay(I2C_DELAY);
        bus->ops->scl_write(bus, 1);
        i2c_delay(I2C_DELAY);
        data = data << 1;
        if (bus->ops->sda_read(bus) == 1)
        {
            data |= 1;
        }
    }

    bus->ops->scl_write(bus, 0);
    i2c_delay(I2C_DELAY);
    i2c_bus_send_ack(bus, ack);

    return data;
}

static int take_i2c_bus(i2c_device_t device)
{
    if (device->bus->lock == 1)
    {
        return -I2C_ERR_BUSY;
    }

    if (device->bus->owner != device)
    {
        device->bus->owner = device;
    }

    device->bus->lock = 1;

    return I2C_ERR_OK;
}

static int release_i2c_bus(i2c_device_t device)
{
    if (device->bus->owner != device)
    {
        return -I2C_ERR_BUSY;
    }

    device->bus->lock = 0;

    return I2C_ERR_OK;
}

/**
 * @brief This function initialize the i2c bus.
 *
 * @param bus The i2c bus to initialize.
 * @param scl_ctrl The clock control function for i2c bus.
 * @param sda_ctrl The sda control function for i2c bus.
 * @param read The sda_read sda function for i2c bus.
 * @param data The data for user.
 */
void i2c_bus_init(i2c_bus_t bus, const struct i2c_bus_ops *ops, void *data)
{
    I2C_ASSERT(bus != NULL);
    I2C_ASSERT(ops != NULL);

    bus->owner = NULL;
    bus->lock = 0;

    bus->data = data;

    bus->ops = ops;
}

/**
 * @brief This function initialize the i2c device.
 *
 * @param device The i2c device to initialize.
 * @param addr The i2c device address.
 * @param bus The i2c bus to initialize.
 */
void i2c_device_init(i2c_device_t device, uint8_t addr, i2c_bus_t bus)
{
    I2C_ASSERT(device != NULL);
    I2C_ASSERT(bus != NULL);

    device->addr = addr;
    device->bus = bus;
}

/**
 * @brief This function write data to i2c device.
 *
 * @param device The i2c device to write.
 * @param reg The register address to write.
 * @param data The data to write.
 */
void i2c_device_write_reg(i2c_device_t device, uint8_t reg, uint8_t data)
{
    int ret = I2C_ERR_OK;

    I2C_ASSERT(device != NULL);

    ret = take_i2c_bus(device);
    if (ret != I2C_ERR_OK)
    {
        return;
    }

    i2c_bus_start(device->bus);
    i2c_bus_write(device->bus, device->addr << 1);
    i2c_bus_write(device->bus, reg);
    i2c_bus_write(device->bus, data);
    i2c_bus_stop(device->bus);

    release_i2c_bus(device);
}

/**
 * @brief This function sda_read data from i2c device.
 *
 * @param device The i2c device to sda_read.
 * @param reg The register address to sda_read.
 *
 * @return The data sda_read from i2c device.
 */
uint8_t i2c_device_read_reg(i2c_device_t device, uint8_t reg)
{
    int ret = I2C_ERR_OK;
    uint8_t data = 0;

    I2C_ASSERT(device != NULL);

    ret = take_i2c_bus(device);
    if (ret != I2C_ERR_OK)
    {
        return 0;
    }

    i2c_bus_start(device->bus);
    i2c_bus_write(device->bus, device->addr << 1);
    i2c_bus_write(device->bus, reg);

    i2c_bus_start(device->bus);
    i2c_bus_write(device->bus, (device->addr << 1) | 0x01);
    data = i2c_bus_read(device->bus, 1);
    i2c_bus_stop(device->bus);

    release_i2c_bus(device);

    return data;
}

/**
 * @brief This function transfer data to i2c device.
 *
 * @param device The i2c device to transfer.
 * @param write_buffer The data buffer to write.
 * @param write_size The size of data to write.
 * @param read_buffer The data buffer to sda_read.
 * @param read_size The size of data to sda_read.
 */
void i2c_device_transfer(i2c_device_t device,
                         const uint8_t *write_buffer,
                         size_t write_size,
                         uint8_t *read_buffer,
                         size_t read_size)
{
    int ret = I2C_ERR_OK;

    I2C_ASSERT(device != NULL);
    I2C_ASSERT(write_buffer != NULL || write_size == 0);
    I2C_ASSERT(read_buffer != NULL || read_size == 0);

    ret = take_i2c_bus(device);
    if (ret != I2C_ERR_OK)
    {
        return;
    }

    i2c_bus_start(device->bus);
    i2c_bus_write(device->bus, device->addr << 1);
    while (write_size--)
    {
        i2c_bus_write(device->bus, *write_buffer++);
    }

    i2c_bus_start(device->bus);
    i2c_bus_write(device->bus, (device->addr << 1) | 0x01);
    while (read_size--)
    {
        *read_buffer++ = i2c_bus_read(device->bus, 1);
    }
    i2c_bus_stop(device->bus);

    release_i2c_bus(device);
}