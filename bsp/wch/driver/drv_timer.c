/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-17     MacRsh       first version
 */

#include "drv_timer.h"

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

enum
{
#ifdef MR_BSP_TIMER_1
    CH32_TIMER_1_INDEX,
#endif
#ifdef MR_BSP_TIMER_2
    CH32_TIMER_2_INDEX,
#endif
#ifdef MR_BSP_TIMER_3
    CH32_TIMER_3_INDEX,
#endif
#ifdef MR_BSP_TIMER_4
    CH32_TIMER_4_INDEX,
#endif
#ifdef MR_BSP_TIMER_5
    CH32_TIMER_5_INDEX,
#endif
#ifdef MR_BSP_TIMER_6
    CH32_TIMER_6_INDEX,
#endif
#ifdef MR_BSP_TIMER_7
    CH32_TIMER_7_INDEX,
#endif
#ifdef MR_BSP_TIMER_8
    CH32_TIMER_8_INDEX,
#endif
#ifdef MR_BSP_TIMER_9
    CH32_TIMER_9_INDEX,
#endif
#ifdef MR_BSP_TIMER_10
    CH32_TIMER_10_INDEX,
#endif
};

static struct ch32_timer_data ch32_timer_data[] =
    {
#ifdef MR_BSP_TIMER_1
        {"timer1", TIM1, RCC_APB2Periph_TIM1, TIM1_UP_IRQn},
#endif
#ifdef MR_BSP_TIMER_2
        {"timer2", TIM2, RCC_APB1Periph_TIM2, TIM2_IRQn},
#endif
#ifdef MR_BSP_TIMER_3
        {"timer3", TIM3, RCC_APB1Periph_TIM3, TIM3_IRQn},
#endif
#ifdef MR_BSP_TIMER_4
        {"timer4", TIM4, RCC_APB1Periph_TIM4, TIM4_IRQn},
#endif
#ifdef MR_BSP_TIMER_5
        {"timer5", TIM5, RCC_APB1Periph_TIM5, TIM5_IRQn},
#endif
#ifdef MR_BSP_TIMER_6
        {"timer6", TIM6, RCC_APB1Periph_TIM6, TIM6_IRQn},
#endif
#ifdef MR_BSP_TIMER_7
        {"timer7", TIM7, RCC_APB1Periph_TIM7, TIM7_IRQn},
#endif
#ifdef MR_BSP_TIMER_8
        {"timer8", TIM8, RCC_APB2Periph_TIM8, TIM8_UP_IRQn},
#endif
#ifdef MR_BSP_TIMER_9
        {"timer9", TIM9, RCC_APB2Periph_TIM9, TIM9_UP_IRQn},
#endif
#ifdef MR_BSP_TIMER_10
        {"timer10", TIM10, RCC_APB2Periph_TIM10, TIM10_UP_IRQn},
#endif
    };

static struct mr_timer_data timer_device_data = {10000000, 5000, 0xffff, MR_TIMER_COUNT_MODE_UP};

static struct mr_timer timer_device[mr_array_number_of(ch32_timer_data)];

static mr_err_t ch32_timer_configure(mr_timer_t timer, mr_timer_config_t config)
{
    struct ch32_timer_data *timer_data = (struct ch32_timer_data *)timer->device.data;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStructure = {0};
    mr_uint32_t pclk_freq = 0;

    RCC_GetClocksFreq(&RCC_ClockStructure);

    if ((uint32_t)timer_data->instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(timer_data->timer_periph_clock, ENABLE);
        if ((RCC->CFGR0 & RCC_PPRE2) == 0)
        {
            pclk_freq = RCC_ClockStructure.PCLK2_Frequency;
        } else
        {
            pclk_freq = 2 * RCC_ClockStructure.PCLK2_Frequency;
        }
    } else
    {
        RCC_APB1PeriphClockCmd(timer_data->timer_periph_clock, ENABLE);
        if ((RCC->CFGR0 & RCC_PPRE1) == 0)
        {
            pclk_freq = RCC_ClockStructure.PCLK1_Frequency;
        } else
        {
            pclk_freq = 2 * RCC_ClockStructure.PCLK1_Frequency;
        }
    }

    switch (timer->data->count_mode)
    {
        case MR_TIMER_COUNT_MODE_UP:
        {
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            break;
        }

        case MR_TIMER_COUNT_MODE_DOWN:
        {
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    switch (timer->data->count_max)
    {
        case 0xFFFF:
        {
            TIM_TimeBaseInitStructure.TIM_Prescaler = (pclk_freq / config->freq) - 1;
            break;
        }

        default:
            return -MR_ERR_INVALID;
    }

    NVIC_InitStructure.NVIC_IRQChannel = timer_data->irqno;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ClearITPendingBit(timer_data->instance, TIM_IT_Update);
    TIM_ITConfig(timer_data->instance, TIM_IT_Update, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timer_data->instance, &TIM_TimeBaseInitStructure);

    return MR_ERR_OK;
}

static void ch32_timer_start(mr_timer_t timer, mr_uint32_t period_reload)
{
    struct ch32_timer_data *timer_data = (struct ch32_timer_data *)timer->device.data;

    if (timer->data->count_max == MR_TIMER_COUNT_MODE_UP)
    {
        timer_data->instance->CNT = 0;
    } else
    {
        timer_data->instance->CNT = period_reload - 1;
    }
    timer_data->instance->ATRLR = period_reload - 1;
    TIM_Cmd(timer_data->instance, ENABLE);
}

static void ch32_timer_stop(mr_timer_t timer)
{
    struct ch32_timer_data *timer_data = (struct ch32_timer_data *)timer->device.data;

    TIM_Cmd(timer_data->instance, DISABLE);
}

static mr_uint32_t ch32_timer_get_count(mr_timer_t timer)
{
    struct ch32_timer_data *timer_data = (struct ch32_timer_data *)timer->device.data;

    return timer_data->instance->CNT;
}

static void ch32_timer_isr(mr_timer_t timer)
{
    struct ch32_timer_data *timer_data = (struct ch32_timer_data *)timer->device.data;

    if (TIM_GetITStatus(timer_data->instance, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(timer, MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(timer_data->instance, TIM_IT_Update);
    }
}

#ifdef MR_BSP_TIMER_1
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_1_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_2
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_2_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_3
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_3_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_4
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_4_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_5
void TIM5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM5_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_5_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_6
void TIM6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM6_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_6_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_7
void TIM7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM7_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_7_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_8
void TIM8_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM8_UP_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_8_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_9
void TIM9_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM9_UP_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_9_INDEX]);
}
#endif

#ifdef MR_BSP_TIMER_10
void TIM10_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM10_UP_IRQHandler(void)
{
    ch32_timer_isr(&timer_device[CH32_TIMER_10_INDEX]);
}
#endif

mr_err_t drv_timer_init(void)
{
    static struct mr_timer_ops drv_ops =
        {
            ch32_timer_configure,
            ch32_timer_start,
            ch32_timer_stop,
            ch32_timer_get_count,
        };
    mr_size_t count = mr_array_number_of(timer_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_timer_device_add(&timer_device[count],
                                  ch32_timer_data[count].name,
                                  &drv_ops,
                                  &timer_device_data,
                                  &ch32_timer_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
MR_INIT_DRIVER_EXPORT(drv_timer_init);

#endif