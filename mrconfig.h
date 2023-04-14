#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#include <board.h>

#define MR_CONF_DISABLE                 0
#define MR_CONF_ENABLE                  1

//<---------------------------- Kernel -------------------------------------->
#define MR_NAME_MAX                     12

//<---------------------------- Device -------------------------------------->
//<<< GPIO >>>
#define MR_DEVICE_PIN                   MR_CONF_ENABLE

//<<< SERIAL >>>
#define MR_DEVICE_SERIAL                MR_CONF_ENABLE
#define MR_SERIAL_FIFO_SIZE             64

//<<< SPI >>>
#define MR_DEVICE_SPI                   MR_CONF_ENABLE

//<<< I2C >>>
#define MR_DEVICE_I2C                   MR_CONF_ENABLE

//<<< ADC >>>
#define MR_DEVICE_ADC                   MR_CONF_ENABLE

//<<< DAC >>>
#define MR_DEVICE_DAC                   MR_CONF_ENABLE

//<<< TIMER >>>
#define MR_DEVICE_TIMER                 MR_CONF_ENABLE

#endif
