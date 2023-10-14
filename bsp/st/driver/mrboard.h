#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @include bsp head file
 *
 * like this: #include "stm32f1xx.h"
 */
#warning "Please include bsp head file, then remove this line"

/**
 * @def Bsp name
 */
#define MR_BSP_NAME                     "bsp-name"

/**
 * @def Bsp system clock
 */
#define MR_BSP_SYSCLK_FREQ              144000000

/**
 * @def Bsp pin number
 */
#define MR_BSP_PIN_NUMBER               144

/**
 * @def Bsp uart
 */
#define MR_BSP_UART_1
#define MR_BSP_UART_2
#define MR_BSP_UART_3
#define MR_BSP_UART_4
#define MR_BSP_UART_5
#define MR_BSP_UART_6
#define MR_BSP_UART_7
#define MR_BSP_UART_8

/**
 * @def Bsp adc
 */
#define MR_BSP_ADC_1
#define MR_BSP_ADC_2
#define MR_BSP_ADC_3

/**
 * @def Bsp dac
 */
#define MR_BSP_DAC_1
#define MR_BSP_DAC_2
#define MR_BSP_DAC_3

/**
 * @def Bsp i2c
 */
#define MR_BSP_I2C_1
#define MR_BSP_I2C_2
#define MR_BSP_I2C_3

/**
 * @def Bsp spi
 */
#define MR_BSP_SPI_1
#define MR_BSP_SPI_2
#define MR_BSP_SPI_3

/**
 * @def Bsp pwm
 */
#define MR_BSP_PWM_1
#define MR_BSP_PWM_2
#define MR_BSP_PWM_3

/**
 * @def Bsp timer
 */
#define MR_BSP_TIMER_1
#define MR_BSP_TIMER_2
#define MR_BSP_TIMER_3

#ifdef __cplusplus
}
#endif

#endif /* _MR_BOARD_H_ */
