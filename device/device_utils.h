/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-08     MacRsh       first version
 */

#ifndef _DEVICE_UTILS_H_
#define _DEVICE_UTILS_H_

#include <mrlib.h>

#if (MR_CONF_PIN == MR_CONF_ENABLE)
#include <pin/pin.h>

mr_inline mr_err_t mr_pin_mode(mr_device_t pin, mr_uint16_t number, mr_uint16_t mode)
{
	struct mr_pin_config config = {number,
								   mode};

	return mr_device_ioctl(pin, MR_CTRL_CONFIG, &config);
}

mr_inline mr_uint8_t mr_pin_read(mr_device_t pin, mr_uint16_t number)
{
	mr_uint8_t level = MR_LOW;

	mr_device_read(pin, number, &level, sizeof(level));
	return level;
}

mr_inline void mr_pin_write(mr_device_t pin, mr_uint16_t number, mr_uint8_t level)
{
	mr_device_write(pin, number, &level, sizeof(level));
}

mr_inline void mr_pin_toggle(mr_device_t pin, mr_uint16_t number)
{
	mr_uint8_t level = MR_LOW;

	mr_device_read(pin, number, &level, sizeof(level));
	level = ! level;
	mr_device_write(pin, number, &level, sizeof(level));
}

#endif

#if (MR_CONF_SERIAL == MR_CONF_ENABLE)
#include <serial/serial.h>

mr_inline mr_err_t mr_serial_mode(mr_device_t serial, mr_uint32_t baud_rate)
{
	struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;

	config.baud_rate = baud_rate;
	return mr_device_ioctl(serial, MR_CTRL_CONFIG, &config);
}

mr_inline mr_uint8_t mr_serial_read(mr_device_t serial)
{
	mr_uint8_t data = 0;

	mr_device_read(serial, 0, &data, sizeof(data));
	return data;
}

mr_inline mr_size_t mr_serial_read_buffer(mr_device_t serial, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_read(serial, 0, buffer, count);
}

mr_inline void mr_serial_write(mr_device_t serial, mr_uint8_t data)
{
	mr_device_write(serial, 0, &data, sizeof(data));
}

mr_inline mr_size_t mr_serial_write_buffer(mr_device_t serial, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_write(serial, 0, buffer, count);
}

mr_inline mr_err_t mr_serial_set_rx_callback(mr_device_t serial,
											 mr_err_t (*rx_callback)(mr_device_t device, void *args))
{
	return mr_device_ioctl(serial, MR_CTRL_SET_RX_CB, (void *)rx_callback);
}

#endif

#if (MR_CONF_SPI == MR_CONF_ENABLE)
#include <spi/spi.h>

mr_inline mr_err_t mr_spi_mode(mr_device_t spi_device, mr_uint32_t baud_rate, mr_uint8_t mode)
{
	struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;
	mr_err_t ret = MR_ERR_OK;

	config.baud_rate = baud_rate;
	config.mode = mode;
	return mr_device_ioctl(spi_device, MR_CTRL_CONFIG, &config);
}

mr_inline mr_uint8_t mr_spi_read(mr_device_t spi_device)
{
	mr_uint8_t data = 0;

	mr_device_read(spi_device, 0, &data, sizeof(data));
	return data;
}

mr_inline mr_size_t mr_spi_read_buffer(mr_device_t spi_device, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_read(spi_device, 0, buffer, count);
}

mr_inline void mr_spi_write(mr_device_t spi_device, mr_uint8_t data)
{
	mr_device_write(spi_device, 0, &data, sizeof(data));
}

mr_inline mr_size_t mr_spi_write_buffer(mr_device_t spi_device, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_write(spi_device, 0, buffer, count);
}

mr_inline void mr_spi_write_reg(mr_device_t spi_device, mr_uint8_t reg, mr_uint8_t data)
{
	mr_device_write(spi_device, reg, &data, sizeof(data));
}

mr_inline mr_uint8_t mr_spi_read_reg(mr_device_t spi_device, mr_uint8_t reg)
{
	mr_uint8_t data = 0;

	mr_device_read(spi_device, reg, &data, sizeof(data));
	return data;
}

#endif

#if (MR_CONF_ADC == MR_CONF_ENABLE)
#include <adc/adc.h>

mr_inline mr_err_t mr_adc_mode(mr_device_t adc, mr_uint16_t channel, mr_uint8_t state)
{
	struct mr_adc_config config = {channel,
								   state};

	return mr_device_ioctl(adc, MR_CTRL_CONFIG, &config);
}

mr_inline mr_uint16_t mr_adc_read(mr_device_t adc, mr_uint16_t channel)
{
	mr_uint32_t data = 0;

	mr_device_read(adc, 0, &data, sizeof(data));
	return data;
}

#endif

#if (MR_CONF_DAC == MR_CONF_ENABLE)
#include <dac/dac.h>

mr_inline mr_err_t mr_dac_mode(mr_device_t dac, mr_uint16_t channel, mr_uint8_t state)
{
	struct mr_dac_config config = {channel,
								   MR_ENABLE};

	return mr_device_ioctl(dac, MR_CTRL_CONFIG, &config);
}

mr_inline void mr_dac_write(mr_device_t dac, mr_uint16_t channel, mr_uint32_t value)
{
	mr_device_write(dac, 0, &value, sizeof(value));
}

#endif