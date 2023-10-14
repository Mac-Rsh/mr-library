/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#include "drv_gpio.h"

#if (MR_CFG_PIN == MR_CFG_ENABLE)

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))

static IRQn_Type irq[] =
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

static mr_off_t irq_mask[16] = {-1,
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
                                -1};

static struct drv_gpio_data drv_gpio_data[] =
    {
        {"pin"},
    };

static struct mr_pin pin_device;

static mr_err_t drv_pin_configure(mr_pin_t pin, mr_pin_config_t config)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    mr_uint32_t exti_line = config->number % 16;

    if (config->number < 0)
    {
        return MR_ERR_INVALID;
    }

#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    __HAL_RCC_GPIOA_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    __HAL_RCC_GPIOC_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
#ifdef SOC_SERIES_STM32L4
    HAL_PWREx_EnableVddIO2();
#endif
    __HAL_RCC_GPIOG_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    __HAL_RCC_GPIOH_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    __HAL_RCC_GPIOI_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    __HAL_RCC_GPIOJ_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    __HAL_RCC_GPIOK_CLK_ENABLE();
#endif

    switch (config->mode)
    {
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
            break;
    }

    if (config->mode >= MR_PIN_MODE_IRQ_RISING)
    {
        if ((irq_mask[exti_line] != -1 && irq_mask[exti_line] != config->number))
        {
            return MR_ERR_BUSY;
        }

        irq_mask[exti_line] = config->number;
        HAL_NVIC_SetPriority(irq[exti_line], 5, 0);
        HAL_NVIC_EnableIRQ(irq[exti_line]);
    } else if (config->number == irq_mask[config->number % 16])
    {
        if (exti_line >= 5 && exti_line <= 9)
        {
            if (irq_mask[5] == -1 && irq_mask[6] == -1 && irq_mask[7] == -1 && irq_mask[8] == -1 && irq_mask[9] == -1)
            {
                HAL_NVIC_DisableIRQ(irq[exti_line]);
            }
        } else
        {
            if (irq_mask[10] == -1 && irq_mask[11] == -1 && irq_mask[12] == -1 && irq_mask[13] == -1 &&
                irq_mask[14] == -1
                && irq_mask[15] == -1)
            {
                HAL_NVIC_DisableIRQ(irq[exti_line]);
            }
        }

        irq_mask[exti_line] = -1;
    }

    GPIO_InitStructure.Pin = PIN_STPIN(config->number);
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

    if (config->mode != MR_PIN_MODE_NONE)
    {
        HAL_GPIO_Init(PIN_STPORT(config->number), &GPIO_InitStructure);
    } else
    {
        HAL_GPIO_DeInit(PIN_STPORT(config->number), PIN_STPIN(config->number));
    }

    return MR_ERR_OK;
}

static mr_level_t drv_pin_read(mr_pin_t pin, mr_off_t number)
{
    if (number > MR_BSP_PIN_NUMBER)
    {
        return 0;
    }

    return (mr_level_t)HAL_GPIO_ReadPin(PIN_STPORT(number), PIN_STPIN(number));
}

static void drv_pin_write(mr_pin_t pin, mr_off_t number, mr_level_t level)
{
    if (number > MR_BSP_PIN_NUMBER)
    {
        return;
    }

    HAL_GPIO_WritePin(PIN_STPORT(number), PIN_STPIN(number), (GPIO_PinState)level);
}

void EXTI0_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[0]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[1]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    }
}

void EXTI2_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[2]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[3]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
    }
}

void EXTI4_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[4]);
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
        mr_pin_device_isr(&pin_device, irq_mask[5]);
        mr_pin_device_isr(&pin_device, irq_mask[6]);
        mr_pin_device_isr(&pin_device, irq_mask[7]);
        mr_pin_device_isr(&pin_device, irq_mask[8]);
        mr_pin_device_isr(&pin_device, irq_mask[9]);
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
        mr_pin_device_isr(&pin_device, irq_mask[10]);
        mr_pin_device_isr(&pin_device, irq_mask[11]);
        mr_pin_device_isr(&pin_device, irq_mask[12]);
        mr_pin_device_isr(&pin_device, irq_mask[13]);
        mr_pin_device_isr(&pin_device, irq_mask[14]);
        mr_pin_device_isr(&pin_device, irq_mask[15]);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
    }
}

mr_err_t drv_gpio_init(void)
{
    static struct mr_pin_ops drv_ops =
        {
            drv_pin_configure,
            drv_pin_read,
            drv_pin_write,
        };
    mr_err_t ret = MR_ERR_OK;

    ret = mr_pin_device_add(&pin_device, drv_gpio_data[0].name, &drv_ops, &drv_gpio_data[0]);
    MR_ASSERT(ret == MR_ERR_OK);

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_gpio_init);

#endif
