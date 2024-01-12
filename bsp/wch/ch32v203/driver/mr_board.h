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

#include "ch32v20x.h"

#define MR_USING_CH32V20X

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

#if (MR_CFG_I2C1_GROUP == 1)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7, I2C1_EV_IRQn, 0}
#elif (MR_CFG_I2C1_GROUP == 2)
#define DRV_I2C1_CONFIG                 \
    {I2C1, RCC_APB1Periph_I2C1, RCC_APB2Periph_GPIOD, GPIOB, GPIO_Pin_8, GPIOB, GPIO_Pin_9, I2C1_EV_IRQn, GPIO_Remap_I2C1}
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
#endif /* MR_CFG_PWM4_GROUP */
#if (MR_CFG_PWM5_GROUP == 1)
#define DRV_PWM5_CONFIG                 \
    {TIM5, RCC_APB1Periph_TIM5, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIOA, GPIO_Pin_1, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3, 0}
#endif /* MR_CFG_PWM5_GROUP */

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


#if (MR_CFG_UART1_GROUP == 1)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_10, GPIOA, GPIO_Pin_9, USART1_IRQn, 0}
#elif (MR_CFG_UART1_GROUP == 2)
#define DRV_UART1_CONFIG                \
    {USART1, RCC_APB2Periph_USART1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_7, GPIOB, GPIO_Pin_6, USART1_IRQn, GPIO_Remap_USART1}
#endif /* MR_CFG_UART1_GROUP */
#if (MR_CFG_UART2_GROUP == 1)
#define DRV_UART2_CONFIG                \
    {USART2, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_3, GPIOA, GPIO_Pin_2, USART2_IRQn, 0}
#endif /* MR_CFG_UART2_GROUP */
#if (MR_CFG_UART3_GROUP == 1)
#define DRV_UART3_CONFIG                \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_11, GPIOB, GPIO_Pin_10, USART3_IRQn, 0}
#elif (MR_CFG_UART3_GROUP == 2)
#define DRV_UART3_CONFIG                \
    {USART3, RCC_APB1Periph_USART3, RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_11, GPIOC, GPIO_Pin_10, USART3_IRQn, GPIO_PartialRemap_USART3}
#endif /* MR_CFG_UART3_GROUP */
#if (MR_CFG_UART4_GROUP == 1)
#define DRV_UART4_CONFIG                \
    {UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_1, GPIOB, GPIO_Pin_0, UART4_IRQn, 0}
#elif (MR_CFG_UART4_GROUP == 2)
#define DRV_UART4_CONFIG                \
    {UART4, RCC_APB1Periph_UART4, RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_5, GPIOA, GPIO_Pin_5, UART4_IRQn, GPIO_PartialRemap_USART4}
#endif /* MR_CFG_UART4_GROUP */

#if (MR_CFG_SPI1_GROUP == 1)
#define DRV_SPI1_CONFIG                 \
    {SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5, GPIOA, GPIO_Pin_6, GPIOA, GPIO_Pin_7, SPI1_IRQn, 0}
#elif (MR_CFG_SPI1_GROUP == 2)
#define DRV_SPI1_CONFIG                 \
    {SPI1, RCC_APB2Periph_SPI1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_3, GPIOB, GPIO_Pin_4, GPIOB, GPIO_Pin_5, SPI1_IRQn, GPIO_Remap_SPI1}
#endif /* MR_CFG_SPI1_GROUP */
#define DRV_SPI2_CONFIG                 \
    {SPI2, RCC_APB1Periph_SPI2, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13, GPIOB, GPIO_Pin_14, GPIOB, GPIO_Pin_15, SPI2_IRQn, 0}

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
