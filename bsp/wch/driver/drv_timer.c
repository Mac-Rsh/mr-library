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

#if (MR_CONF_TIMER == MR_CONF_ENABLE)

enum
{
#ifdef BSP_TIMER_1
    TIMER1_INDEX,
#endif
#ifdef BSP_TIMER_2
    TIMER2_INDEX,
#endif
#ifdef BSP_TIMER_3
    TIMER3_INDEX,
#endif
#ifdef BSP_TIMER_4
    TIMER4_INDEX,
#endif
#ifdef BSP_TIMER_5
    TIMER5_INDEX,
#endif
#ifdef BSP_TIMER_6
    TIMER6_INDEX,
#endif
#ifdef BSP_TIMER_7
    TIMER7_INDEX,
#endif
#ifdef BSP_TIMER_8
    TIMER8_INDEX,
#endif
#ifdef BSP_TIMER_9
    TIMER9_INDEX,
#endif
#ifdef BSP_TIMER_10
    TIMER10_INDEX,
#endif
};

static struct ch32_timer ch32_timer[] =
        {
#ifdef BSP_TIMER_1
                {"timer1", {TIM1, RCC_APB2Periph_TIM1, TIM1_UP_IRQn}},
#endif
#ifdef BSP_TIMER_2
                {"timer2", {TIM2, RCC_APB1Periph_TIM2, TIM2_IRQn}},
#endif
#ifdef BSP_TIMER_3
                {"timer3", {TIM3, RCC_APB1Periph_TIM3, TIM3_IRQn}},
#endif
#ifdef BSP_TIMER_4
                {"timer4", {TIM4, RCC_APB1Periph_TIM4, TIM4_IRQn}},
#endif
#ifdef BSP_TIMER_5
                {"timer5", {TIM5, RCC_APB1Periph_TIM5, TIM5_IRQn}},
#endif
#ifdef BSP_TIMER_6
                {"timer6", {TIM6, RCC_APB1Periph_TIM6, TIM6_IRQn}},
#endif
#ifdef BSP_TIMER_7
                {"timer7", {TIM7, RCC_APB1Periph_TIM7, TIM7_IRQn}},
#endif
#ifdef BSP_TIMER_8
                {"timer8", {TIM8, RCC_APB2Periph_TIM8, TIM8_UP_IRQn}},
#endif
#ifdef BSP_TIMER_9
                {"timer9", {TIM9, RCC_APB2Periph_TIM9, TIM9_UP_IRQn}},
#endif
#ifdef BSP_TIMER_10
                {"timer10", {TIM10, RCC_APB2Periph_TIM10, TIM10_UP_IRQn}},
#endif
        };

static struct mr_timer_info timer_device_info = {10000000, 5000, 0xffff, _MR_TIMER_CNT_MODE_UP};

static struct mr_timer timer_device[mr_array_get_length(ch32_timer)];

static mr_err_t ch32_timer_configure(mr_timer_t timer, struct mr_timer_config *config)
{
    struct ch32_timer *driver = (struct ch32_timer *)timer->device.data;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStructure = {0};
    mr_uint32_t pclk_freq = 0;

    RCC_GetClocksFreq(&RCC_ClockStructure);

    if ((uint32_t)driver->info.Instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(driver->info.timer_periph_clock, ENABLE);
        if ((RCC->CFGR0 & RCC_PPRE2) == 0)
        {
            pclk_freq = RCC_ClockStructure.PCLK2_Frequency;
        } else
        {
            pclk_freq = 2 * RCC_ClockStructure.PCLK2_Frequency;
        }
    } else
    {
        RCC_APB1PeriphClockCmd(driver->info.timer_periph_clock, ENABLE);
        if ((RCC->CFGR0 & RCC_PPRE1) == 0)
        {
            pclk_freq = RCC_ClockStructure.PCLK1_Frequency;
        } else
        {
            pclk_freq = 2 * RCC_ClockStructure.PCLK1_Frequency;
        }
    }

    switch (timer->info.cnt_mode)
    {
        case _MR_TIMER_CNT_MODE_UP:
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            break;
        case _MR_TIMER_CNT_MODE_DOWN:
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
            break;
        default:
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    }

    switch (timer->info.max_cnt)
    {
        case 0xFFFF:
            TIM_TimeBaseInitStructure.TIM_Prescaler = (pclk_freq / config->freq) - 1;
            break;

        default:
            TIM_TimeBaseInitStructure.TIM_Prescaler = (pclk_freq / config->freq) - 1;
    }

    printf("%d,%d\r\n", TIM_TimeBaseInitStructure.TIM_Prescaler, pclk_freq);

    NVIC_InitStructure.NVIC_IRQChannel = driver->info.irqno;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ClearITPendingBit(driver->info.Instance, TIM_IT_Update);
    TIM_ITConfig(driver->info.Instance, TIM_IT_Update, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(driver->info.Instance, &TIM_TimeBaseInitStructure);

    return MR_ERR_OK;
}

static mr_err_t ch32_timer_start(mr_timer_t timer, mr_uint32_t period_reload)
{
    struct ch32_timer *driver = (struct ch32_timer *)timer->device.data;

    if (timer->info.cnt_mode == _MR_TIMER_CNT_MODE_UP)
    {
        driver->info.Instance->CNT = 0;
    } else
    {
        driver->info.Instance->CNT = period_reload - 1;
    }
    driver->info.Instance->ATRLR = period_reload - 1;
    TIM_Cmd(driver->info.Instance, ENABLE);

    return MR_ERR_OK;
}

static mr_err_t ch32_timer_stop(mr_timer_t timer)
{
    struct ch32_timer *driver = (struct ch32_timer *)timer->device.data;

    TIM_Cmd(driver->info.Instance, DISABLE);

    return MR_ERR_OK;
}

static mr_uint32_t ch32_timer_get_count(mr_timer_t timer)
{
    struct ch32_timer *driver = (struct ch32_timer *)timer->device.data;

    return driver->info.Instance->CNT;
}

#ifdef BSP_TIMER_1
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER1_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_2
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER2_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_3
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER3_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_4
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER4_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_5
void TIM5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM5_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER5_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_6
void TIM6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER6_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_7
void TIM7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER7_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_8
void TIM8_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM8_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER8_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_9
void TIM9_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM9_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER9_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
    }
}
#endif

#ifdef BSP_TIMER_10
void TIM10_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM10_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET)
    {
        mr_timer_device_isr(&timer_device[TIMER10_INDEX], MR_TIMER_EVENT_PIT_INT);
        TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
    }
}
#endif

mr_err_t ch32_timer_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(timer_device);
    static struct mr_timer_ops driver =
            {
                    ch32_timer_configure,
                    ch32_timer_start,
                    ch32_timer_stop,
                    ch32_timer_get_count,
            };

    while (count--)
    {
        ret = mr_timer_device_add(&timer_device[count],
                                  ch32_timer[count].name,
                                  &ch32_timer[count],
                                  &driver,
                                  &timer_device_info);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_timer_init);

#endif /* MR_CONF_TIMER */