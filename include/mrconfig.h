#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#include "mrboard.h"

//<<< Config switch >>>
#define MR_CONF_DISABLE                 0
#define MR_CONF_ENABLE                  1

//<<< Log level >>>
#define MR_CONF_LOG_LEVEL_ASSERT        0
#define MR_CONF_LOG_LEVEL_ERROR         1
#define MR_CONF_LOG_LEVEL_WARNING       2
#define MR_CONF_LOG_LEVEL_INFO          3
#define MR_CONF_LOG_LEVEL_DEBUG         4

//<---------------------------- Kernel -------------------------------------->
#define MR_CONF_NAME_MAX                12
#define MR_CONF_CONSOLE                 MR_CONF_ENABLE
#define MR_CONF_CONSOLE_BUFSZ           64
#define MR_CONF_CONSOLE_NAME			"uart1"

//<<< Log >>>
#define MR_CONF_LOG_OUTPUT              MR_CONF_ENABLE
#define MR_CONF_LOG_BUFSZ               64
#define MR_CONF_LOG_ASSERT              MR_CONF_ENABLE
#define MR_CONF_LOG_LEVEL               MR_CONF_LOG_LEVEL_DEBUG

//<<< Event >>>
#define MR_CONF_EVENT                 	MR_CONF_ENABLE

//<---------------------------- Device -------------------------------------->
//<<< GPIO >>>
#define MR_CONF_PIN              		MR_CONF_ENABLE
//<<< SERIAL >>>
#define MR_CONF_SERIAL           		MR_CONF_ENABLE
#define MR_CONF_SERIAL_BUFSZ            64
//<<< SPI >>>
#define MR_CONF_SPI              		MR_CONF_ENABLE
//<<< I2C >>>
#define MR_CONF_I2C              		MR_CONF_DISABLE
//<<< ADC >>>
#define MR_CONF_ADC              		MR_CONF_ENABLE
//<<< DAC >>>
#define MR_CONF_DAC             	 	MR_CONF_ENABLE
//<<< TIMER >>>
#define MR_CONF_TIMER            		MR_CONF_DISABLE

#endif