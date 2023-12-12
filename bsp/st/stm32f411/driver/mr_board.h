/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-12    MacRsh       First version
 */

#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f4xx.h"

#define DRV_ADC_CHANNEL_CONFIG \
    {                          \
        {ADC_CHANNEL_0},       \
        {ADC_CHANNEL_1},       \
        {ADC_CHANNEL_2},       \
        {ADC_CHANNEL_3},       \
        {ADC_CHANNEL_4},       \
        {ADC_CHANNEL_5},       \
        {ADC_CHANNEL_6},       \
        {ADC_CHANNEL_7},       \
        {ADC_CHANNEL_8},       \
        {ADC_CHANNEL_9},       \
        {ADC_CHANNEL_10},      \
        {ADC_CHANNEL_11},      \
        {ADC_CHANNEL_12},      \
        {ADC_CHANNEL_13},      \
        {ADC_CHANNEL_14},      \
        {ADC_CHANNEL_15},      \
        {ADC_CHANNEL_16},      \
        {ADC_CHANNEL_17},      \
    }

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

#if !defined(GPIOD) && !defined(GPIOE)
#define DRV_PIN_PORT_CONFIG             \
    {                                   \
        GPIOA,                          \
        GPIOB,                          \
        GPIOC,                          \
    }
#elif defined(GPIOE)
#define DRV_PIN_PORT_CONFIG             \
    {                                   \
        GPIOA,                          \
        GPIOB,                          \
        GPIOC,                          \
        GPIOD,                          \
    }
#else
#define DRV_PIN_PORT_CONFIG             \
    {                                   \
        GPIOA,                          \
        GPIOB,                          \
        GPIOC,                          \
        GPIOD,                          \
        GPIOE,                          \
    }
#endif /* GPIOD */

#define DRV_PIN_CONFIG                  \
    {                                   \
        GPIO_PIN_0,                     \
        GPIO_PIN_1,                     \
        GPIO_PIN_2,                     \
        GPIO_PIN_3,                     \
        GPIO_PIN_4,                     \
        GPIO_PIN_5,                     \
        GPIO_PIN_6,                     \
        GPIO_PIN_7,                     \
        GPIO_PIN_8,                     \
        GPIO_PIN_9,                     \
        GPIO_PIN_10,                    \
        GPIO_PIN_11,                    \
        GPIO_PIN_12,                    \
        GPIO_PIN_13,                    \
        GPIO_PIN_14,                    \
        GPIO_PIN_15,                    \
    }

#define DRV_TIMER1_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER2_INFO_CONFIG          \
    {0, UINT16_MAX, UINT32_MAX}
#define DRV_TIMER3_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER4_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER5_INFO_CONFIG          \
    {0, UINT16_MAX, UINT32_MAX}
#define DRV_TIMER9_INFO_CONFIG          \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER10_INFO_CONFIG         \
    {0, UINT16_MAX, UINT16_MAX}
#define DRV_TIMER11_INFO_CONFIG         \
    {0, UINT16_MAX, UINT16_MAX}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
