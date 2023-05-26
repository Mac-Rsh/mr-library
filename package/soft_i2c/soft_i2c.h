/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-26     MacRsh       first version
 */

#ifndef _SOFT_I2C_H_
#define _SOFT_I2C_H_

#include "stdint.h"

#define I2C_ASSERT(x)

typedef struct i2c_bus *i2c_bus_t;
struct i2c_device
{
	uint8_t addr;

	struct i2c_bus *bus;
};
typedef struct i2c_device *i2c_device_t;

struct i2c_bus
{
	struct
	{
		void (*clk_ctrl)(i2c_bus_t i2c_bus, uint8_t state);
		void (*sda_ctrl)(i2c_bus_t i2c_bus, uint8_t state);
		uint8_t (*read)(i2c_bus_t i2c_bus);
	} io;

	struct
	{
		struct i2c_device *device;
		uint8_t lock;
	} owner;

	void *data;
};

#define I2C_ERR_OK                      0
#define I2C_ERR_BUSY                    1

void i2c_bus_init(i2c_bus_t bus,
				  void (*clk_ctrl)(i2c_bus_t i2c_bus, uint8_t state),
				  void (*sda_ctrl)(i2c_bus_t i2c_bus, uint8_t state),
				  uint8_t (*read)(i2c_bus_t i2c_bus),
				  void *data);
void i2c_device_init(i2c_device_t device, uint8_t addr, i2c_bus_t bus);
void i2c_device_write_reg(i2c_device_t device, uint8_t reg, uint8_t data);
uint8_t i2c_device_read_reg(i2c_device_t device, uint8_t reg);
void i2c_device_transfer(i2c_device_t device,
						 const uint8_t *write_buffer,
						 size_t write_size,
						 uint8_t *read_buffer,
						 size_t read_size);

#endif
