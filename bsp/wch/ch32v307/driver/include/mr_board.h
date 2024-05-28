/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

#include "../mr-library/include/mr_config.h"
#include "ch32v30x.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MR_USE_CH32V30X

#ifdef MR_USE_ADC1
#define _DRIVER_ADC1_CONFIG                                                    \
    {                                                                          \
        .instance = ADC1, .clock = RCC_APB2Periph_ADC1                         \
    }
#endif /* MR_USE_ADC1 */
#ifdef MR_USE_ADC2
#define _DRIVER_ADC2_CONFIG                                                    \
    {                                                                          \
        .instance = ADC2, .clock = RCC_APB2Periph_ADC2                         \
    }
#endif /* MR_USE_ADC2 */

#if defined(MR_USE_ADC1) || defined(MR_USE_ADC2)
#define _DRIVER_ADC_CHANNEL_CONFIG                                             \
    {                                                                          \
        {.channel = ADC_Channel_0,                                             \
         .gpio_clock = RCC_APB2Periph_GPIOA,                                   \
         .port = GPIOA,                                                        \
         .pin = GPIO_Pin_0},                                                   \
            {.channel = ADC_Channel_1,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_1},                                               \
            {.channel = ADC_Channel_2,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_2},                                               \
            {.channel = ADC_Channel_3,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_3},                                               \
            {.channel = ADC_Channel_4,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_4},                                               \
            {.channel = ADC_Channel_5,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_5},                                               \
            {.channel = ADC_Channel_6,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_6},                                               \
            {.channel = ADC_Channel_7,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOA,                               \
             .port = GPIOA,                                                    \
             .pin = GPIO_Pin_7},                                               \
            {.channel = ADC_Channel_8,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOB,                               \
             .port = GPIOB,                                                    \
             .pin = GPIO_Pin_0},                                               \
            {.channel = ADC_Channel_9,                                         \
             .gpio_clock = RCC_APB2Periph_GPIOB,                               \
             .port = GPIOB,                                                    \
             .pin = GPIO_Pin_1},                                               \
            {.channel = ADC_Channel_10,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_0},                                               \
            {.channel = ADC_Channel_11,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_1},                                               \
            {.channel = ADC_Channel_12,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_2},                                               \
            {.channel = ADC_Channel_13,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_3},                                               \
            {.channel = ADC_Channel_14,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_4},                                               \
            {.channel = ADC_Channel_15,                                        \
             .gpio_clock = RCC_APB2Periph_GPIOC,                               \
             .port = GPIOC,                                                    \
             .pin = GPIO_Pin_5},                                               \
            {.channel = ADC_Channel_16,                                        \
             .gpio_clock = 0,                                                  \
             .port = NULL,                                                     \
             .pin = 0},                                                        \
            {.channel = ADC_Channel_17,                                        \
             .gpio_clock = 0,                                                  \
             .port = NULL,                                                     \
             .pin = 0},                                                        \
    }
#define _DRIVER_ADC_CHANNELS (0x3ffff)
#define _DRIVER_ADC_RESOLUTION (12)
#endif /* defined(MR_USE_ADC1) || defined(MR_USE_ADC2) */

#define _DRIVER_PIN_IRQ_CONFIG                                                 \
    {                                                                          \
        EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn,            \
            EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn,            \
            EXTI9_5_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn,      \
            EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn,                    \
    }

#define _DRIVER_PIN_PORT_CONFIG                                                \
    {                                                                          \
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,                                     \
    }

#define _DRIVER_PIN_PIN_CONFIG                                                 \
    {                                                                          \
        GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4,            \
            GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9,        \
            GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14,   \
            GPIO_Pin_15,                                                       \
    }

#if (MR_CFG_SERIAL1_GROUP == 1)
#define _DRIVER_SERIAL1_CONFIG                                                 \
    {                                                                          \
        .instance = USART1, .clock = RCC_APB2Periph_USART1,                    \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_10, .tx_port = GPIOA, .tx_pin = GPIO_Pin_9,         \
        .irq = USART1_IRQn, .remap = 0, .rx_dma_channel = DMA1_Channel5,       \
        .rx_dma_clock = RCC_AHBPeriph_DMA1, .rx_dma_irq = DMA1_Channel5_IRQn,  \
        .rx_dma_irq_it = DMA1_IT_TC5                                           \
    }
#elif (MR_CFG_SERIAL1_GROUP == 2)
#define _DRIVER_SERIAL1_CONFIG                                                 \
    {                                                                          \
        .instance = USART1, .clock = RCC_APB2Periph_USART1,                    \
        .gpio_clock = RCC_APB2Periph_GPIOB, .rx_port = GPIOB,                  \
        .rx_pin = GPIO_Pin_7, .tx_port = GPIOB, .tx_pin = GPIO_Pin_6,          \
        .irq = USART1_IRQn, .remap = GPIO_Remap_USART1,                        \
        .rx_dma_channel = DMA1_Channel5, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel5_IRQn, .rx_dma_irq_it = DMA1_IT_TC5         \
    }
#elif (MR_CFG_SERIAL1_GROUP == 3)
#define _DRIVER_SERIAL1_CONFIG                                                 \
    {                                                                          \
        .instance = USART1, .clock = RCC_APB2Periph_USART1,                    \
        .gpio_clock = RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,             \
        .rx_port = GPIOA, .rx_pin = GPIO_Pin_8, .tx_port = GPIOB,              \
        .tx_pin = GPIO_Pin_15, .irq = USART1_IRQn, .remap = 0,                 \
        .rx_dma_channel = DMA1_Channel5, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel5_IRQn, .rx_dma_irq_it = DMA1_IT_TC5         \
    }
#elif (MR_CFG_SERIAL1_GROUP == 4)
#define _DRIVER_SERIAL1_CONFIG                                                 \
    {                                                                          \
        .instance = USART1, .clock = RCC_APB2Periph_USART1,                    \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_7, .tx_port = GPIOA, .tx_pin = GPIO_Pin_6,          \
        .irq = USART1_IRQn, .remap = GPIO_Remap_USART1,                        \
        .rx_dma_channel = DMA1_Channel5, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel5_IRQn, .rx_dma_irq_it = DMA1_IT_TC5         \
    }
#endif /* MR_CFG_SERIAL1_GROUP */

#if (MR_CFG_SERIAL2_GROUP == 1)
#define _DRIVER_SERIAL2_CONFIG                                                 \
    {                                                                          \
        .instance = USART2, .clock = RCC_APB1Periph_USART2,                    \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_3, .tx_port = GPIOA, .tx_pin = GPIO_Pin_2,          \
        .irq = USART2_IRQn, .remap = 0, .rx_dma_channel = DMA1_Channel6,       \
        .rx_dma_clock = RCC_AHBPeriph_DMA1, .rx_dma_irq = DMA1_Channel6_IRQn,  \
        .rx_dma_irq_it = DMA1_IT_TC6                                           \
    }
#elif (MR_CFG_SERIAL2_GROUP == 2)
#define _DRIVER_SERIAL2_CONFIG                                                 \
    {                                                                          \
        .instance = USART2, .clock = RCC_APB1Periph_USART2,                    \
        .gpio_clock = RCC_APB2Periph_GPIOD, .rx_port = GPIOD,                  \
        .rx_pin = GPIO_Pin_6, .tx_port = GPIOD, .tx_pin = GPIO_Pin_5,          \
        .irq = USART2_IRQn, .remap = GPIO_Remap_USART2,                        \
        .rx_dma_channel = DMA1_Channel6, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel6_IRQn, .rx_dma_irq_it = DMA1_IT_TC6         \
    }
#endif /* MR_CFG_SERIAL2_GROUP */

#if (MR_CFG_SERIAL3_GROUP == 1)
#define _DRIVER_SERIAL3_CONFIG                                                 \
    {                                                                          \
        .instance = USART3, .clock = RCC_APB1Periph_USART3,                    \
        .gpio_clock = RCC_APB2Periph_GPIOB, .rx_port = GPIOB,                  \
        .rx_pin = GPIO_Pin_11, .tx_port = GPIOB, .tx_pin = GPIO_Pin_10,        \
        .irq = USART3_IRQn, .remap = 0, .rx_dma_channel = DMA1_Channel3,       \
        .rx_dma_clock = RCC_AHBPeriph_DMA1, .rx_dma_irq = DMA1_Channel3_IRQn,  \
        .rx_dma_irq_it = DMA1_IT_TC3                                           \
    }
#elif (MR_CFG_SERIAL3_GROUP == 2)
#define _DRIVER_SERIAL3_CONFIG                                                 \
    {                                                                          \
        .instance = USART3, .clock = RCC_APB1Periph_USART3,                    \
        .gpio_clock = RCC_APB2Periph_GPIOC, .rx_port = GPIOC,                  \
        .rx_pin = GPIO_Pin_11, .tx_port = GPIOC, .tx_pin = GPIO_Pin_10,        \
        .irq = USART3_IRQn, .remap = GPIO_PartialRemap_USART3,                 \
        .rx_dma_channel = DMA1_Channel3, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel3_IRQn, .rx_dma_irq_it = DMA1_IT_TC3         \
    }
#elif (MR_CFG_SERIAL3_GROUP == 3)
#define _DRIVER_SERIAL3_CONFIG                                                 \
    {                                                                          \
        .instance = USART3, .clock = RCC_APB1Periph_USART3,                    \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_14, .tx_port = GPIOA, .tx_pin = GPIO_Pin_13,        \
        .irq = USART3_IRQn, .remap = GPIO_PartialRemap1_USART3,                \
        .rx_dma_channel = DMA1_Channel3, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel3_IRQn, .rx_dma_irq_it = DMA1_IT_TC3         \
    }
#elif (MR_CFG_SERIAL3_GROUP == 4)
#define _DRIVER_SERIAL3_CONFIG                                                 \
    {                                                                          \
        .instance = USART3, .clock = RCC_APB1Periph_USART3,                    \
        .gpio_clock = RCC_APB2Periph_GPIOD, .rx_port = GPIOD,                  \
        .rx_pin = GPIO_Pin_9, .tx_port = GPIOD, .tx_pin = GPIO_Pin_8,          \
        .irq = USART3_IRQn, .remap = GPIO_FullRemap_USART3,                    \
        .rx_dma_channel = DMA1_Channel3, .rx_dma_clock = RCC_AHBPeriph_DMA1,   \
        .rx_dma_irq = DMA1_Channel3_IRQn, .rx_dma_irq_it = DMA1_IT_TC3         \
    }
#endif /* MR_CFG_SERIAL3_GROUP */

#if (MR_CFG_SERIAL4_GROUP == 1)
#define _DRIVER_SERIAL4_CONFIG                                                 \
    {                                                                          \
        .nstance = UART4, .clock = RCC_APB1Periph_UART4,                       \
        .pio_clock = RCC_APB2Periph_GPIOC, .rx_port = GPIOC,                   \
        .rx_pin = GPIO_Pin_11, .x_port = GPIOC, .tx_pin = GPIO_Pin_10,         \
        .irq = UART4_IRQn, .remap = 0, .rx_dma_channel = DMA2_Channel3,        \
        .rx_dma_clock = RCC_AHBPeriph_DMA2, .rx_dma_irq = DMA2_Channel3_IRQn,  \
        .rx_dma_irq_it = DMA2_IT_TC3                                           \
    }
#elif (MR_CFG_SERIAL4_GROUP == 2)
#define _DRIVER_SERIAL4_CONFIG                                                 \
    {                                                                          \
        .instance = UART4, .clock = RCC_APB1Periph_UART4,                      \
        .gpio_clock = RCC_APB2Periph_GPIOB, .rx_port = GPIOB,                  \
        .rx_pin = GPIO_Pin_1, .tx_port = GPIOB, .tx_pin = GPIO_Pin_0,          \
        .irq = UART4_IRQn, .remap = GPIO_PartialRemap_USART4,                  \
        .rx_dma_channel = DMA2_Channel3, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel3_IRQn, .rx_dma_irq_it = DMA2_IT_TC3         \
    }
#elif (MR_CFG_SERIAL4_GROUP == 3)
#define _DRIVER_SERIAL4_CONFIG                                                 \
    {                                                                          \
        .instance = UART4, .clock = RCC_APB1Periph_UART4,                      \
        .gpio_clock = RCC_APB2Periph_GPIOE, .rx_port = GPIOE,                  \
        .rx_pin = GPIO_Pin_1, .tx_port = GPIOE, .tx_pin = GPIO_Pin_0,          \
        .irq = UART4_IRQn, .remap = GPIO_FullRemap_USART4,                     \
        .rx_dma_channel = DMA2_Channel3, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel3_IRQn, .rx_dma_irq_it = DMA2_IT_TC3         \
    }
#endif /* MR_CFG_SERIAL4_GROUP */

#if (MR_CFG_SERIAL5_GROUP == 1)
#define _DRIVER_SERIAL5_CONFIG                                                 \
    {                                                                          \
        .instance = UART5, .clock = RCC_APB1Periph_UART5,                      \
        .gpio_clock = RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,             \
        .rx_port = GPIOD, .rx_pin = GPIO_Pin_2, .tx_port = GPIOC,              \
        .tx_pin = GPIO_Pin_12, .irq = UART5_IRQn, .remap = 0,                  \
        .rx_dma_channel = DMA2_Channel2, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel2_IRQn, .rx_dma_irq_it = DMA2_IT_TC2         \
    }
#elif (MR_CFG_SERIAL5_GROUP == 2)
#define _DRIVER_SERIAL5_CONFIG                                                 \
    {                                                                          \
        .instance = UART5, .clock = RCC_APB1Periph_UART5,                      \
        .gpio_clock = RCC_APB2Periph_GPIOB, .rx_port = GPIOB,                  \
        .rx_pin = GPIO_Pin_5, .tx_port = GPIOB, .tx_pin = GPIO_Pin_4,          \
        .irq = UART5_IRQn, .remap = GPIO_PartialRemap_USART5,                  \
        .rx_dma_channel = DMA2_Channel2, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel2_IRQn, .rx_dma_irq_it = DMA2_IT_TC2         \
    }
#elif (MR_CFG_SERIAL5_GROUP == 3)
#define _DRIVER_SERIAL5_CONFIG                                                 \
    {                                                                          \
        .instance = UART5, .clock = RCC_APB1Periph_UART5,                      \
        .gpio_clock = RCC_APB2Periph_GPIOE, .rx_port = GPIOE,                  \
        .rx_pin = GPIO_Pin_9, .tx_port = GPIOE, .tx_pin = GPIO_Pin_8,          \
        .irq = UART5_IRQn, .remap = GPIO_FullRemap_USART5,                     \
        .rx_dma_channel = DMA2_Channel2, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel2_IRQn, .rx_dma_irq_it = DMA2_IT_TC2         \
    }
#endif /* MR_CFG_SERIAL5_GROUP */

#if (MR_CFG_SERIAL6_GROUP == 1)
#define _DRIVER_SERIAL6_CONFIG                                                 \
    {                                                                          \
        .instance = UART6, .clock = RCC_APB1Periph_UART6,                      \
        .gpio_clock = RCC_APB2Periph_GPIOC, .rx_port = GPIOC,                  \
        .rx_pin = GPIO_Pin_1, .tx_port = GPIOC, .tx_pin = GPIO_Pin_0,          \
        .irq = UART6_IRQn, .remap = 0, .rx_dma_channel = DMA2_Channel7,        \
        .rx_dma_clock = RCC_AHBPeriph_DMA2, .rx_dma_irq = DMA2_Channel7_IRQn,  \
        .rx_dma_irq_it = DMA2_IT_TC7                                           \
    }
#elif (MR_CFG_SERIAL6_GROUP == 2)
#define _DRIVER_SERIAL6_CONFIG                                                 \
    {                                                                          \
        .instance = UART6, .clock = RCC_APB1Periph_UART6,                      \
        .gpio_clock = RCC_APB2Periph_GPIOB, .rx_port = GPIOB,                  \
        .rx_pin = GPIO_Pin_9, .tx_port = GPIOB, .tx_pin = GPIO_Pin_8,          \
        .irq = UART6_IRQn, .remap = GPIO_PartialRemap_USART6,                  \
        .rx_dma_channel = DMA2_Channel7, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel7_IRQn, .rx_dma_irq_it = DMA2_IT_TC7         \
    }
#elif (MR_CFG_SERIAL6_GROUP == 3)
#define _DRIVER_SERIAL6_CONFIG                                                 \
    {                                                                          \
        .instance = UART6, .clock = RCC_APB1Periph_UART6,                      \
        .gpio_clock = RCC_APB2Periph_GPIOE, .rx_port = GPIOE,                  \
        .rx_pin = GPIO_Pin_11, .tx_port = GPIOE, .tx_pin = GPIO_Pin_10,        \
        .irq = UART6_IRQn, .remap = GPIO_FullRemap_USART6,                     \
        .rx_dma_channel = DMA2_Channel7, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel7_IRQn, .rx_dma_irq_it = DMA2_IT_TC7         \
    }
#endif /* MR_CFG_SERIAL6_GROUP */

#if (MR_CFG_SERIAL7_GROUP == 1)
#define _DRIVER_SERIAL7_CONFIG                                                 \
    {                                                                          \
        .instance = UART7, .clock = RCC_APB1Periph_UART7,                      \
        .gpio_clock = RCC_APB2Periph_GPIOC, .rx_port = GPIOC,                  \
        .rx_pin = GPIO_Pin_3, .tx_port = GPIOC, .tx_pin = GPIO_Pin_2,          \
        .irq = UART7_IRQn, .remap = 0, .rx_dma_channel = DMA2_Channel9,        \
        .rx_dma_clock = RCC_AHBPeriph_DMA2, .rx_dma_irq = DMA2_Channel9_IRQn,  \
        .rx_dma_irq_it = DMA2_IT_TC9                                           \
    }
#elif (MR_CFG_SERIAL7_GROUP == 2)
#define _DRIVER_SERIAL7_CONFIG                                                 \
    {                                                                          \
        .instance = UART7, .clock = RCC_APB1Periph_UART7,                      \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_7, .tx_port = GPIOA, .tx_pin = GPIO_Pin_6,          \
        .irq = UART7_IRQn, .remap = GPIO_PartialRemap_USART7,                  \
        .rx_dma_channel = DMA2_Channel9, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel9_IRQn, .rx_dma_irq_it = DMA2_IT_TC9         \
    }
#elif (MR_CFG_SERIAL7_GROUP == 3)
#define _DRIVER_SERIAL7_CONFIG                                                 \
    {                                                                          \
        .instance = UART7, .clock = RCC_APB1Periph_UART7,                      \
        .gpio_clock = RCC_APB2Periph_GPIOE, .rx_port = GPIOE,                  \
        .rx_pin = GPIO_Pin_13, .tx_port = GPIOE, .tx_pin = GPIO_Pin_12,        \
        .irq = UART7_IRQn, .remap = GPIO_FullRemap_USART7,                     \
        .rx_dma_channel = DMA2_Channel9, .rx_dma_clock = RCC_AHBPeriph_DMA2,   \
        .rx_dma_irq = DMA2_Channel9_IRQn, .rx_dma_irq_it = DMA2_IT_TC9         \
    }
#endif /* MR_CFG_SERIAL7_GROUP */

#if (MR_CFG_SERIAL8_GROUP == 1)
#define _DRIVER_SERIAL8_CONFIG                                                 \
    {                                                                          \
        .instance = UART8, .clock = RCC_APB1Periph_UART8,                      \
        .gpio_clock = RCC_APB2Periph_GPIOC, .rx_port = GPIOC,                  \
        .rx_pin = GPIO_Pin_5, .tx_port = GPIOC, .tx_pin = GPIO_Pin_4,          \
        .irq = UART8_IRQn, .remap = 0, .rx_dma_channel = DMA2_Channel11,       \
        .rx_dma_clock = RCC_AHBPeriph_DMA2, .rx_dma_irq = DMA2_Channel11_IRQn, \
        .rx_dma_irq_it = DMA2_IT_TC11                                          \
    }
#elif (MR_CFG_SERIAL8_GROUP == 2)
#define _DRIVER_SERIAL8_CONFIG                                                 \
    {                                                                          \
        .instance = UART8, .clock = RCC_APB1Periph_UART8,                      \
        .gpio_clock = RCC_APB2Periph_GPIOA, .rx_port = GPIOA,                  \
        .rx_pin = GPIO_Pin_15, .tx_port = GPIOA, .tx_pin = GPIO_Pin_14,        \
        .irq = UART8_IRQn, .remap = GPIO_PartialRemap_USART8,                  \
        .rx_dma_channel = DMA2_Channel11, .rx_dma_clock = RCC_AHBPeriph_DMA2,  \
        .rx_dma_irq = DMA2_Channel11_IRQn, .rx_dma_irq_it = DMA2_IT_TC11       \
    }
#elif (MR_CFG_SERIAL8_GROUP == 3)
#define _DRIVER_SERIAL8_CONFIG                                                 \
    {                                                                          \
        .instance = UART8, .clock = RCC_APB1Periph_UART8,                      \
        .gpio_clock = RCC_APB2Periph_GPIOE, .rx_port = GPIOE,                  \
        .rx_pin = GPIO_Pin_15, .tx_port = GPIOE, .tx_pin = GPIO_Pin_14,        \
        .irq = UART8_IRQn, .remap = GPIO_FullRemap_USART8,                     \
        .rx_dma_channel = DMA2_Channel11, .rx_dma_clock = RCC_AHBPeriph_DMA2,  \
        .rx_dma_irq = DMA2_Channel11_IRQn, .rx_dma_irq_it = DMA2_IT_TC11       \
    }
#endif /* MR_CFG_SERIAL8_GROUP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
