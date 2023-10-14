/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-12     MacRsh       first version
 */

#include "drv_pwm.h"

#if (MR_CFG_PWM == MR_CFG_ENABLE)

enum
{
#ifdef MR_BSP_PWM_1
    CH32_PWM_1_INDEX,
#endif
#ifdef MR_BSP_PWM_2
    CH32_PWM_2_INDEX,
#endif
#ifdef MR_BSP_PWM_3
    CH32_PWM_3_INDEX,
#endif
#ifdef MR_BSP_PWM_4
    CH32_PWM_4_INDEX,
#endif
#ifdef MR_BSP_PWM_5
    CH32_PWM_5_INDEX,
#endif
#ifdef MR_BSP_PWM_8
    CH32_PWM_8_INDEX,
#endif
#ifdef MR_BSP_PWM_9
    CH32_PWM_9_INDEX,
#endif
#ifdef MR_BSP_PWM_10
    CH32_PWM_10_INDEX,
#endif
};

static struct ch32_pwm_data ch32_pwm_data[] =
    {
#ifdef MR_BSP_PWM_1
        {"pwm1", TIM1, RCC_APB2Periph_TIM1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10,
         GPIO_Pin_11},
#endif
#ifdef MR_BSP_PWM_2
        {"pwm2", TIM2, RCC_APB1Periph_TIM2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2,
         GPIO_Pin_3},
#endif
#ifdef MR_BSP_PWM_3
        {"pwm3", TIM3, RCC_APB1Periph_TIM3, RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, GPIOA, GPIO_Pin_6,
         GPIO_Pin_7,},
#endif
#ifdef MR_BSP_PWM_4
        {"pwm4", TIM4, RCC_APB1Periph_TIM4, RCC_APB2Periph_GPIOD},
#endif
#ifdef MR_BSP_PWM_5
        {"pwm5", TIM5, RCC_APB1Periph_TIM5},
#endif
#ifdef MR_BSP_PWM_6
        {"pwm6", TIM6, RCC_APB1Periph_TIM6},
#endif
#ifdef MR_BSP_PWM_7
        {"pwm7", TIM7, RCC_APB1Periph_TIM7},
#endif
#ifdef MR_BSP_PWM_8
        {"pwm8", TIM8, RCC_APB2Periph_TIM8},
#endif
#ifdef MR_BSP_PWM_9
        {"pwm9", TIM9, RCC_APB2Periph_TIM9},
#endif
#ifdef MR_BSP_PWM_10
        {"pwm10", TIM10, RCC_APB2Periph_TIM10},
#endif
    };

static struct mr_pwm pwm_device[mr_array_num(ch32_pwm_data)];

static mr_err_t ch32_pwm_configure(mr_pwm_t pwm, mr_pwm_config_t config)
{

}

static void ch32_pwm_write(mr_pwm_t pwm, mr_off_t channel, mr_uint32_t duty)
{

}

static mr_uint32_t ch32_pwm_read(mr_pwm_t pwm, mr_off_t channel)
{

}

mr_err_t drv_pwm_init(void)
{
    static struct mr_pwm_ops drv_ops =
        {
            ch32_pwm_configure,
            ch32_pwm_write,
            ch32_pwm_read,
        };
    mr_size_t count = mr_array_num(pwm_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_pwm_device_add(&pwm_device[count],
                                ch32_pwm_data[count].name,
                                &drv_ops, &ch32_pwm_data[count]
        );
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_pwm_init);

#endif
