#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#include <board.h>

//<<< Config switch >>>
#define MR_CONF_DISABLE                 0
#define MR_CONF_ENABLE                  1

//<<< Log level >>>
#define MR_CONF_LOG_LEVEL_ASSERT		0
#define MR_CONF_LOG_LEVEL_ERROR			1
#define MR_CONF_LOG_LEVEL_WARNING		2
#define MR_CONF_LOG_LEVEL_INFO			3
#define MR_CONF_LOG_LEVEL_DEBUG			4

//<---------------------------- Kernel -------------------------------------->
#define MR_CONF_NAME_MAX                12

//<<< Log >>>
#define MR_CONF_LOG_OUTPUT				MR_CONF_DISABLE
#define MR_CONF_LOG_BUFSZ				128
#define MR_CONF_LOG_ASSERT				MR_CONF_DISABLE
#define MR_CONF_LOG_LEVEL				MR_CONF_LOG_LEVEL_ASSERT

//<<< Manager >>>
#define MR_CONF_MANAGER				    MR_CONF_DISABLE
#define MR_CONF_MANAGER_PARSER_BUFSZ	16

//<<< Device >>>
#define MR_CONF_DEVICE					MR_CONF_DISABLE

//<---------------------------- Device -------------------------------------->
//<<< GPIO >>>
#define MR_CONF_DEVICE_PIN             MR_CONF_DISABLE

//<<< SERIAL >>>
#define MR_CONF_DEVICE_SERIAL          MR_CONF_DISABLE
#define MR_CONF_SERIAL_BUFSZ           64

//<<< SPI >>>
#define MR_CONF_DEVICE_SPI             MR_CONF_DISABLE

//<<< I2C >>>
#define MR_CONF_DEVICE_I2C             MR_CONF_DISABLE

//<<< ADC >>>
#define MR_CONF_DEVICE_ADC             MR_CONF_DISABLE

//<<< DAC >>>
#define MR_CONF_DEVICE_DAC             MR_CONF_DISABLE

//<<< TIMER >>>
#define MR_CONF_DEVICE_TIMER           MR_CONF_DISABLE

#endif
