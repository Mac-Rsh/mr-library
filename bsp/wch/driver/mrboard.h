#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

//<---------------------------- Hardware -------------------------------------->
//<<< Please include bsp head file >>>
#include "ch32v30x.h"

//<<< Please give your bsp a name >>>
#define BSP_NAME                          "CH32V30x"

//<<< Bsp system frequency >>>
#define BSP_SYSCLK_FREQ                   SystemCoreClock

//<--------------------------- BSP-GPIO ------------------------------------->
//<<< Bsp using GPIOx >>>
#define BSP_GPIO_A
#define BSP_GPIO_B
#define BSP_GPIO_C
#define BSP_GPIO_D
#define BSP_GPIO_E

//<--------------------------- BSP-UART ------------------------------------->
//<<< Bsp using UARTx >>>
#define BSP_UART_1
#define BSP_UART_2
#define BSP_UART_3
#define BSP_UART_4
#define BSP_UART_5
#define BSP_UART_6
#define BSP_UART_7
#define BSP_UART_8

//<--------------------------- BSP-SPI -------------------------------------->
//<<< Bsp using SPIx >>>
#define BSP_SPI_1
#define BSP_SPI_2
#define BSP_SPI_3

//<--------------------------- BSP-ADC -------------------------------------->
//<<< Bsp using ADCx >>>
#define BSP_ADC_1
#define BSP_ADC_2

//<--------------------------- BSP-DAC -------------------------------------->
//<<< Bsp using DACx >>>
#define BSP_DAC_1
#define BSP_DAC_2

//<--------------------------- BSP-TIMER ------------------------------------>
//<<< Bsp using TIMERx >>>
#define BSP_TIMER_1
#define BSP_TIMER_2
#define BSP_TIMER_3
#define BSP_TIMER_4
#define BSP_TIMER_5
#define BSP_TIMER_6
#define BSP_TIMER_7
#define BSP_TIMER_8
#define BSP_TIMER_9
#define BSP_TIMER_10

#endif /* _MR_BOARD_H_ */