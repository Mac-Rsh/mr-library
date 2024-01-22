/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-01    MacRsh       First version
 */

#include "drv_pwm.h"

#ifdef MR_USING_PWM

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
#ifdef MR_USING_PWM8
    DRV_INDEX_PWM8,
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
    DRV_INDEX_PWM9,
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
    DRV_INDEX_PWM10,
#endif /* MR_USING_PWM10 */
    DRV_INDEX_PWM_MAX
};

static const char *pwm_name[] =
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
#ifdef MR_USING_PWM8
        "pwm8",
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        "pwm9",
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        "pwm10",
#endif /* MR_USING_PWM10 */
    };

static struct drv_pwm_data pwm_drv_data[] =
    {
#ifdef MR_USING_PWM1
        DRV_PWM1_CONFIG,
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        DRV_PWM2_CONFIG,
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        DRV_PWM3_CONFIG,
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        DRV_PWM4_CONFIG,
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        DRV_PWM5_CONFIG,
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM8
        DRV_PWM8_CONFIG,
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        DRV_PWM9_CONFIG,
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        DRV_PWM10_CONFIG,
#endif /* MR_USING_PWM10 */
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
#ifdef MR_USING_PWM8
        DRV_PWM8_INFO_CONFIG,
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        DRV_PWM9_INFO_CONFIG,
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        DRV_PWM10_INFO_CONFIG,
#endif /* MR_USING_PWM10 */
    };

static int drv_pwm_configure(struct mr_pwm *pwm, int state)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    RCC_ClocksTypeDef RCC_ClockStructure = {0};
    uint32_t pclk = 0;

    /* Configure clock */
    RCC_GetClocksFreq(&RCC_ClockStructure);
    if ((uint32_t)pwm_data->instance > APB2PERIPH_BASE)
    {
        RCC_APB2PeriphClockCmd(pwm_data->clock, state);
        pclk = RCC_ClockStructure.PCLK2_Frequency;
    } else
    {
        RCC_APB1PeriphClockCmd(pwm_data->clock, state);
        pclk = RCC_ClockStructure.PCLK1_Frequency;
    }

    /* Update pwm clock(Hz) */
    pwm->info->clk = pclk;

    /* Configure remap */
    if (pwm_data->remap != 0)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(pwm_data->remap, state);
    }

    /* Configure pwm */
    TIM_TimeBaseInitStructure.TIM_Period = 0;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(pwm_data->instance, &TIM_TimeBaseInitStructure);
    if (state == DISABLE)
    {
        TIM_Cmd(pwm_data->instance, DISABLE);
    }
    return MR_EOK;
}

static int drv_pwm_channel_configure(struct mr_pwm *pwm, int channel, int state, int polarity)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;
    int OCPreload = (state == ENABLE) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable;
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(pwm_data->gpio_clock, ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = (state == MR_ENABLE) ? GPIO_Mode_AF_PP : GPIO_Mode_IN_FLOATING;

    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_OutputState = (state == MR_ENABLE) ? TIM_OutputState_Enable : TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity = (polarity == MR_PWM_POLARITY_NORMAL)
                                         ? TIM_OCPolarity_High
                                         : TIM_OCPolarity_Low;
    switch (channel)
    {
        case 1:
        {
            GPIO_InitStructure.GPIO_Pin = pwm_data->ch1_pin;
            GPIO_Init(pwm_data->ch1_port, &GPIO_InitStructure);
            TIM_OC1Init(pwm_data->instance, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(pwm_data->instance, OCPreload);
            return MR_EOK;
        }
        case 2:
        {
            GPIO_InitStructure.GPIO_Pin = pwm_data->ch2_pin;
            GPIO_Init(pwm_data->ch2_port, &GPIO_InitStructure);
            TIM_OC2Init(pwm_data->instance, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(pwm_data->instance, OCPreload);
            return MR_EOK;
        }
        case 3:
        {
            GPIO_InitStructure.GPIO_Pin = pwm_data->ch3_pin;
            GPIO_Init(pwm_data->ch3_port, &GPIO_InitStructure);
            TIM_OC3Init(pwm_data->instance, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(pwm_data->instance, OCPreload);
            return MR_EOK;
        }
        case 4:
        {
            GPIO_InitStructure.GPIO_Pin = pwm_data->ch4_pin;
            GPIO_Init(pwm_data->ch4_port, &GPIO_InitStructure);
            TIM_OC4Init(pwm_data->instance, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(pwm_data->instance, OCPreload);
            return MR_EOK;
        }
        default:
        {
            return MR_EINVAL;
        }
    }
}

static void drv_pwm_start(struct mr_pwm *pwm, uint32_t prescaler, uint32_t period)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;

    /* Set the PSC and ARR, and enable the timer */
    TIM_SetCounter(pwm_data->instance, 0);
    TIM_SetAutoreload(pwm_data->instance, period - 1);
    TIM_PrescalerConfig(pwm_data->instance, prescaler - 1, TIM_PSCReloadMode_Update);
    TIM_CtrlPWMOutputs(pwm_data->instance, ENABLE);
    TIM_Cmd(pwm_data->instance, ENABLE);
}

static void drv_pwm_write(struct mr_pwm *pwm, int channel, uint32_t compare_value)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;

    switch (channel)
    {
        case 1:
        {
            TIM_SetCompare1(pwm_data->instance, compare_value);
            return;
        }
        case 2:
        {
            TIM_SetCompare2(pwm_data->instance, compare_value);
            return;
        }
        case 3:
        {
            TIM_SetCompare3(pwm_data->instance, compare_value);
            return;
        }
        case 4:
        {
            TIM_SetCompare4(pwm_data->instance, compare_value);
            return;
        }
        default:
        {
            return;
        }
    }
}

static uint32_t drv_pwm_read(struct mr_pwm *pwm, int channel)
{
    struct drv_pwm_data *pwm_data = (struct drv_pwm_data *)pwm->dev.drv->data;

    switch (channel)
    {
        case 1:
        {
            return TIM_GetCapture1(pwm_data->instance);
        }
        case 2:
        {
            return TIM_GetCapture2(pwm_data->instance);
        }
        case 3:
        {
            return TIM_GetCapture3(pwm_data->instance);
        }
        case 4:
        {
            return TIM_GetCapture4(pwm_data->instance);
        }
        default:
        {
            return 0;
        }
    }
}

static struct mr_pwm_ops pwm_drv_ops =
    {
        drv_pwm_configure,
        drv_pwm_channel_configure,
        drv_pwm_start,
        drv_pwm_write,
        drv_pwm_read
    };

static struct mr_drv pwm_drv[] =
    {
#ifdef MR_USING_PWM1
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM1]
        },
#endif /* MR_USING_PWM1 */
#ifdef MR_USING_PWM2
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM2]
        },
#endif /* MR_USING_PWM2 */
#ifdef MR_USING_PWM3
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM3]
        },
#endif /* MR_USING_PWM3 */
#ifdef MR_USING_PWM4
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM4]
        },
#endif /* MR_USING_PWM4 */
#ifdef MR_USING_PWM5
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM5]
        },
#endif /* MR_USING_PWM5 */
#ifdef MR_USING_PWM8
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM8]
        },
#endif /* MR_USING_PWM8 */
#ifdef MR_USING_PWM9
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM9]
        },
#endif /* MR_USING_PWM9 */
#ifdef MR_USING_PWM10
        {
            Mr_Drv_Type_PWM,
            &pwm_drv_ops,
            &pwm_drv_data[DRV_INDEX_PWM10]
        },
#endif /* MR_USING_PWM10 */
    };

static int drv_pwm_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(pwm_dev); i++)
    {
        mr_pwm_register(&pwm_dev[i], pwm_name[i], &pwm_drv[i], &pwm_info[i]);
    }
    return MR_EOK;
}
MR_INIT_DRV_EXPORT(drv_pwm_init);

#endif /* MR_USING_PWM */
