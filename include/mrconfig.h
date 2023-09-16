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

//<------------------------------------ Kernel config ------------------------------------>

/**
 * @def Object name size config
 *
 * Recommend size: 4 * n.
 */
#define MR_CFG_OBJECT_NAME_SIZE         12

/**
 * @def Debug config
 *
 * MR_CFG_DISABLE: Disable debug
 * MR_CFG_ENABLE: Enable debug
 */
#define MR_CFG_DEBUG                    MR_CFG_ENABLE

#if (MR_CFG_DEBUG == MR_CFG_ENABLE)

/**
 * @def Debug level config
 *
 * MR_CFG_DISABLE: Disable one
 * MR_CFG_ENABLE: Enable one
 */
#define MR_CFG_DEBUG_ASSERT             MR_CFG_ENABLE
#define MR_CFG_DEBUG_ERROR              MR_CFG_ENABLE
#define MR_CFG_DEBUG_WARNING            MR_CFG_ENABLE
#define MR_CFG_DEBUG_INFO               MR_CFG_ENABLE
#define MR_CFG_DEBUG_DEBUG              MR_CFG_ENABLE

#endif

/**
 * @def Auto init config
 *
 * MR_CFG_DISABLE: Disable auto init
 * MR_CFG_ENABLE: Enable auto init
 */
#define MR_CFG_AUTO_INIT                MR_CFG_ENABLE

//<------------------------------------ Device config ------------------------------------>

/**
 * @def Device config
 *
 * MR_CFG_DISABLE: Disable device
 * MR_CFG_ENABLE: Enable device
 */
#define MR_CFG_DEVICE                   MR_CFG_ENABLE

#if (MR_CFG_DEVICE == MR_CFG_ENABLE)

/**
 * @def Console name config
 *
 * Device name, default is "uart1"
 */
#define MR_CFG_CONSOLE_NAME             "uart1"

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
 * @def Serial RX/TX buffer default size
 *
 * If the default configuration is not required, set the value to 0
 */
#define MR_CFG_SERIAL_RX_BUFSZ          32
#define MR_CFG_SERIAL_TX_BUFSZ          0

/**
 * @def Serial DMA config
 *
 * MR_CFG_DISABLE: Disable serial dma
 * MR_CFG_ENABLE: Enable serial dma
 */
#define MR_CFG_SERIAL_DMA               MR_CFG_ENABLE
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

//<------------------------------------ Module config ------------------------------------>

#ifdef __cplusplus
}
#endif

#endif /* _MR_CONFIG_H_ */