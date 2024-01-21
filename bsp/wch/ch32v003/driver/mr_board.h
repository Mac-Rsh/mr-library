/*
 * @copyright (c) 2023-2024, MR Development Team
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

#include "ch32v00x.h"

#define MR_USING_CH32V00X

#define DRV_ADC_CHANNEL_CONFIG \
    {                          \
        {ADC_Channel_0, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_2}, \
        {ADC_Channel_1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_1}, \
        {ADC_Channel_2, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_4}, \
        {ADC_Channel_3, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_2}, \
        {ADC_Channel_4, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_3}, \
        {ADC_Channel_5, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_5}, \
        {ADC_Channel_6, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_6}, \
        {ADC_Channel_7, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_4}, \
        {ADC_Channel_8, 0, NULL, 0},                              \
        {ADC_Channel_9, 0, NULL, 0},                              \
    }

#if (MR_CFG_I2C1_GROUP == 1)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_2, GPIOC, GPIO_Pin_1, I2C1_EV_IRQn, 0}
#elif (MR_CFG_I2C1_GROUP == 2)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_1, GPIOD, GPIO_Pin_0, I2C1_EV_IRQn, GPIO_PartialRemap_I2C1}
#elif (MR_CFG_I2C1_GROUP == 3)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_5, GPIOC, GPIO_Pin_6, I2C1_EV_IRQn, GPIO_FullRemap_I2C1}
#endif /* MR_CFG_I2C1_GROUP */

#define DRV_PIN_IRQ_MAP_CONFIG          \
    {                                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
        EXTI7_0_IRQn,                   \
    }

#define DRV_PIN_PORT_CONFIG             \
    {                                   \
        GPIOA,                          \
        MR_NULL,                        \
        GPIOC,                          \
        GPIOD,                          \
    }

#define DRV_PIN_CONFIG                  \
    {                                   \
        GPIO_Pin_0,                     \
        GPIO_Pin_1,                     \
        GPIO_Pin_2,                     \
        GPIO_Pin_3,                     \
        GPIO_Pin_4,                     \
        GPIO_Pin_5,                     \
        GPIO_Pin_6,                     \
        GPIO_Pin_7,                     \
    }

#if (MR_CFG_UART1_GROUP == 1)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_6, GPIOD, GPIO_Pin_5, USART1_IRQn, 0}
#elif (MR_CFG_UART1_GROUP == 2)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_1, GPIOD, GPIO_Pin_0, USART1_IRQn, GPIO_PartialRemap1_USART1)
#elif (MR_CFG_UART1_GROUP == 3)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_5, GPIOD, GPIO_Pin_6, USART1_IRQn, GPIO_PartialRemap2_USART1)
#elif (MR_CFG_UART1_GROUP == 4)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_1, GPIOC, GPIO_Pin_0, USART1_IRQn, GPIO_FullRemap_USART1)
#endif /* MR_CFG_UART1_GROUP */

#if (MR_CFG_SPI1_GROUP == 1)
#define DRV_SPI1_CONFIG                 \
    {SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_5, GPIOC, GPIO_Pin_7, GPIOC, GPIO_Pin_6, SPI1_IRQn, 0}
#endif /* MR_CFG_SPI2_GROUP */

#define DRV_TIMER1_CONFIG               \
    {TIM1, RCC_APB2Periph_TIM1, TIM1_UP_IRQn}
#define DRV_TIMER2_CONFIG               \
    {TIM2, RCC_APB1Periph_TIM2, TIM2_IRQn}

#define DRV_TIMER1_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER2_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
