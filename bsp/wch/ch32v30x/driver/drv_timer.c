/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-30    MacRsh       First version
 */

#include "drv_timer.h"

#ifdef MR_USING_TIMER

#if !defined(MR_USING_TIMER1) && !defined(MR_USING_TIMER2) && !defined(MR_USING_TIMER3) && !defined(MR_USING_TIMER4) && !defined(MR_USING_TIMER5) && !defined(MR_USING_TIMER6) && !defined(MR_USING_TIMER7) && !defined(MR_USING_TIMER8) && !defined(MR_USING_TIMER9) && !defined(MR_USING_TIMER10)
#error "Please define at least one Timer macro like MR_USING_TIMER1. Otherwise undefine MR_USING_TIMER."
#else

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
};

static const char *timer_name[] =
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
        {TIM1, RCC_APB2Periph_TIM1, TIM1_UP_IRQn},
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        {TIM2, RCC_APB1Periph_TIM2, TIM2_IRQn},
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        {TIM3, RCC_APB1Periph_TIM3, TIM3_IRQn},
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        {TIM4, RCC_APB1Periph_TIM4, TIM4_IRQn},
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        {TIM5, RCC_APB1Periph_TIM5, TIM5_IRQn},
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        {TIM6, RCC_APB1Periph_TIM6, TIM6_IRQn},
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        {TIM7, RCC_APB1Periph_TIM7, TIM7_IRQn},
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        {TIM8, RCC_APB2Periph_TIM8, TIM8_UP_IRQn},
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        {TIM9, RCC_APB2Periph_TIM9, TIM9_UP_IRQn},
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        {TIM10, RCC_APB2Periph_TIM10, TIM10_UP_IRQn},
#endif /* MR_USING_TIMER10 */
    };

static struct mr_timer timer_dev[mr_array_num(timer_drv_data)];

static struct mr_timer_info timer_info[] =
    {
#ifdef MR_USING_TIMER1
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        {0, UINT16_MAX, UINT16_MAX},
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        {0, UINT16_MAX, UINT16_MAX},
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
    timer->info->clk = pclk / 1000000;

    /* Configure timer */
    TIM_TimeBaseInitStructure.TIM_Period = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timer_data->instance, &TIM_TimeBaseInitStructure);

    /* Configure NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = timer_data->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(timer_data->instance, TIM_IT_Update, state);
    return MR_EOK;
}

static void drv_timer_start(struct mr_timer *timer, uint32_t prescaler, uint32_t period)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Set the PSC and ARR, and enable the timer */
    timer_data->instance->CNT = 0;
    timer_data->instance->PSC = prescaler - 1;
    timer_data->instance->ATRLR = period - 1;
    TIM_Cmd(timer_data->instance, ENABLE);
}

static void drv_timer_stop(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Disable the timer */
    TIM_Cmd(timer_data->instance, DISABLE);
}

static uint32_t drv_timer_get_count(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    return timer_data->instance->CNT;
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
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER1]
        },
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER2]
        },
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER3]
        },
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER4]
        },
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER5]
        },
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER6]
        },
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER7]
        },
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER8]
        },
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER9]
        },
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER10]
        },
#endif /* MR_USING_TIMER10 */
    };

int drv_timer_init(void)
{
    int index = 0;

    for (index = 0; index < mr_array_num(timer_dev); index++)
    {
        mr_timer_register(&timer_dev[index], timer_name[index], &timer_drv[index], &timer_info[index]);
    }
    return MR_EOK;
}
MR_DRV_EXPORT(drv_timer_init);

#endif /* !defined(MR_USING_TIMER1) && !defined(MR_USING_TIMER2) && !defined(MR_USING_TIMER3) && !defined(MR_USING_TIMER4) && !defined(MR_USING_TIMER5) && !defined(MR_USING_TIMER6) && !defined(MR_USING_TIMER7) && !defined(MR_USING_TIMER8) && !defined(MR_USING_TIMER9) && !defined(MR_USING_TIMER10) */

#endif /* MR_USING_TIMER */
