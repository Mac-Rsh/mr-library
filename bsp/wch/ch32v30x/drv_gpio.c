/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "drv_gpio.h"

#ifdef MR_USING_GPIO

#if !defined(MR_USING_GPIOA) && !defined(MR_USING_GPIOB) && !defined(MR_USING_GPIOC) && !defined(MR_USING_GPIOD) && !defined(MR_USING_GPIOE) && !defined(MR_USING_GPIOF)
#error "Please define at least one GPIO macro like MR_USING_GPIOA. Otherwise undefine MR_USING_GPIO."
#endif

static IRQn_Type gpio_irq_map[] =
    {
        EXTI0_IRQn,
        EXTI1_IRQn,
        EXTI2_IRQn,
        EXTI3_IRQn,
        EXTI4_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
    };

static int gpio_irq_mask[] =
    {
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
    };

static struct drv_gpio_data gpio_drv_data[] =
    {
#ifdef MR_USING_GPIOA
        {GPIOA, GPIO_Pin_0},
        {GPIOA, GPIO_Pin_1},
        {GPIOA, GPIO_Pin_2},
        {GPIOA, GPIO_Pin_3},
        {GPIOA, GPIO_Pin_4},
        {GPIOA, GPIO_Pin_5},
        {GPIOA, GPIO_Pin_6},
        {GPIOA, GPIO_Pin_7},
        {GPIOA, GPIO_Pin_8},
        {GPIOA, GPIO_Pin_9},
        {GPIOA, GPIO_Pin_10},
        {GPIOA, GPIO_Pin_11},
        {GPIOA, GPIO_Pin_12},
        {GPIOA, GPIO_Pin_13},
        {GPIOA, GPIO_Pin_14},
        {GPIOA, GPIO_Pin_15},
#else
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
#endif /* MR_USING_GPIOA */
#ifdef MR_USING_GPIOB
        {GPIOB, GPIO_Pin_0},
        {GPIOB, GPIO_Pin_1},
        {GPIOB, GPIO_Pin_2},
        {GPIOB, GPIO_Pin_3},
        {GPIOB, GPIO_Pin_4},
        {GPIOB, GPIO_Pin_5},
        {GPIOB, GPIO_Pin_6},
        {GPIOB, GPIO_Pin_7},
        {GPIOB, GPIO_Pin_8},
        {GPIOB, GPIO_Pin_9},
        {GPIOB, GPIO_Pin_10},
        {GPIOB, GPIO_Pin_11},
        {GPIOB, GPIO_Pin_12},
        {GPIOB, GPIO_Pin_13},
        {GPIOB, GPIO_Pin_14},
        {GPIOB, GPIO_Pin_15},
#else
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
#endif /* MR_USING_GPIOB */
#ifdef MR_USING_GPIOC
        {GPIOC, GPIO_Pin_0},
        {GPIOC, GPIO_Pin_1},
        {GPIOC, GPIO_Pin_2},
        {GPIOC, GPIO_Pin_3},
        {GPIOC, GPIO_Pin_4},
        {GPIOC, GPIO_Pin_5},
        {GPIOC, GPIO_Pin_6},
        {GPIOC, GPIO_Pin_7},
        {GPIOC, GPIO_Pin_8},
        {GPIOC, GPIO_Pin_9},
        {GPIOC, GPIO_Pin_10},
        {GPIOC, GPIO_Pin_11},
        {GPIOC, GPIO_Pin_12},
        {GPIOC, GPIO_Pin_13},
        {GPIOC, GPIO_Pin_14},
        {GPIOC, GPIO_Pin_15},
#else
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
#endif /* MR_USING_GPIOC */
#ifdef MR_USING_GPIOD
        {GPIOD, GPIO_Pin_0},
        {GPIOD, GPIO_Pin_1},
        {GPIOD, GPIO_Pin_2},
        {GPIOD, GPIO_Pin_3},
        {GPIOD, GPIO_Pin_4},
        {GPIOD, GPIO_Pin_5},
        {GPIOD, GPIO_Pin_6},
        {GPIOD, GPIO_Pin_7},
        {GPIOD, GPIO_Pin_8},
        {GPIOD, GPIO_Pin_9},
        {GPIOD, GPIO_Pin_10},
        {GPIOD, GPIO_Pin_11},
        {GPIOD, GPIO_Pin_12},
        {GPIOD, GPIO_Pin_13},
        {GPIOD, GPIO_Pin_14},
        {GPIOD, GPIO_Pin_15},
#else
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
#endif /* MR_USING_GPIOD */
#ifdef MR_USING_GPIOE
        {GPIOE, GPIO_Pin_0},
        {GPIOE, GPIO_Pin_1},
        {GPIOE, GPIO_Pin_2},
        {GPIOE, GPIO_Pin_3},
        {GPIOE, GPIO_Pin_4},
        {GPIOE, GPIO_Pin_5},
        {GPIOE, GPIO_Pin_6},
        {GPIOE, GPIO_Pin_7},
        {GPIOE, GPIO_Pin_8},
        {GPIOE, GPIO_Pin_9},
        {GPIOE, GPIO_Pin_10},
        {GPIOE, GPIO_Pin_11},
        {GPIOE, GPIO_Pin_12},
        {GPIOE, GPIO_Pin_13},
        {GPIOE, GPIO_Pin_14},
        {GPIOE, GPIO_Pin_15},
#else
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
        {MR_NULL, 0},
#endif /* MR_USING_GPIOE */
    };

static struct mr_gpio gpio_dev;

static struct drv_gpio_data *drv_gpio_get_data(int pin)
{
    if ((pin >= mr_array_num(gpio_drv_data)) || (gpio_drv_data[pin].port == MR_NULL))
    {
        return NULL;
    }
    return &gpio_drv_data[pin];
}

static int drv_gpio_configure(struct mr_gpio *gpio, int pin, int mode)
{
    struct drv_gpio_data *gpio_data = drv_gpio_get_data(pin);
    uint32_t exti_line = pin % 16;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    /* Check pin is valid */
    if (gpio_data == MR_NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
#ifdef MR_USING_GPIOA
    if (gpio_data->port == GPIOA)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
#endif /* MR_USING_GPIOA */
#ifdef MR_USING_GPIOB
    if (gpio_data->port == GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
#endif /* MR_USING_GPIOB */
#ifdef MR_USING_GPIOC
    if (gpio_data->port == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
#endif /* MR_USING_GPIOC */
#ifdef MR_USING_GPIOD
    if (gpio_data->port == GPIOD)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
#endif /* MR_USING_GPIOD */
#ifdef MR_USING_GPIOE
    if (gpio_data->port == GPIOE)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    }
#endif /* MR_USING_GPIOE */

    switch (mode)
    {
        case MR_GPIO_MODE_NONE:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }

        case MR_GPIO_MODE_OUTPUT:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            break;
        }

        case MR_GPIO_MODE_OUTPUT_OD:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
            break;
        }

        case MR_GPIO_MODE_INPUT:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }

        case MR_GPIO_MODE_INPUT_DOWN:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
            break;
        }

        case MR_GPIO_MODE_INPUT_UP:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            break;
        }

        case MR_GPIO_MODE_IRQ_RISING:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
            break;
        }

        case MR_GPIO_MODE_IRQ_FALLING:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
            break;
        }

        case MR_GPIO_MODE_IRQ_EDGE:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            break;
        }

        default:
        {
            break;
        }
    }

    /* Configure EXTI */
    if (mode >= MR_GPIO_MODE_IRQ_RISING)
    {
        if ((gpio_irq_mask[exti_line] != -1) && (gpio_irq_mask[exti_line] != pin))
        {
            return MR_EBUSY;
        }

        gpio_irq_mask[exti_line] = pin;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

        EXTI_InitStructure.EXTI_Line = gpio_data->pin;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        GPIO_EXTILineConfig(pin / 16, exti_line);
        EXTI_Init(&EXTI_InitStructure);

        /* Configure NVIC */
        NVIC_InitStructure.NVIC_IRQChannel = gpio_irq_map[exti_line];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } else if (pin == gpio_irq_mask[exti_line])
    {
        if ((exti_line >= 5) && (exti_line <= 9))
        {
            if ((gpio_irq_mask[5] == -1) && (gpio_irq_mask[6] == -1) && (gpio_irq_mask[7] == -1) &&
                (gpio_irq_mask[8] == -1) && (gpio_irq_mask[9] == -1))
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        } else
        {
            if ((gpio_irq_mask[10] == -1) && (gpio_irq_mask[11] == -1) && (gpio_irq_mask[12] == -1) &&
                (gpio_irq_mask[13] == -1) && (gpio_irq_mask[14] == -1) && (gpio_irq_mask[15] == -1))
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        }
        gpio_irq_mask[exti_line] = -1;

        EXTI_InitStructure.EXTI_Line = gpio_data->pin;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        GPIO_EXTILineConfig(pin / 16, exti_line);
        EXTI_Init(&EXTI_InitStructure);
    }

    /* Configure GPIO */
    GPIO_InitStructure.GPIO_Pin = gpio_data->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_data->port, &GPIO_InitStructure);
    return MR_EOK;
}

static int drv_gpio_read(struct mr_gpio *gpio, int pin)
{
    struct drv_gpio_data *gpio_data = drv_gpio_get_data(pin);

    /* Check pin is valid */
    if (gpio_data == NULL)
    {
        return 0;
    }
    return (int)GPIO_ReadInputDataBit(gpio_data->port, gpio_data->pin);
}

static void drv_gpio_write(struct mr_gpio *gpio, int pin, int value)
{
    struct drv_gpio_data *gpio_data = drv_gpio_get_data(pin);

    /* Check pin is valid */
    if (gpio_data == NULL)
    {
        return;
    }
    GPIO_WriteBit(gpio_data->port, gpio_data->pin, value);
}

void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[1]);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[2]);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[3]);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[4]);
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void)
{
    if ((EXTI_GetITStatus(EXTI_Line5) != RESET)
        || (EXTI_GetITStatus(EXTI_Line6) != RESET)
        || (EXTI_GetITStatus(EXTI_Line7) != RESET)
        || (EXTI_GetITStatus(EXTI_Line8) != RESET)
        || (EXTI_GetITStatus(EXTI_Line9) != RESET))
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[5]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[6]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[7]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[8]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[9]);
        EXTI_ClearITPendingBit(EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9);
    }
}

void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI_GetITStatus(EXTI_Line10) != RESET)
        || (EXTI_GetITStatus(EXTI_Line11) != RESET)
        || (EXTI_GetITStatus(EXTI_Line12) != RESET)
        || (EXTI_GetITStatus(EXTI_Line13) != RESET)
        || (EXTI_GetITStatus(EXTI_Line14) != RESET)
        || (EXTI_GetITStatus(EXTI_Line15) != RESET))
    {
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[10]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[11]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[12]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[13]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[14]);
        mr_dev_isr(&gpio_dev.dev, MR_ISR_EVENT_RD_INTER, &gpio_irq_mask[15]);
        EXTI_ClearITPendingBit(EXTI_Line10 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
    }
}

static struct mr_gpio_ops gpio_drv_ops =
    {
        drv_gpio_configure,
        drv_gpio_read,
        drv_gpio_write
    };

static struct mr_drv gpio_drv =
    {
        Mr_Drv_Type_Gpio,
        &gpio_drv_ops,
        &gpio_drv_ops
    };

int drv_gpio_init(void)
{
    return mr_gpio_register(&gpio_dev, "gpio", &gpio_drv);
}
MR_INIT_DRV_EXPORT(drv_gpio_init);

#endif /* MR_USING_GPIO */
