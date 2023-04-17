#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#include <board.h>

//<<< CONFIG SWITCH >>>
#define MR_CONF_DISABLE                 0
#define MR_CONF_ENABLE                  1

//<<< LOG LEVEL >>>
#define MR_LOG_LEVEL_ASSERT				0
#define MR_LOG_LEVEL_ERROR				1
#define MR_LOG_LEVEL_WARNING			2
#define MR_LOG_LEVEL_INFO				3
#define MR_LOG_LEVEL_DEBUG				4

//<---------------------------- Kernel -------------------------------------->
#define MR_NAME_MAX                     12

//<<< LOG >>>
#define MR_LOG_OUTPUT					MR_CONF_ENABLE
#define MR_LOG_BUFSZ					128
#define MR_LOG_ASSERT					MR_CONF_ENABLE
#define MR_LOG_LEVEL					MR_LOG_LEVEL_DEBUG

//<---------------------------- Device -------------------------------------->
//<<< GPIO >>>
#define MR_DEVICE_PIN                   MR_CONF_ENABLE

//<<< SERIAL >>>
#define MR_DEVICE_SERIAL                MR_CONF_ENABLE
#define MR_SERIAL_BUFSZ                 64

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
