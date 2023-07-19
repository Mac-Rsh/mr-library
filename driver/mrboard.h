#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

//<---------------------------- Hardware -------------------------------------->
//<<< Please include bsp head file >>>


//<<< Please give your bsp a name >>>
#define BSP_NAME                          "bsp-name"

//<<< Bsp system frequency >>>
#define BSP_SYSCLK_FREQ                   144000000

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

//<--------------------------- BSP-I2C -------------------------------------->
//<<< Bsp using I2Cx >>>
#define BSP_I2C_1
#define BSP_I2C_2
#define BSP_I2C_3

//<--------------------------- BSP-ADC -------------------------------------->
//<<< Bsp using ADCx >>>
#define BSP_ADC_1
#define BSP_ADC_2

//<--------------------------- BSP-DAC -------------------------------------->
//<<< Bsp using DACx >>>
#define BSP_DAC_1
#define BSP_DAC_2
#define BSP_DAC_3

//<--------------------------- BSP-TIMER ------------------------------------>
//<<< Bsp using TIMERx >>>
#define BSP_TIMER_1
#define BSP_TIMER_2
#define BSP_TIMER_3
#define BSP_TIMER_4

#endif /* _MR_BOARD_H_ */
