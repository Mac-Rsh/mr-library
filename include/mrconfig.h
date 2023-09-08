#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def Configuration mode
 */
#define MR_CFG_DISABLE                  0
#define MR_CFG_ENABLE                   1

/**
 * @def Debug level
 */
#define MR_CFG_DEBUG_LEVEL_ASSERT       0
#define MR_CFG_DEBUG_LEVEL_ERROR        1
#define MR_CFG_DEBUG_LEVEL_WARNING      2
#define MR_CFG_DEBUG_LEVEL_INFO         3
#define MR_CFG_DEBUG_LEVEL_DEBUG        4

//<------------------------------------ Configuration ------------------------------------>
/**
 * @def Object name size config
 *
 * Recommend size: (4 * n) - 1.
 */
#define MR_CFG_OBJECT_NAME_SIZE         12

/**
 * @def Auto init config
 *
 * MR_CFG_DISABLE: Disable auto init
 * MR_CFG_ENABLE: Enable auto init
 */
#define MR_CFG_AUTO_INIT                MR_CFG_ENABLE

/**
 * @def Task config
 *
 * MR_CFG_DISABLE: Disable task
 * MR_CFG_ENABLE: Enable task
 */
#define MR_CFG_TASK                     MR_CFG_ENABLE

#if (MR_CFG_TASK == MR_CFG_ENABLE)

/**
 * @def Task usage rate record config
 *
 * MR_CFG_DISABLE: Disable task usage rate record
 * MR_CFG_ENABLE: Enable task usage rate record
 */
#define MR_CFG_TASK_USAGE               MR_CFG_ENABLE

#endif

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
 * MR_CFG_DEBUG_LEVEL_INFO: Information
 * MR_CFG_DEBUG_LEVEL_DEBUG: Debug
 */
#define MR_CFG_DEBUG_LEVEL              MR_CFG_DEBUG_LEVEL_DEBUG

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
 * @include Board support package
 */
#include "mrboard.h"

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
 * @def Pin config
 *
 * MR_CFG_DISABLE: Disable pin
 * MR_CFG_ENABLE: Enable pin
 */
#define MR_CFG_PIN                      MR_CFG_ENABLE

/**
 * @def PWM config
 *
 * MR_CFG_DISABLE: Disable pwm
 * MR_CFG_ENABLE: Enable pwm
 */
#define MR_CFG_PWM                      MR_CFG_ENABLE

/**
 * @def Serial config
 *
 * MR_CFG_DISABLE: Disable serial
 * MR_CFG_ENABLE: Enable serial
 */
#define MR_CFG_SERIAL                   MR_CFG_ENABLE

#if (MR_CFG_SERIAL == MR_CFG_ENABLE)

/**
 * @def Serial RX buffer default size
 *
 * If the default configuration is not required, set the value to 0
 */
#define MR_CFG_SERIAL_RX_BUFSZ          32

/**
 * @def Serial TX buffer default size
 *
 * If the default configuration is not required, set the value to 0
 */
#define MR_CFG_SERIAL_TX_BUFSZ          0

#endif

/**
 * @def SPI config
 *
 * MR_CFG_DISABLE: Disable spi
 * MR_CFG_ENABLE: Enable spi
 */
#define MR_CFG_SPI                      MR_CFG_ENABLE

#if (MR_CFG_SPI == MR_CFG_ENABLE)

/**
 * @def SPI RX buffer default size
 *
 * If the default configuration is not required, set the value to 0
 */
#define MR_CFG_SPI_RX_BUFSZ             32

#endif

/**
 * @def Timer config
 *
 * MR_CFG_DISABLE: Disable timer
 * MR_CFG_ENABLE: Enable timer
 */
#define MR_CFG_TIMER                    MR_CFG_ENABLE

#endif

#ifdef __cplusplus
}
#endif

#endif /* _MR_CONFIG_H_ */