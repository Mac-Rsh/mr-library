/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ch32v30x.h"

#define MR_USING_ADC1
#define MR_USING_ADC2

#define MR_USING_DAC1

#define MR_USING_GPIOA
#define MR_USING_GPIOB
#define MR_USING_GPIOC
#define MR_USING_GPIOD
#define MR_USING_GPIOE

#define MR_USING_UART1
#define MR_CFG_UART1_GROUP              1
#define MR_USING_UART2
#define MR_CFG_UART2_GROUP              1
#define MR_USING_UART3
#define MR_CFG_UART3_GROUP              1
#define MR_USING_UART4
#define MR_CFG_UART4_GROUP              1
#define MR_USING_UART5
#define MR_CFG_UART5_GROUP              1
#define MR_USING_UART6
#define MR_CFG_UART6_GROUP              1
#define MR_USING_UART7
#define MR_CFG_UART7_GROUP              1
#define MR_USING_UART8
#define MR_CFG_UART8_GROUP              1

#define MR_USING_SPI1
#define MR_CFG_SPI1_GROUP               1
#define MR_USING_SPI2
#define MR_CFG_SPI2_GROUP               1
#define MR_USING_SPI3
#define MR_CFG_SPI3_GROUP               1

#define MR_USING_I2C1
#define MR_CFG_I2C1_GROUP               1
#define MR_USING_I2C2
#define MR_CFG_I2C2_GROUP               1

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
