/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-30    MacRsh       First version
 */

#include "drv_timer.h"

#ifdef MR_USING_TIMER

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
#ifdef MR_USING_TIMER11
    DRV_INDEX_TIMER11,
#endif /* MR_USING_TIMER11 */
#ifdef MR_USING_TIMER12
    DRV_INDEX_TIMER12,
#endif /* MR_USING_TIMER12 */
#ifdef MR_USING_TIMER13
    DRV_INDEX_TIMER13,
#endif /* MR_USING_TIMER13 */
#ifdef MR_USING_TIMER14
    DRV_INDEX_TIMER14,
#endif /* MR_USING_TIMER14 */
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
#ifdef MR_USING_TIMER11
        "timer11",
#endif /* MR_USING_TIMER11 */
#ifdef MR_USING_TIMER12
        "timer12",
#endif /* MR_USING_TIMER12 */
#ifdef MR_USING_TIMER13
        "timer13",
#endif /* MR_USING_TIMER13 */
#ifdef MR_USING_TIMER14
        "timer14",
#endif /* MR_USING_TIMER14 */
    };

static struct drv_timer_data timer_drv_data[] =
    {
#ifdef MR_USING_TIMER1
        {{0}, TIM1, TIM1_UP_TIM10_IRQn},
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER2
        {{0}, TIM2, TIM2_IRQn},
#endif /* MR_USING_TIMER2 */
#ifdef MR_USING_TIMER3
        {{0}, TIM3, TIM3_IRQn},
#endif /* MR_USING_TIMER3 */
#ifdef MR_USING_TIMER4
        {{0}, TIM4, TIM4_IRQn},
#endif /* MR_USING_TIMER4 */
#ifdef MR_USING_TIMER5
        {{0}, TIM5, TIM5_IRQn},
#endif /* MR_USING_TIMER5 */
#ifdef MR_USING_TIMER6
        {{0}, TIM6, TIM6_DAC_IRQn},
#endif /* MR_USING_TIMER6 */
#ifdef MR_USING_TIMER7
        {{0}, TIM7, TIM7_IRQn},
#endif /* MR_USING_TIMER7 */
#ifdef MR_USING_TIMER8
        {{0}, TIM8, TIM8_UP_TIM13_IRQn},
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER9
        {{0}, TIM9, TIM1_BRK_TIM9_IRQn},
#endif /* MR_USING_TIMER9 */
#ifdef MR_USING_TIMER10
        {{0}, TIM10, TIM1_UP_TIM10_IRQn},
#endif /* MR_USING_TIMER10 */
#ifdef MR_USING_TIMER11
        {{0}, TIM11, TIM1_TRG_COM_TIM11_IRQn},
#endif /* MR_USING_TIMER11 */
#ifdef MR_USING_TIMER12
        {{0}, TIM12, TIM8_BRK_TIM12_IRQn},
#endif /* MR_USING_TIMER12 */
#ifdef MR_USING_TIMER13
        {{0}, TIM13, TIM8_UP_TIM13_IRQn},
#endif /* MR_USING_TIMER13 */
#ifdef MR_USING_TIMER14
        {{0}, TIM14, TIM8_TRG_COM_TIM14_IRQn},
#endif /* MR_USING_TIMER14 */
    };

static struct mr_timer timer_dev[mr_array_num(timer_drv_data)];

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
#ifdef MR_USING_TIMER11
        DRV_TIMER11_INFO_CONFIG,
#endif /* MR_USING_TIMER11 */
#ifdef MR_USING_TIMER12
        DRV_TIMER12_INFO_CONFIG,
#endif /* MR_USING_TIMER12 */
#ifdef MR_USING_TIMER13
        DRV_TIMER13_INFO_CONFIG,
#endif /* MR_USING_TIMER13 */
#ifdef MR_USING_TIMER14
        DRV_TIMER14_INFO_CONFIG,
#endif /* MR_USING_TIMER14 */
    };

static int drv_timer_configure(struct mr_timer *timer, int state)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;
    timer_data->handle.Instance = timer_data->instance;
    RCC_ClkInitTypeDef RCC_ClkInitStructure = {0};
    uint32_t pclk = 0, flatency = 0;

    /* Configure clock */
    HAL_RCC_GetClockConfig(&RCC_ClkInitStructure, &flatency);
    if ((uint32_t)timer_data->instance > APB2PERIPH_BASE)
    {
        pclk = HAL_RCC_GetPCLK2Freq();
        if (RCC_ClkInitStructure.APB2CLKDivider != RCC_HCLK_DIV1)
        {
            pclk *= 2;
        }
    } else
    {
        pclk = HAL_RCC_GetPCLK1Freq();
        if (RCC_ClkInitStructure.APB1CLKDivider != RCC_HCLK_DIV1)
        {
            pclk *= 2;
        }
    }

    /* Update timer clock(MHz) */
    timer->info->clk = pclk / 1000000;

    if (state == ENABLE)
    {
        /* Configure timer */
        timer_data->handle.Init.Prescaler = 0;
        timer_data->handle.Init.CounterMode = TIM_COUNTERMODE_UP;
        timer_data->handle.Init.Period = 0;
        timer_data->handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        timer_data->handle.Init.RepetitionCounter = 0;
        timer_data->handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        HAL_TIM_Base_Init(&timer_data->handle);

        /* Configure NVIC */
        HAL_NVIC_SetPriority(timer_data->irq, 3, 0);
        HAL_NVIC_EnableIRQ(timer_data->irq);
        __HAL_TIM_ENABLE_IT(&timer_data->handle, TIM_IT_UPDATE);
    } else
    {
        /* Configure timer */
        HAL_TIM_Base_DeInit(&timer_data->handle);

        /* Configure timer */
        __HAL_TIM_DISABLE_IT(&timer_data->handle, TIM_IT_UPDATE);
    }
    return MR_EOK;
}

static void drv_timer_start(struct mr_timer *timer, uint32_t prescaler, uint32_t period)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Set the PSC and ARR, and enable the timer */
    __HAL_TIM_SET_COUNTER(&timer_data->handle, 0);
    __HAL_TIM_SET_AUTORELOAD(&timer_data->handle, period - 1);
    __HAL_TIM_SET_PRESCALER(&timer_data->handle, prescaler - 1);
    __HAL_TIM_ENABLE(&timer_data->handle);
    timer_data->handle.Instance->EGR = TIM_EGR_UG;
}

static void drv_timer_stop(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    /* Disable the timer */
    __HAL_TIM_DISABLE(&timer_data->handle);
}

static uint32_t drv_timer_get_count(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    return __HAL_TIM_GET_COUNTER(&timer_data->handle);
}

static void drv_timer_isr(struct mr_timer *timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->dev.drv->data;

    if ((__HAL_TIM_GET_FLAG(&timer_data->handle, TIM_FLAG_UPDATE) != RESET)
        && (__HAL_TIM_GET_IT_SOURCE(&timer_data->handle, TIM_IT_UPDATE) != RESET))
    {
        mr_dev_isr(&timer->dev, MR_ISR_TIMER_TIMEOUT_INT, MR_NULL);
        __HAL_TIM_CLEAR_FLAG(&timer_data->handle, TIM_FLAG_UPDATE);
    }
}

#if defined(MR_USING_TIMER1) || defined(MR_USING_TIMER10)
void TIM1_UP_TIM10_IRQHandler(void)
{
#ifdef MR_USING_TIMER1
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER1]);
#endif /* MR_USING_TIMER1 */
#ifdef MR_USING_TIMER10
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER10]);
#endif /* MR_USING_TIMER10 */
}
#endif /* MR_USING_TIMER1 || MR_USING_TIMER10 */

#ifdef MR_USING_TIMER2
void TIM2_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER2]);
}
#endif /* MR_USING_TIMER2 */

#ifdef MR_USING_TIMER3
void TIM3_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER3]);
}
#endif /* MR_USING_TIMER3 */

#ifdef MR_USING_TIMER4
void TIM4_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER4]);
}
#endif /* MR_USING_TIMER4 */

#ifdef MR_USING_TIMER5
void TIM5_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER5]);
}
#endif /* MR_USING_TIMER5 */

#ifdef MR_USING_TIMER6
void TIM6_DAC_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER6]);
}
#endif /* MR_USING_TIMER6 */

#ifdef MR_USING_TIMER7
void TIM7_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER7]);
}
#endif /* MR_USING_TIMER7 */

#if defined(MR_USING_TIMER8) || defined(MR_USING_TIMER13)
void TIM8_UP_TIM13_IRQHandler(void)
{
#ifdef MR_USING_TIMER8
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER8]);
#endif /* MR_USING_TIMER8 */
#ifdef MR_USING_TIMER13
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER13]);
#endif /* MR_USING_TIMER13 */
}
#endif /* MR_USING_TIMER8 || MR_USING_TIMER13 */

#ifdef MR_USING_TIMER9
void TIM1_BRK_TIM9_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER9]);
}
#endif /* MR_USING_TIMER9 */

#ifdef MR_USING_TIMER11
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER11]);
}
#endif /* MR_USING_TIMER11 */

#ifdef MR_USING_TIMER12
void TIM8_BRK_TIM12_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER12]);
}
#endif /* MR_USING_TIMER12 */

#ifdef MR_USING_TIMER14
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
    drv_timer_isr(&timer_dev[DRV_INDEX_TIMER14]);
}
#endif /* MR_USING_TIMER14 */

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
#ifdef MR_USING_TIMER11
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER11]
        },
#endif /* MR_USING_TIMER11 */
#ifdef MR_USING_TIMER12
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER12]
        },
#endif /* MR_USING_TIMER12 */
#ifdef MR_USING_TIMER13
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER13]
        },
#endif /* MR_USING_TIMER13 */
#ifdef MR_USING_TIMER14
        {
            Mr_Drv_Type_Timer,
            &timer_drv_ops,
            &timer_drv_data[DRV_INDEX_TIMER14]
        },
#endif /* MR_USING_TIMER14 */
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

#endif /* MR_USING_TIMER */
