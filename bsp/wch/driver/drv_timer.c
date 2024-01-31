/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-30    MacRsh       First version
 */

#include "drv_timer.h"

#ifdef MR_USING_TIMER

#if !defined(MR_USING_TIMER1) && !defined(MR_USING_TIMER2) && !defined(MR_USING_TIMER3) && !defined(MR_USING_TIMER4) && \
    !defined(MR_USING_TIMER5) && !defined(MR_USING_TIMER8) && !defined(MR_USING_TIMER9) && !defined(MR_USING_TIMER10)
#warning "Please enable at least one Timer driver"
#endif /* !defined(MR_USING_TIMER1) && !defined(MR_USING_TIMER2) && !defined(MR_USING_TIMER3) && !defined(MR_USING_TIMER4) &&
        * !defined(MR_USING_TIMER5) && !defined(MR_USING_TIMER8) && !defined(MR_USING_TIMER9) && !defined(MR_USING_TIMER10) */

enum drv_timer_index
{
#ifdef MR_USING_TIMER1
    DRV_INDEX_TIMER1,
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
    DRV_INDEX_TIMER2,
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
    DRV_INDEX_TIMER3,
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
    DRV_INDEX_TIMER4,
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
    DRV_INDEX_TIMER5,
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
    DRV_INDEX_TIMER6,
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
    DRV_INDEX_TIMER7,
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
    DRV_INDEX_TIMER8,
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
    DRV_INDEX_TIMER9,
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
    DRV_INDEX_TIMER10,
#endif /* MR_USING_TIMER10 */
    DRV_INDEX_TIMER_MAX
};

static const char *timer_path[] =
    {
#ifdef MR_USING_TIMER1
        "timer1",
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        "timer2",
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        "timer3",
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        "timer4",
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        "timer5",
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        "timer6",
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        "timer7",
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        "timer8",
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        "timer9",
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        "timer10",
#endif /* MR_USING_TIMER10 */
    };

static struct drv_timer_data timer_drv_data[] =
    {
#ifdef MR_USING_TIMER1
        DRV_TIMER1_CONFIG,
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        DRV_TIMER2_CONFIG,
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        DRV_TIMER3_CONFIG,
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        DRV_TIMER4_CONFIG,
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        DRV_TIMER5_CONFIG,
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        DRV_TIMER6_CONFIG,
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        DRV_TIMER7_CONFIG,
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        DRV_TIMER8_CONFIG,
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        DRV_TIMER9_CONFIG,
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        DRV_TIMER10_CONFIG,
#endif /* MR_USING_TIMER10 */
    };

static struct mr_timer timer_dev[MR_ARRAY_NUM(timer_drv_data)];

static struct mr_timer_info timer_info[] =
    {
#ifdef MR_USING_TIMER1
        DRV_TIMER1_INFO_CONFIG,
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        DRV_TIMER2_INFO_CONFIG,
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        DRV_TIMER3_INFO_CONFIG,
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        DRV_TIMER4_INFO_CONFIG,
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        DRV_TIMER5_INFO_CONFIG,
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        DRV_TIMER6_INFO_CONFIG,
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        DRV_TIMER7_INFO_CONFIG,
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        DRV_TIMER8_INFO_CONFIG,
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        DRV_TIMER9_INFO_CONFIG,
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        DRV_TIMER10_INFO_CONFIG,
#endif /* MR_USING_TIMER10 */
    };

static int drv_timer_configure(struct mr_timer *timer, int state)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStructure = {0};
    uint32_t pclk = 0;

    /* Configure clock */
    RCC_GetClocksFreq(&RCC_ClockStructure);
    if ((uint32_t)timer_data->instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(timer_data->clock, state);
        pclk = RCC_ClockStructure.PCLK2_Frequency;
    } else
    {
        RCC_APB1PeriphClockCmd(timer_data->clock, state);
        pclk = RCC_ClockStructure.PCLK1_Frequency;
    }

    /* Update timer clock(MHz) */
    timer->info->clk = pclk;

    /* Configure timer */
    TIM_TimeBaseInitStructure.TIM_Period = 0;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timer_data->instance, &TIM_TimeBaseInitStructure);

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = timer_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    if (state == DISABLE)
    {
        TIM_ITConfig(timer_data->instance, TIM_IT_Update, DISABLE);
        TIM_Cmd(timer_data->instance, DISABLE);
    }
    return MR_EOK;
}

static void drv_timer_start(struct mr_timer *timer, uint32_t prescaler, uint32_t period)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Set the PSC and ARR, and enable the timer */
    TIM_SetCounter(timer_data->instance, 0);
    TIM_SetAutoreload(timer_data->instance, period - 1);
    TIM_PrescalerConfig(timer_data->instance, prescaler - 1, TIM_PSCReloadMode_Immediate);
    TIM_ClearITPendingBit(timer_data->instance, TIM_IT_Update);
    TIM_ITConfig(timer_data->instance, TIM_IT_Update, ENABLE);
    TIM_Cmd(timer_data->instance, ENABLE);
}

static void drv_timer_stop(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Disable the timer */
    TIM_ITConfig(timer_data->instance, TIM_IT_Update, DISABLE);
    TIM_Cmd(timer_data->instance, DISABLE);
}

static uint32_t drv_timer_get_count(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    return TIM_GetCounter(timer_data->instance);
}

static void drv_timer_isr(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    if (TIM_GetITStatus(timer_data->instance, TIM_IT_Update) != RESET)
    {
        mr_dev_isr(&timer->dev, MR_ISR_TIMER_TIMEOUT_INT, MR_NULL);
        TIM_ClearITPendingBit(timer_data->instance, TIM_IT_Update);
    }
}

#ifdef MR_USING_TIMER1
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER1]);
}
#endif /* MR_USING_TIMER1 */

#ifdef MR_USING_TIMER2
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER2]);
}
#endif /* MR_USING_TIMER2 */

#ifdef MR_USING_TIMER3
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER3]);
}
#endif /* MR_USING_TIMER3 */

#ifdef MR_USING_TIMER4
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER4]);
}
#endif /* MR_USING_TIMER4 */

#ifdef MR_USING_TIMER5
void TIM5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM5_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER5]);
}
#endif /* MR_USING_TIMER5 */

#ifdef MR_USING_TIMER6
void TIM6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM6_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER6]);
}
#endif /* MR_USING_TIMER6 */

#ifdef MR_USING_TIMER7
void TIM7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM7_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER7]);
}
#endif /* MR_USING_TIMER7 */

#ifdef MR_USING_TIMER8
void TIM8_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM8_UP_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER8]);
}
#endif /* MR_USING_TIMER8 */

#ifdef MR_USING_TIMER9
void TIM9_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM9_UP_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER9]);
}
#endif /* MR_USING_TIMER9 */

#ifdef MR_USING_TIMER10
void TIM10_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM10_UP_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER10]);
}
#endif /* MR_USING_TIMER10 */

static struct mr_timer_ops timer_drv_ops =
    {
        drv_timer_configure,
        drv_timer_start,
        drv_timer_stop,
        drv_timer_get_count
    };

static struct mr_drv timer_drv[] =
    {
#ifdef MR_USING_TIMER1
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER1]
        },
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER2]
        },
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER3]
        },
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER4]
        },
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER5]
        },
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER6]
        },
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER7]
        },
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER8]
        },
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER9]
        },
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        {
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER10]
        },
#endif /* MR_USING_TIMER10 */
    };

static void drv_timer_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(timer_dev); i++)
    {
        mr_timer_register(&timer_dev[i], timer_path[i], &timer_drv[i], &timer_info[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_timer_init);

#endif /* MR_USING_TIMER */
