#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

/**
 * @include bsp head file
 */
#include "ch32v30x.h"

/**
 * @def Bsp name
 */
#define MR_BSP_NAME                     "ch32-v2/v3"

/**
 * @def Bsp system clock
 */
#define MR_BSP_SYSCLK_FREQ              SystemCoreClock

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

/**
 * @def Bsp dac
 */
#define MR_BSP_DAC_1
#define MR_BSP_DAC_2

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

#endif /* _MR_BOARD_H_ */