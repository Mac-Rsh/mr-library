#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#define MR_CONF_ENABLE                  1
#define MR_CONF_DISABLE                 0

#include <board.h>

#define MR_NAME_MAX                     10

#define MR_DEVICE_SERIAL                MR_CONF_ENABLE
#define MR_SERIAL_FIFO_SIZE             64

#define MR_DEVICE_PIN                    MR_CONF_ENABLE
#define MR_DEVICE_SPI                    MR_CONF_ENABLE

#endif
