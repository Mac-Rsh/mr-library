/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "drv_pin.h"

#ifdef MR_USING_PIN

#if !defined(MR_USING_GPIOA) && !defined(MR_USING_GPIOB) && !defined(MR_USING_GPIOC) && !defined(MR_USING_GPIOD) && !defined(MR_USING_GPIOE)
#error "Please define at least one GPIO macro like MR_USING_GPIOA. Otherwise undefine MR_USING_GPIO."
#else

static IRQn_Type pin_irq_map[] =
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

static int pin_irq_mask[] =
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

static struct drv_pin_port_data pin_port_drv_data[] =
    {
#ifdef MR_USING_GPIOA
        {GPIOA},
#else
        {MR_NULL},
#endif /* MR_USING_GPIOA */
#ifdef MR_USING_GPIOB
        {GPIOB},
#else
        {MR_NULL},
#endif /* MR_USING_GPIOB */
#ifdef MR_USING_GPIOC
        {GPIOC},
#else
        {MR_NULL},
#endif /* MR_USING_GPIOC */
#ifdef MR_USING_GPIOD
        {GPIOD},
#else
        {MR_NULL},
#endif /* MR_USING_GPIOD */
#ifdef MR_USING_GPIOE
        {GPIOE},
#else
        {MR_NULL},
#endif /* MR_USING_GPIOE */
    };

static struct drv_pin_data pin_drv_data[] =
    {
        {GPIO_PIN_0},
        {GPIO_PIN_1},
        {GPIO_PIN_2},
        {GPIO_PIN_3},
        {GPIO_PIN_4},
        {GPIO_PIN_5},
        {GPIO_PIN_6},
        {GPIO_PIN_7},
        {GPIO_PIN_8},
        {GPIO_PIN_9},
        {GPIO_PIN_10},
        {GPIO_PIN_11},
        {GPIO_PIN_12},
        {GPIO_PIN_13},
        {GPIO_PIN_14},
        {GPIO_PIN_15},
    };

static struct mr_pin pin_dev;

static struct drv_pin_port_data *drv_pin_get_port_data(int pin)
{
    pin /= 16;
    if ((pin >= mr_array_num(pin_port_drv_data)) || (pin_port_drv_data[pin].port == MR_NULL))
    {
        return MR_NULL;
    }
    return &pin_port_drv_data[pin];
}

static struct drv_pin_data *drv_pin_get_data(int pin)
{
    pin %= 16;
    if (pin >= mr_array_num(pin_drv_data))
    {
        return MR_NULL;
    }
    return &pin_drv_data[pin];
}

static int drv_pin_configure(struct mr_pin *pin, int number, int mode)
{
    struct drv_pin_port_data *pin_port_data = drv_pin_get_port_data(number);
    struct drv_pin_data *pin_data = drv_pin_get_data(number);
    uint32_t exti_line = number % 16;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* Check pin is valid */
    if (pin_port_data == MR_NULL || pin_data == MR_NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
#ifdef MR_USING_GPIOA
    if (pin_port_data->port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
#endif /* MR_USING_GPIOA */
#ifdef MR_USING_GPIOB
    if (pin_port_data->port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
#endif /* MR_USING_GPIOB */
#ifdef MR_USING_GPIOC
    if (pin_port_data->port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
#endif /* MR_USING_GPIOC */
#ifdef MR_USING_GPIOD
    if (pin_port_data->port == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
#endif /* MR_USING_GPIOD */
#ifdef MR_USING_GPIOE
    if (pin_port_data->port == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
#endif /* MR_USING_GPIOE */

    switch (mode)
    {
        case MR_PIN_MODE_NONE:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
						GPIO_InitStructure.Pull = GPIO_NOPULL;
            break;
        }

        case MR_PIN_MODE_OUTPUT:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
						GPIO_InitStructure.Pull = GPIO_NOPULL;
            break;
        }

        case MR_PIN_MODE_OUTPUT_OD:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStructure.Pull = GPIO_NOPULL;
						break;
        }

        case MR_PIN_MODE_INPUT:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
            GPIO_InitStructure.Pull = GPIO_NOPULL;
            break;
        }

        case MR_PIN_MODE_INPUT_DOWN:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
            GPIO_InitStructure.Pull = GPIO_PULLDOWN;
            break;
        }

        case MR_PIN_MODE_INPUT_UP:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
            GPIO_InitStructure.Pull = GPIO_PULLUP;
            break;
        }

        case MR_PIN_MODE_IRQ_RISING:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
            GPIO_InitStructure.Pull = GPIO_PULLDOWN;
            break;
        }

        case MR_PIN_MODE_IRQ_FALLING:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
            GPIO_InitStructure.Pull = GPIO_PULLUP;
            break;
        }

        case MR_PIN_MODE_IRQ_EDGE:
        {
            GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
            GPIO_InitStructure.Pull = GPIO_NOPULL;
            break;
        }

        default:
        {
            return MR_EINVAL;
        }
    }

    /* Configure EXTI */
    if (mode >= MR_PIN_MODE_IRQ_RISING)
    {
        if ((pin_irq_mask[exti_line] != -1) && (pin_irq_mask[exti_line] != number))
        {
            return MR_EBUSY;
        }

        pin_irq_mask[exti_line] = number;

        HAL_NVIC_SetPriority(pin_irq_map[exti_line], 5, 0);
        HAL_NVIC_EnableIRQ(pin_irq_map[exti_line]);
    } else if (number == pin_irq_mask[exti_line])
    {
        if ((exti_line >= 5) && (exti_line <= 9))
        {
            if ((pin_irq_mask[5] == -1) && (pin_irq_mask[6] == -1) && (pin_irq_mask[7] == -1) &&
                (pin_irq_mask[8] == -1) && (pin_irq_mask[9] == -1))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[exti_line]);
            }
        } else
        {
            if ((pin_irq_mask[10] == -1) && (pin_irq_mask[11] == -1) && (pin_irq_mask[12] == -1) &&
                (pin_irq_mask[13] == -1) && (pin_irq_mask[14] == -1) && (pin_irq_mask[15] == -1))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[exti_line]);
            }
        }
        pin_irq_mask[exti_line] = -1;
    }

    /* Configure GPIO */
    GPIO_InitStructure.Pin = pin_data->pin;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pin_port_data->port, &GPIO_InitStructure);
    return MR_EOK;
}

static uint8_t drv_pin_read(struct mr_pin *pin, int number)
{
    struct drv_pin_port_data *pin_port_data = drv_pin_get_port_data(number);
    struct drv_pin_data *pin_data = drv_pin_get_data(number);

    /* Check pin is valid */
    if (pin_port_data == NULL || pin_data == NULL)
    {
        return 0;
    }
    return (int)HAL_GPIO_ReadPin(pin_port_data->port, pin_data->pin);
}

static void drv_pin_write(struct mr_pin *pin, int number, uint8_t value)
{
    struct drv_pin_port_data *pin_port_data = drv_pin_get_port_data(number);
    struct drv_pin_data *pin_data = drv_pin_get_data(number);

    /* Check pin is valid */
    if (pin_port_data == NULL || pin_data == NULL)
    {
        return;
    }
    HAL_GPIO_WritePin(pin_port_data->port, pin_data->pin, (GPIO_PinState)value);
}

void EXTI0_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[0]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET)
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[1]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    }
}

void EXTI2_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[2]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET)
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[3]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
    }
}

void EXTI4_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[4]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if ((__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_8) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET))
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[5]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[6]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[7]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[8]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[9]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if ((__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET)
        || (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET))
    {
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[10]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[11]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[12]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[13]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[14]);
        mr_dev_isr(&pin_dev.dev, MR_ISR_PIN_EXTI_INT, &pin_irq_mask[15]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
    }
}

static struct mr_pin_ops pin_drv_ops =
    {
        drv_pin_configure,
        drv_pin_read,
        drv_pin_write
    };

static struct mr_drv pin_drv =
    {
        Mr_Drv_Type_Pin,
        &pin_drv_ops,
        MR_NULL
    };

int drv_pin_init(void)
{
    return mr_pin_register(&pin_dev, "pin", &pin_drv);
}
MR_DRV_EXPORT(drv_pin_init);

#endif /* !defined(MR_USING_GPIOA) && !defined(MR_USING_GPIOB) && !defined(MR_USING_GPIOC) && !defined(MR_USING_GPIOD) && !defined(MR_USING_GPIOE) */

#endif /* MR_USING_PIN */
