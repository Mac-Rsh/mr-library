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

#include "ch32v30x.h"

#define MR_USING_CH32V30X

#define DRV_ADC_CHANNEL_CONFIG \
    {                          \
        {ADC_Channel_0,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0}, \
        {ADC_Channel_1,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_1}, \
        {ADC_Channel_2,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_2}, \
        {ADC_Channel_3,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3}, \
        {ADC_Channel_4,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_4}, \
        {ADC_Channel_5,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5}, \
        {ADC_Channel_6,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_6}, \
        {ADC_Channel_7,  RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_7}, \
        {ADC_Channel_8,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_0}, \
        {ADC_Channel_9,  RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_1}, \
        {ADC_Channel_10, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_0}, \
        {ADC_Channel_11, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_1}, \
        {ADC_Channel_12, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_2}, \
        {ADC_Channel_13, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_3}, \
        {ADC_Channel_14, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_4}, \
        {ADC_Channel_15, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_5}, \
        {ADC_Channel_16, 0, NULL, 0},                              \
        {ADC_Channel_17, 0, NULL, 0},                              \
    }

#define DRV_DAC_CHANNEL_CONFIG          \
    {                                   \
        {0, 0, NULL, 0},                                          \
        {DAC_Channel_1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_4}, \
        {DAC_Channel_2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5}, \
    }

#if (MR_CFG_I2C1_GROUP == 1)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7, I2C1_EV_IRQn, 0}
#elif (MR_CFG_I2C1_GROUP == 2)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8, GPIOB, GPIO_Pin_9, I2C1_EV_IRQn, GPIO_Remap_I2C1}
#endif /* MR_CFG_I2C1_GROUP */
#if (MR_CFG_I2C2_GROUP == 1)
#define DRV_I2C2_CONFIG                 \
    {I2C2, RCC_APB1Periph_I2C2, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_10, GPIOB, GPIO_Pin_11, I2C2_EV_IRQn, 0}
#endif /* MR_CFG_I2C2_GROUP */

#define DRV_PIN_IRQ_MAP_CONFIG          \
    {                                   \
        EXTI0_IRQn,                     \
        EXTI1_IRQn,                     \
        EXTI2_IRQn,                     \
        EXTI3_IRQn,                     \
        EXTI4_IRQn,                     \
        EXTI9_5_IRQn,                   \
        EXTI9_5_IRQn,                   \
        EXTI9_5_IRQn,                   \
        EXTI9_5_IRQn,                   \
        EXTI9_5_IRQn,                   \
        EXTI15_10_IRQn,                 \
        EXTI15_10_IRQn,                 \
        EXTI15_10_IRQn,                 \
        EXTI15_10_IRQn,                 \
        EXTI15_10_IRQn,                 \
        EXTI15_10_IRQn,                 \
    }

#define DRV_PIN_PORT_CONFIG             \
    {                                   \
        GPIOA,                          \
        GPIOB,                          \
        GPIOC,                          \
        GPIOD,                          \
        GPIOE,                          \
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
        GPIO_Pin_8,                     \
        GPIO_Pin_9,                     \
        GPIO_Pin_10,                    \
        GPIO_Pin_11,                    \
        GPIO_Pin_12,                    \
        GPIO_Pin_13,                    \
        GPIO_Pin_14,                    \
        GPIO_Pin_15,                    \
    }

#if (MR_CFG_PWM1_GROUP == 1)
#define DRV_PWM1_CONFIG                 \
    {TIM1, RCC_APB2Periph_TIM1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_8, GPIOA, GPIO_Pin_9, GPIOA, GPIO_Pin_10, GPIOA, GPIO_Pin_11, 0}
#elif (MR_CFG_PWM1_GROUP == 2)
#define DRV_PWM1_CONFIG                 \
       {TIM1, RCC_APB2Periph_TIM1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_8, GPIOA, GPIO_Pin_9, GPIOA, GPIO_Pin_10, GPIOA, GPIO_Pin_11, GPIO_PartialRemap_TIM1}
#elif (MR_CFG_PWM1_GROUP == 3)
#define DRV_PWM1_CONFIG                 \
    {TIM1, RCC_APB2Periph_TIM1, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_9, GPIOE, GPIO_Pin_11, GPIOE, GPIO_Pin_13, GPIOE, GPIO_Pin_14, GPIO_FullRemap_TIM1}
#endif /* MR_CFG_PWM1_GROUP */
#if (MR_CFG_PWM2_GROUP == 1)
#define DRV_PWM2_CONFIG                 \
    {TIM2, RCC_APB1Periph_TIM2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIOA, GPIO_Pin_1, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, 0}
#elif (MR_CFG_PWM2_GROUP == 2)
#define DRV_PWM2_CONFIG                 \
    {TIM2, RCC_APB1Periph_TIM2, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_15, GPIOB, GPIO_Pin_3, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, GPIO_PartialRemap1_TIM2}
#elif (MR_CFG_PWM2_GROUP == 3)
#define DRV_PWM2_CONFIG                 \
    {TIM2, RCC_APB1Periph_TIM2, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_0, GPIOA, GPIO_Pin_1, GPIOB, GPIO_Pin_10, GPIOA, GPIO_Pin_3, GPIO_PartialRemap2_TIM2}
#elif (MR_CFG_PWM2_GROUP == 4)
#define DRV_PWM2_CONFIG                 \
    {TIM2, RCC_APB1Periph_TIM2, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_15, GPIOB, GPIO_Pin_3, GPIOB, GPIO_Pin_10, GPIOB, GPIO_Pin_11, GPIO_FullRemap_TIM2}
#endif /* MR_CFG_PWM2_GROUP */
#if (MR_CFG_PWM3_GROUP == 1)
#define DRV_PWM3_CONFIG                 \
    {TIM3, RCC_APB1Periph_TIM3, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_6, GPIOA, GPIO_Pin_7, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_1, 0}
#elif (MR_CFG_PWM3_GROUP == 2)
#define DRV_PWM3_CONFIG                 \
    {TIM3, RCC_APB1Periph_TIM3, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_4, GPIOB, GPIO_Pin_5, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_1, GPIO_PartialRemap_TIM3}
#elif (MR_CFG_PWM3_GROUP == 3)
#define DRV_PWM3_CONFIG                 \
    {TIM3, RCC_APB1Periph_TIM3, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_6, GPIOC, GPIO_Pin_7, GPIOC, GPIO_Pin_8, GPIOC, GPIO_Pin_9, GPIO_FullRemap_TIM3}
#endif /* MR_CFG_PWM3_GROUP */
#if (MR_CFG_PWM4_GROUP == 1)
#define DRV_PWM4_CONFIG                 \
    {TIM4, RCC_APB1Periph_TIM4, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7, GPIOB, GPIO_Pin_8, GPIOB, GPIO_Pin_9, 0}
#elif (MR_CFG_PWM4_GROUP == 2)
#define DRV_PWM4_CONFIG                 \
    {TIM4, RCC_APB1Periph_TIM4, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_12, GPIOD, GPIO_Pin_13, GPIOD, GPIO_Pin_14, GPIOD, GPIO_Pin_15, GPIO_Remap_TIM4}
#endif /* MR_CFG_PWM4_GROUP */
#if (MR_CFG_PWM5_GROUP == 1)
#define DRV_PWM5_CONFIG                 \
    {TIM5, RCC_APB1Periph_TIM5, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIOA, GPIO_Pin_1, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, 0}
#endif /* MR_CFG_PWM5_GROUP */
#if (MR_CFG_PWM8_GROUP == 1)
#define DRV_PWM8_CONFIG                 \
    {TIM8, RCC_APB2Periph_TIM8, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_6, GPIOC, GPIO_Pin_7, GPIOC, GPIO_Pin_8, GPIOC, GPIO_Pin_9, 0}
#elif (MR_CFG_PWM8_GROUP == 2)
#defined DRV_PWM8_CONFIG                 \
    {TIM8, RCC_APB2Periph_TIM8, RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7, GPIOB, GPIO_Pin_8, GPIOC, GPIO_Pin_13, GPIO_Remap_TIM8}
#endif /* MR_CFG_PWM8_GROUP */
#if (MR_CFG_PWM9_GROUP == 1)
#define DRV_PWM9_CONFIG                 \
    {TIM9, RCC_APB2Periph_TIM9, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, GPIOA, GPIO_Pin_4, GPIOC, GPIO_Pin_4, 0}
#elif (MR_CFG_PWM9_GROUP == 2)
#define DRV_PWM9_CONFIG                 \
    {TIM9, RCC_APB2Periph_TIM9, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, GPIOA, GPIO_Pin_4, GPIOC, GPIO_Pin_14, GPIO_PartialRemap_TIM9}
#elif (MR_CFG_PWM9_GROUP == 3)
#define DRV_PWM9_CONFIG                 \
    {TIM9, RCC_APB2Periph_TIM9, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_9, GPIOD, GPIO_Pin_11, GPIOD, GPIO_Pin_13, GPIOD, GPIO_Pin_15, GPIO_FullRemap_TIM9}
#endif /* MR_CFG_PWM9_GROUP */
#if (MR_CFG_PWM10_GROUP == 1)
#define DRV_PWM10_CONFIG                \
    {TIM10, RCC_APB2Periph_TIM10, RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, GPIOB, GPIO_Pin_8, GPIOB, GPIO_Pin_9, GPIOC, GPIO_Pin_3, GPIOC, GPIO_Pin_11, 0}
#elif (MR_CFG_PWM10_GROUP == 2)
#define DRV_PWM10_CONFIG                \
    {TIM10, RCC_APB2Periph_TIM10, RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, GPIOB, GPIO_Pin_3, GPIOB, GPIO_Pin_4, GPIOB, GPIO_Pin_5, GPIOC, GPIO_Pin_15, GPIO_PartialRemap_TIM10}
#elif (MR_CFG_PWM10_GROUP == 3)
#define DRV_PWM10_CONFIG                \
    {TIM10, RCC_APB2Periph_TIM10, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_1, GPIOD, GPIO_Pin_3, GPIOD, GPIO_Pin_5, GPIOD, GPIO_Pin_7, GPIO_FullRemap_TIM10}
#endif /* MR_CFG_PWM10_GROUP */

#define DRV_PWM1_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM2_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM3_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM4_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM5_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM8_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM9_INFO_CONFIG            \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_PWM10_INFO_CONFIG           \
    {0, UINT16_MAX, UINT16_MAX}

#if (MR_CFG_UART1_GROUP == 1)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_10, GPIOA, GPIO_Pin_9, USART1_IRQn, 0}
#elif (MR_CFG_UART1_GROUP == 2)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_7, GPIOB, GPIO_Pin_6, USART1_IRQn, GPIO_Remap_USART1}
#elif (MR_CFG_UART1_GROUP == 3)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_8, GPIOB, GPIO_Pin_15, USART1_IRQn, 0}
#elif (MR_CFG_UART1_GROUP == 4)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_7, GPIOA, GPIO_Pin_6, USART1_IRQn, GPIO_Remap_USART1}
#endif /* MR_CFG_UART1_GROUP */
#if (MR_CFG_UART2_GROUP == 1)
#define DRV_UART2_CONFIG                \
    {USART2, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3, GPIOA, GPIO_Pin_2, USART2_IRQn, 0}
#elif (MR_CFG_UART2_GROUP == 2)
#define DRV_UART2_CONFIG                \
    {USART2, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_6, GPIOD, GPIO_Pin_5, USART2_IRQn, GPIO_Remap_USART2}
#endif /* MR_CFG_UART2_GROUP */
#if (MR_CFG_UART3_GROUP == 1)
#define DRV_UART3_CONFIG                \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_11, GPIOB, GPIO_Pin_10, USART3_IRQn, 0}
#elif (MR_CFG_UART3_GROUP == 2)
#define DRV_UART3_CONFIG \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, GPIOC, GPIO_Pin_10, USART3_IRQn, GPIO_PartialRemap_USART3}
#elif (MR_CFG_UART3_GROUP == 3)
#define DRV_UART3_CONFIG \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_14, GPIOA, GPIO_Pin_13, USART3_IRQn, GPIO_PartialRemap1_USART3}
#elif (MR_CFG_UART3_GROUP == 4)
#define DRV_UART3_CONFIG \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_9, GPIOD, GPIO_Pin_8, USART3_IRQn, GPIO_FullRemap_USART3}
#endif /* MR_CFG_UART3_GROUP */
#if (MR_CFG_UART4_GROUP == 1)
#define DRV_UART4_CONFIG                \
    {UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, GPIOC, GPIO_Pin_10, UART4_IRQn, 0}
#elif (MR_CFG_UART4_GROUP == 2)
#define DRV_UART4_CONFIG                \
    {UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_1, GPIOB, GPIO_Pin_0, UART4_IRQn, GPIO_PartialRemap_USART4}
#elif (MR_CFG_UART4_GROUP == 3)
#define DRV_UART4_CONFIG                \
    {UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_1, GPIOE, GPIO_Pin_0, UART4_IRQn, GPIO_FullRemap_USART4}
#endif /* MR_CFG_UART4_GROUP */
#if (MR_CFG_UART5_GROUP == 1)
#define DRV_UART5_CONFIG                \
    {UART5, RCC_APB1Periph_UART5, RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_2, GPIOC, GPIO_Pin_12, UART5_IRQn, 0}
#elif (MR_CFG_UART5_GROUP == 2)
#define DRV_UART5_CONFIG                \
    {UART5, RCC_APB1Periph_UART5, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_5, GPIOB, GPIO_Pin_4, UART5_IRQn, GPIO_PartialRemap_USART5}
#elif (MR_CFG_UART5_GROUP == 3)
#define DRV_UART5_CONFIG                \
    {UART5, RCC_APB1Periph_UART5, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_9, GPIOE, GPIO_Pin_8, UART5_IRQn, GPIO_FullRemap_USART5}
#endif /* MR_CFG_UART5_GROUP */
#if (MR_CFG_UART6_GROUP == 1)
#define DRV_UART6_CONFIG                \
    {UART6, RCC_APB1Periph_UART6, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_1, GPIOC, GPIO_Pin_0, UART6_IRQn, 0}
#elif (MR_CFG_UART6_GROUP == 2)
#define DRV_UART6_CONFIG                \
    {UART6, RCC_APB1Periph_UART6, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_9, GPIOB, GPIO_Pin_8, UART6_IRQn, GPIO_PartialRemap_USART6}
#elif (MR_CFG_UART6_GROUP == 3)
#define DRV_UART6_CONFIG                \
    {UART6, RCC_APB1Periph_UART6, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_11, GPIOE, GPIO_Pin_10, UART6_IRQn, GPIO_FullRemap_USART6}
#endif /* MR_CFG_UART6_GROUP */
#if (MR_CFG_UART7_GROUP == 1)
#define DRV_UART7_CONFIG                \
    {UART7, RCC_APB1Periph_UART7, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_3, GPIOC, GPIO_Pin_2, UART7_IRQn, 0}
#elif (MR_CFG_UART7_GROUP == 2)
#define DRV_UART7_CONFIG                \
    {UART7, RCC_APB1Periph_UART7, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_7, GPIOA, GPIO_Pin_6, UART7_IRQn, GPIO_PartialRemap_USART7}
#elif (MR_CFG_UART7_GROUP == 3)
#define DRV_UART7_CONFIG                \
    {UART7, RCC_APB1Periph_UART7, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_13, GPIOE, GPIO_Pin_12, UART7_IRQn, GPIO_FullRemap_USART7}
#endif /* MR_CFG_UART7_GROUP */
#if (MR_CFG_UART8_GROUP == 1)
#define DRV_UART8_CONFIG                \
    {UART8, RCC_APB1Periph_UART8, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_5, GPIOC, GPIO_Pin_4, UART8_IRQn, 0}
#elif (MR_CFG_UART8_GROUP == 2)
#define DRV_UART8_CONFIG                \
    {UART8, RCC_APB1Periph_UART8, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_15, GPIOA, GPIO_Pin_14, UART8_IRQn, GPIO_PartialRemap_USART8}
#elif (MR_CFG_UART8_GROUP == 3)
#define DRV_UART8_CONFIG                \
    {UART8, RCC_APB1Periph_UART8, RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_15, GPIOE, GPIO_Pin_14, UART8_IRQn, GPIO_FullRemap_USART8}
#endif /* MR_CFG_UART8_GROUP */

#if (MR_CFG_SPI1_GROUP == 1)
#define DRV_SPI1_CONFIG                 \
    {SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5, GPIOA, GPIO_Pin_6, GPIOA, GPIO_Pin_7, SPI1_IRQn, 0}
#elif (MR_CFG_SPI1_GROUP == 2)
#define DRV_SPI1_CONFIG                 \
    {SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_3, GPIOB, GPIO_Pin_4, GPIOB, GPIO_Pin_5, SPI1_IRQn, GPIO_Remap_SPI1}
#endif /* MR_CFG_SPI1_GROUP */
#define DRV_SPI2_CONFIG                 \
    {SPI2, RCC_APB1Periph_SPI2, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13, GPIOB, GPIO_Pin_14, GPIOB, GPIO_Pin_15, SPI2_IRQn, 0}
#if (MR_CFG_SPI3_GROUP == 1)
#define DRV_SPI3_CONFIG                 \
    {SPI3, RCC_APB1Periph_SPI3, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_3, GPIOB, GPIO_Pin_4, GPIOB, GPIO_Pin_5, SPI3_IRQn, 0}
#elif (MR_CFG_SPI3_GROUP == 2)
#define DRV_SPI3_CONFIG                 \
    {SPI3, RCC_APB1Periph_SPI3, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_10, GPIOC, GPIO_Pin_11, GPIOC, GPIO_Pin_12, SPI3_IRQn, GPIO_Remap_SPI3}
#endif /* MR_CFG_SPI3_GROUP */

#define DRV_TIMER1_CONFIG               \
    {TIM1, RCC_APB2Periph_TIM1, TIM1_UP_IRQn}
#define DRV_TIMER2_CONFIG               \
    {TIM2, RCC_APB1Periph_TIM2, TIM2_IRQn}
#define DRV_TIMER3_CONFIG               \
    {TIM3, RCC_APB1Periph_TIM3, TIM3_IRQn}
#define DRV_TIMER4_CONFIG               \
    {TIM4, RCC_APB1Periph_TIM4, TIM4_IRQn}
#define DRV_TIMER5_CONFIG               \
    {TIM5, RCC_APB1Periph_TIM5, TIM5_IRQn}
#define DRV_TIMER6_CONFIG               \
    {TIM6, RCC_APB1Periph_TIM6, TIM6_IRQn}
#define DRV_TIMER7_CONFIG               \
    {TIM7, RCC_APB1Periph_TIM7, TIM7_IRQn}
#define DRV_TIMER8_CONFIG               \
    {TIM8, RCC_APB2Periph_TIM8, TIM8_UP_IRQn}
#define DRV_TIMER9_CONFIG               \
    {TIM9, RCC_APB2Periph_TIM9, TIM9_UP_IRQn}
#define DRV_TIMER10_CONFIG              \
    {TIM10, RCC_APB2Periph_TIM10, TIM10_UP_IRQn}

#define DRV_TIMER1_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER2_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER3_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER4_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER5_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER6_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER7_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER8_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER9_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER10_INFO_CONFIG         \
    {0, UINT16_MAX, UINT16_MAX}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
