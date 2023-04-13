#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#define MR_CONF_ENABLE                  1
#define MR_CONF_DISABLE                 0

#include "bsp/wch/board.h"

#include "stdlib.h"
#include "stdio.h"

#define mr_delay_ms                        Sleep

#define MR_NAME_MAX                     12

#define MR_DEVICE_SERIAL                MR_CONF_ENABLE
#define MR_SERIAL_FIFO_SIZE             64

#define MR_DEVICE_PIN                   MR_CONF_ENABLE
#define MR_DEVICE_SPI                   MR_CONF_ENABLE
#define MR_DEVICE_I2C                   MR_CONF_ENABLE
#define MR_DEVICE_ADC                   MR_CONF_ENABLE
#define MR_DEVICE_DAC                   MR_CONF_ENABLE
#define MR_DEVICE_TIMER                 MR_CONF_ENABLE

#endif
