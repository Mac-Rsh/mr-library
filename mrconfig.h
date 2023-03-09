#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#define MR_CONF_ENABLE      1
#define MR_CONF_DISABLE     0

#include <board.h>

#define MR_NAME_MAX         10

#define MR_LIBRARY          MR_CONF_ENABLE

#define MR_DEVICE_PIN                   CONF_ENABLE
#define MR_DEVICE_SPI                   CONF_ENABLE
#define MR_DEVICE_ADC                   CONF_ENABLE
#define MR_DEVICE_DAC                   CONF_ENABLE
#define MR_DEVICE_I2C                   CONF_ENABLE
#define MR_DEVICE_SERIAL                CONF_ENABLE
#define MR_SERIAL_FIFO_MIN              64

#endif
