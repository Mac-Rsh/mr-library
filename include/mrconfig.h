#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#include "mrboard.h"

/**
 * @def Configuration mode
 */
#define MR_CONF_DISABLE                 0
#define MR_CFG_ENABLE                   1

/**
 * @def Debug level
 */
#define MR_CFG_DEBUG_LEVEL_ASSERT       0
#define MR_CFG_DEBUG_LEVEL_ERROR        1
#define MR_CFG_DEBUG_LEVEL_WARNING      2
#define MR_CFG_DEBUG_LEVEL_INFO         3
#define MR_CFG_DEBUG_LEVEL_DEBUG        4

//<---------------------------- Configuration -------------------------------------->
/**
 * @def Object name size
 *
 * Recommend size: 2^n - 1.
 */
#define MR_CFG_OBJECT_NAME_SIZE         15

/**
 * @def Auto init config
 *
 * MR_CFG_DISABLE: Disable auto init
 * MR_CFG_ENABLE: Enable auto init
 */
#define MR_CFG_AUTO_INIT                MR_CFG_ENABLE

/**
 * @def FSM config
 *
 * MR_CFG_DISABLE: Disable fsm
 * MR_CFG_ENABLE: Enable fsm
 */
#define MR_CFG_FSM                      MR_CFG_ENABLE

/**
 * @def Event config
 *
 * MR_CFG_DISABLE: Disable event
 * MR_CFG_ENABLE: Enable event
 */
#define MR_CFG_EVENT                    MR_CFG_ENABLE

/**
 * @def Soft timer config
 *
 * MR_CFG_DISABLE: Disable soft timer
 * MR_CFG_ENABLE: Enable soft timer
 */
#define MR_CFG_SOFT_TIMER               MR_CFG_ENABLE

/**
 * @def Debug config
 *
 * MR_CFG_DISABLE: Disable debug
 * MR_CFG_ENABLE: Enable debug
 */
#define MR_CFG_DEBUG                    MR_CFG_ENABLE

#if (MR_CFG_DEBUG == MR_CFG_ENABLE)

/**
 * @def Assert config
 *
 * MR_CFG_DISABLE: Disable assert
 * MR_CFG_ENABLE: Enable assert
 */
#define MR_CFG_ASSERT                   MR_CFG_ENABLE

/**
 * @def Debug level
 *
 * MR_CFG_DEBUG_LEVEL_ASSERT: Assert
 * MR_CFG_DEBUG_LEVEL_ERROR: Error
 * MR_CFG_DEBUG_LEVEL_WARNING: Warning
 * MR_CFG_DEBUG_LEVEL_INFO: Info
 * MR_CFG_DEBUG_LEVEL_DEBUG: Debug
 */
#define MR_CFG_DEBUG_LEVEL              MR_CFG_DEBUG_LEVEL_DEBUG

/**
 * @def Debug console name
 */
#define MR_CFG_DEBUG_CONSOLE_NAME       "uart1"

#endif

/**
 * @def Device config
 *
 * MR_CFG_DISABLE: Disable device
 * MR_CFG_ENABLE: Enable device
 */
#define MR_CFG_DEVICE                   MR_CFG_ENABLE

#if (MR_CFG_DEVICE == MR_CFG_ENABLE)

/**
 * @def Pin config
 *
 * MR_CFG_DISABLE: Disable pin
 * MR_CFG_ENABLE: Enable pin
 */
#define MR_CFG_PIN                      MR_CFG_ENABLE

/**
 * @def Serial config
 *
 * MR_CFG_DISABLE: Disable serial
 * MR_CFG_ENABLE: Enable serial
 */
#define MR_CFG_SERIAL                  MR_CFG_ENABLE

/**
 * @def ADC config
 *
 * MR_CFG_DISABLE: Disable adc
 * MR_CFG_ENABLE: Enable adc
 */
#define MR_CFG_ADC                      MR_CFG_ENABLE

/**
 * @def DAC config
 *
 * MR_CFG_DISABLE: Disable dac
 * MR_CFG_ENABLE: Enable dac
 */
#define MR_CFG_DAC                      MR_CFG_ENABLE

/**
 * @def I2C config
 *
 * MR_CFG_DISABLE: Disable i2c
 * MR_CFG_ENABLE: Enable i2c
 */
#define MR_CFG_I2C                      MR_CFG_ENABLE

/**
 * @def SPI config
 *
 * MR_CFG_DISABLE: Disable spi
 * MR_CFG_ENABLE: Enable spi
 */
#define MR_CFG_SPI                       MR_CFG_ENABLE

#endif

//<< timer >>
#define MR_CONF_TIMER                   MR_CFG_ENABLE
//<< pwm >>
#define MR_CONF_PWM                     MR_CFG_ENABLE

#endif /* _MR_CONFIG_H_ */