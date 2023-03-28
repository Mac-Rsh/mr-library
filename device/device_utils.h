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

#if (MR_DEVICE_PIN == MR_CONF_ENABLE)
#include <pin/pin.h>

MR_INLINE mr_err_t mr_pin_init(mr_device_t pin, mr_uint16_t number, mr_uint16_t mode)
{
	struct mr_pin_config config = {number,
								   mode};
	mr_err_t ret = MR_ERR_OK;

	ret = mr_device_ioctl(pin, MR_CMD_CONFIG, &config);
	return ret;
}

MR_INLINE mr_level_t mr_pin_read(mr_device_t pin, mr_uint16_t number)
{
	mr_level_t level = MR_LOW;

	mr_device_read(pin, number, &level, 1);
	return level;
}

MR_INLINE void mr_pin_write(mr_device_t pin, mr_uint16_t number, mr_level_t level)
{
	mr_device_write(pin, number, &level, 1);
}

MR_INLINE void mr_pin_toggle(mr_device_t pin, mr_uint16_t number)
{
	mr_level_t level = MR_LOW;

	mr_device_read(pin, number, &level, 1);
	level = ! level;
	mr_device_write(pin, number, &level, 1);
}

#endif

#if (MR_DEVICE_SERIAL == MR_CONF_ENABLE)
#include <serial/serial.h>

MR_INLINE mr_err_t mr_serial_init(mr_device_t serial, mr_uint32_t baud_rate)
{
	struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;
	mr_err_t ret = MR_ERR_OK;

	config.baud_rate = baud_rate;
	ret = mr_device_ioctl(serial, MR_CMD_CONFIG, &config);
	return ret;
}

MR_INLINE mr_uint8_t mr_serial_read(mr_device_t serial)
{
	mr_uint8_t data = 0;

	mr_device_read(serial, - 1, &data, 1);
	return data;
}

MR_INLINE mr_size_t mr_serial_read_buffer(mr_device_t serial, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_read(serial, - 1, buffer, count);
}

MR_INLINE void mr_serial_write(mr_device_t serial, mr_uint8_t data)
{
	mr_device_write(serial, - 1, &data, 1);
}

MR_INLINE mr_size_t mr_serial_write_buffer(mr_device_t serial, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_write(serial, - 1, buffer, count);
}

#endif

#if (MR_DEVICE_SPI == MR_CONF_ENABLE)
#include <spi/spi.h>

MR_INLINE mr_err_t mr_spi_init(mr_device_t spi_device, mr_uint32_t baud_rate, mr_uint8_t mode, const char *bus_name)
{
	struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;
	mr_err_t ret = MR_ERR_OK;

	config.baud_rate = baud_rate;
	config.mode = mode;
	ret = mr_device_ioctl(spi_device, MR_CMD_CONFIG, &config);
	if (ret != MR_ERR_OK)
		return ret;

	ret = mr_device_ioctl(spi_device, MR_CMD_ATTACH, (void *)bus_name);
	return ret;
}

MR_INLINE mr_uint8_t mr_spi_read(mr_device_t spi_device)
{
	mr_uint8_t data = 0;

	mr_device_read(spi_device, - 1, &data, 1);
	return data;
}

MR_INLINE mr_size_t mr_spi_read_buffer(mr_device_t spi_device, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_read(spi_device, - 1, buffer, count);
}

MR_INLINE void mr_spi_write(mr_device_t spi_device, mr_uint8_t data)
{
	mr_device_write(spi_device, - 1, &data, 1);
}

MR_INLINE mr_size_t mr_spi_write_buffer(mr_device_t spi_device, mr_uint8_t *buffer, mr_size_t count)
{
	return mr_device_write(spi_device, - 1, buffer, count);
}

MR_INLINE mr_uint8_t mr_spi_write_then_read(mr_device_t spi_device, mr_uint8_t send_data)
{
	mr_uint8_t data = 0;

	mr_device_write(spi_device, - 1, &send_data, 1);
	mr_device_read(spi_device, - 1, &data, 1);
	return data;
}

#endif

#if (MR_DEVICE_ADC == MR_CONF_ENABLE)
#include <adc/adc.h>

MR_INLINE mr_err_t mr_adc_init(mr_device_t adc, mr_uint16_t channel)
{
	struct mr_adc_config config = {channel,
								   MR_ENABLE};
	mr_err_t ret = MR_ERR_OK;

	ret = mr_device_ioctl(adc, MR_CMD_CONFIG, &config);
	return ret;
}

MR_INLINE mr_uint16_t mr_adc_read(mr_device_t adc, mr_uint16_t channel)
{
	mr_uint16_t data = 0;

	mr_device_read(adc, - 1, &data, 1);
	return data;
}

#endif

#if (MR_DEVICE_DAC == MR_CONF_ENABLE)
#include <dac/dac.h>

MR_INLINE mr_err_t mr_dac_init(mr_device_t dac, mr_uint16_t channel)
{
	struct mr_dac_config config = {channel,
								   MR_ENABLE};
	mr_err_t ret = MR_ERR_OK;

	ret = mr_device_ioctl(dac, MR_CMD_CONFIG, &config);
	return ret;
}

MR_INLINE void mr_dac_write(mr_device_t dac, mr_uint16_t channel, mr_uint16_t value)
{
	mr_device_write(dac, - 1, &value, 1);
}

#endif

#endif
