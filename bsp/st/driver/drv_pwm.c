/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-21    MacRsh       First version
 */

#include "drv_pwm.h"

#ifdef MR_USING_PWM

#if !defined(MR_USING_PWM1) && !defined(MR_USING_PWM2) && !defined(MR_USING_PWM3) && !defined(MR_USING_PWM4) && \
    !defined(MR_USING_PWM5) && !defined(MR_USING_PWM8) && !defined(MR_USING_PWM9) && !defined(MR_USING_PWM10) && \
    !defined(MR_USING_PWM11) && !defined(MR_USING_PWM12) && !defined(MR_USING_PWM13) && !defined(MR_USING_PWM14)
#warning "Please enable at least one PWM driver"
#endif /* !defined(MR_USING_PWM1) && !defined(MR_USING_PWM2) && !defined(MR_USING_PWM3) && !defined(MR_USING_PWM4) && \
        * !defined(MR_USING_PWM5) && !defined(MR_USING_PWM8) && !defined(MR_USING_PWM9) && !defined(MR_USING_PWM10) && \
        * !defined(MR_USING_PWM11) && !defined(MR_USING_PWM12) && !defined(MR_USING_PWM13) && !defined(MR_USING_PWM14) */

enum drv_pwm_index
{
#ifdef MR_USING_PWM1
    DRV_INDEX_PWM1,
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
    DRV_INDEX_PWM2,
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
    DRV_INDEX_PWM3,
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
    DRV_INDEX_PWM4,
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
    DRV_INDEX_PWM5,
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM6
    DRV_INDEX_PWM6,
#endif /* MR_USING_PWM6 */
#ifdef MR_USING_PWM7
    DRV_INDEX_PWM7,
#endif /* MR_USING_PWM7 */
#ifdef MR_USING_PWM8
    DRV_INDEX_PWM8,
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
    DRV_INDEX_PWM9,
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
    DRV_INDEX_PWM10,
#endif /* MR_USING_PWM10 */
#ifdef MR_USING_PWM11
    DRV_INDEX_PWM11,
#endif /* MR_USING_PWM11 */
#ifdef MR_USING_PWM12
    DRV_INDEX_PWM12,
#endif /* MR_USING_PWM12 */
#ifdef MR_USING_PWM13
    DRV_INDEX_PWM13,
#endif /* MR_USING_PWM13 */
#ifdef MR_USING_PWM14
    DRV_INDEX_PWM14,
#endif /* MR_USING_PWM14 */
    DRV_INDEX_PWM_MAX
};

static const char *pwm_path[] =
    {
#ifdef MR_USING_PWM1
        "pwm1",
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        "pwm2",
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        "pwm3",
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        "pwm4",
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        "pwm5",
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM6
        "pwm6",
#endif /* MR_USING_PWM6 */
#ifdef MR_USING_PWM7
        "pwm7",
#endif /* MR_USING_PWM7 */
#ifdef MR_USING_PWM8
        "pwm8",
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        "pwm9",
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        "pwm10",
#endif /* MR_USING_PWM10 */
#ifdef MR_USING_PWM11
        "pwm11",
#endif /* MR_USING_PWM11 */
#ifdef MR_USING_PWM12
        "pwm12",
#endif /* MR_USING_PWM12 */
#ifdef MR_USING_PWM13
        "pwm13",
#endif /* MR_USING_PWM13 */
#ifdef MR_USING_PWM14
        "pwm14",
#endif /* MR_USING_PWM14 */
    };

static struct drv_pwm_data pwm_drv_data[] =
    {
#ifdef MR_USING_PWM1
        {{0}, TIM1},
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        {{0}, TIM2},
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        {{0}, TIM3},
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        {{0}, TIM4},
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        {{0}, TIM5},
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM6
        {{0}, TIM6},
#endif /* MR_USING_PWM6 */
#ifdef MR_USING_PWM7
        {{0}, TIM7},
#endif /* MR_USING_PWM7 */
#ifdef MR_USING_PWM8
        {{0}, TIM8},
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        {{0}, TIM9},
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        {{0}, TIM10},
#endif /* MR_USING_PWM10 */
#ifdef MR_USING_PWM11
        {{0}, TIM11},
#endif /* MR_USING_PWM11 */
#ifdef MR_USING_PWM12
        {{0}, TIM12},
#endif /* MR_USING_PWM12 */
#ifdef MR_USING_PWM13
        {{0}, TIM13},
#endif /* MR_USING_PWM13 */
#ifdef MR_USING_PWM14
        {{0}, TIM14},
#endif /* MR_USING_PWM14 */
    };

static struct mr_pwm pwm_dev[MR_ARRAY_NUM(pwm_drv_data)];

static struct mr_pwm_info pwm_info[] =
    {
#ifdef MR_USING_PWM1
        DRV_PWM1_INFO_CONFIG,
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        DRV_PWM2_INFO_CONFIG,
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        DRV_PWM3_INFO_CONFIG,
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        DRV_PWM4_INFO_CONFIG,
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        DRV_PWM5_INFO_CONFIG,
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM6
        DRV_PWM6_INFO_CONFIG,
#endif /* MR_USING_PWM6 */
#ifdef MR_USING_PWM7
        DRV_PWM7_INFO_CONFIG,
#endif /* MR_USING_PWM7 */
#ifdef MR_USING_PWM8
        DRV_PWM8_INFO_CONFIG,
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        DRV_PWM9_INFO_CONFIG,
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        DRV_PWM10_INFO_CONFIG,
#endif /* MR_USING_PWM10 */
#ifdef MR_USING_PWM11
        DRV_PWM11_INFO_CONFIG,
#endif /* MR_USING_PWM11 */
#ifdef MR_USING_PWM12
        DRV_PWM12_INFO_CONFIG,
#endif /* MR_USING_PWM12 */
#ifdef MR_USING_PWM13
        DRV_PWM13_INFO_CONFIG,
#endif /* MR_USING_PWM13 */
#ifdef MR_USING_PWM14
        DRV_PWM14_INFO_CONFIG,
#endif /* MR_USING_PWM14 */
    };

static int drv_pwm_configure(struct mr_pwm *pwm, int state)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    pwm_data->handle.Instance = pwm_data->instance;
    RCC_ClkInitTypeDef RCC_ClkInitStructure = {0};
    uint32_t pclk = 0, flatency = 0;

    /* Configure clock */
    HAL_RCC_GetClockConfig(&RCC_ClkInitStructure, &flatency);
    if ((uint32_t)pwm_data->instance > APB2PERIPH_BASE)
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

    /* Update pwm clock(MHz) */
    pwm->info->clk = pclk;

    if (state == ENABLE)
    {
        /* Configure pwm */
        pwm_data->handle.Init.Prescaler = 0;
        pwm_data->handle.Init.CounterMode = TIM_COUNTERMODE_UP;
        pwm_data->handle.Init.Period = 0;
        pwm_data->handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        pwm_data->handle.Init.RepetitionCounter = 0;
        pwm_data->handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        HAL_TIM_Base_Init(&pwm_data->handle);

        if (IS_TIM_BREAK_INSTANCE(pwm_data->handle.Instance) != RESET)
        {
            __HAL_TIM_MOE_ENABLE(&pwm_data->handle);
        }
    } else
    {
        /* Configure pwm */
        HAL_TIM_Base_DeInit(&pwm_data->handle);

        if (IS_TIM_BREAK_INSTANCE(pwm_data->handle.Instance) != RESET)
        {
            __HAL_TIM_MOE_DISABLE(&pwm_data->handle);
        }
    }
    return MR_EOK;
}

static int drv_pwm_channel_configure(struct mr_pwm *pwm, int channel, int state, int polarity)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    uint32_t ChannelState = (state == ENABLE) ? TIM_CCx_ENABLE : TIM_CCx_DISABLE;
    uint32_t Channel = (channel - 1) << 2;
    TIM_OC_InitTypeDef sConfigOC = {0};

    if (channel <= 0)
    {
        return MR_EINVAL;
    }

    sConfigOC.Pulse = 0;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = (polarity == MR_PWM_POLARITY_NORMAL) ? TIM_OCPOLARITY_HIGH : TIM_OCPOLARITY_LOW;
    sConfigOC.OCNPolarity = (polarity == MR_PWM_POLARITY_NORMAL) ? TIM_OCNPOLARITY_HIGH : TIM_OCNPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&pwm_data->handle, &sConfigOC, Channel);
    TIM_CCxChannelCmd(pwm_data->handle.Instance, Channel, ChannelState);
    return MR_EOK;
}

static void drv_pwm_start(struct mr_pwm *pwm, uint32_t prescaler, uint32_t period)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;

    /* Set the PSC and ARR, and enable the pwm */
    __HAL_TIM_SET_COUNTER(&pwm_data->handle, 0);
    __HAL_TIM_SET_AUTORELOAD(&pwm_data->handle, period - 1);
    __HAL_TIM_SET_PRESCALER(&pwm_data->handle, prescaler - 1);
    HAL_TIM_GenerateEvent(&pwm_data->handle, TIM_EVENTSOURCE_UPDATE);
    __HAL_TIM_ENABLE(&pwm_data->handle);
}

static int drv_pwm_read(struct mr_pwm *pwm, int channel, uint32_t *compare_value)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    uint32_t Channel = (channel - 1) << 2;

#ifdef MR_USING_PWM_CHANNEL_CHECK
    if ((Channel & TIM_CHANNEL_ALL) || (Channel == 0))
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_PWM_CHANNEL_CHECK */

    *compare_value = __HAL_TIM_GET_COMPARE(&pwm_data->handle, Channel);
    return MR_EOK;
}

static int drv_pwm_write(struct mr_pwm *pwm, int channel, uint32_t compare_value)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    uint32_t Channel = (channel - 1) << 2;

#ifdef MR_USING_PWM_CHANNEL_CHECK
    if ((Channel & TIM_CHANNEL_ALL) || (Channel == 0))
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_PWM_CHANNEL_CHECK */

    __HAL_TIM_SET_COMPARE(&pwm_data->handle, Channel, compare_value);
    return MR_EOK;
}

static struct mr_pwm_ops pwm_drv_ops =
    {
        drv_pwm_configure,
        drv_pwm_channel_configure,
        drv_pwm_start,
        drv_pwm_read,
        drv_pwm_write
    };

static struct mr_drv pwm_drv[] =
    {
#ifdef MR_USING_PWM1
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM1]
        },
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM2]
        },
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM3]
        },
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM4]
        },
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM5]
        },
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM6
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM6]
        },
#endif /* MR_USING_PWM6 */
#ifdef MR_USING_PWM7
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM7]
        },
#endif /* MR_USING_PWM7 */
#ifdef MR_USING_PWM8
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM8]
        },
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM9]
        },
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM10]
        },
#endif /* MR_USING_PWM10 */
#ifdef MR_USING_PWM11
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM11]
        },
#endif /* MR_USING_PWM11 */
#ifdef MR_USING_PWM12
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM12]
        },
#endif /* MR_USING_PWM12 */
#ifdef MR_USING_PWM13
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM13]
        },
#endif /* MR_USING_PWM13 */
#ifdef MR_USING_PWM14
        {
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM14]
        },
#endif /* MR_USING_PWM14 */
    };

static void drv_pwm_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(pwm_dev); i++)
    {
        mr_pwm_register(&pwm_dev[i], pwm_path[i], &pwm_drv[i], &pwm_info[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_pwm_init);

#endif /* MR_USING_PWM */
