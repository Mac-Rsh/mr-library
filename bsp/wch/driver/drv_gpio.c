/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     MacRsh       first version
 */

#include "drv_gpio.h"

#if (MR_CFG_PIN == MR_CFG_ENABLE)

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))
#define PIN_RCC(pin)        (RCC_APB2Periph_GPIOA << PIN_PORT(pin))

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

static struct mr_pin pin_device;

static mr_err_t drv_pin_configure(mr_pin_t pin, mr_pin_config_t config)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    mr_uint32_t exti_line = config->number % 16;

    if (config->number < 0)
    {
        return MR_ERR_INVALID;
    }

    RCC_APB2PeriphClockCmd(PIN_RCC(config->number), ENABLE);

    switch (config->mode)
    {
        case MR_PIN_MODE_OUTPUT:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            break;
        }

        case MR_PIN_MODE_OUTPUT_OD:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
            break;
        }

        case MR_PIN_MODE_INPUT:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }

        case MR_PIN_MODE_INPUT_DOWN:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
            break;
        }

        case MR_PIN_MODE_INPUT_UP:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            break;
        }

        case MR_PIN_MODE_IRQ_RISING:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
            break;
        }

        case MR_PIN_MODE_IRQ_FALLING:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
            break;
        }

        case MR_PIN_MODE_IRQ_EDGE:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            break;
        }

        default:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }
    }

    if (config->mode >= MR_PIN_MODE_IRQ_RISING)
    {
        if ((irq_mask[exti_line] != -1 && irq_mask[exti_line] != config->number))
        {
            return MR_ERR_BUSY;
        }

        irq_mask[exti_line] = config->number;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

        EXTI_InitStructure.EXTI_Line = PIN_STPIN(config->number);
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        GPIO_EXTILineConfig(PIN_PORT(config->number), exti_line);
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = irq[exti_line];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } else if (config->number == irq_mask[config->number % 16])
    {
        if (exti_line >= 5 && exti_line <= 9)
        {
            if (irq_mask[5] == -1 && irq_mask[6] == -1 && irq_mask[7] == -1 && irq_mask[8] == -1 && irq_mask[9] == -1)
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        } else
        {
            if (irq_mask[10] == -1 && irq_mask[11] == -1 && irq_mask[12] == -1 && irq_mask[13] == -1 &&
                irq_mask[14] == -1
                && irq_mask[15] == -1)
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        }

        irq_mask[exti_line] = -1;

        EXTI_InitStructure.EXTI_Line = PIN_STPIN(config->number);
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        GPIO_EXTILineConfig(PIN_PORT(config->number), exti_line);
        EXTI_Init(&EXTI_InitStructure);
    }

    GPIO_InitStructure.GPIO_Pin = PIN_STPIN(config->number);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PIN_STPORT(config->number), &GPIO_InitStructure);

    return MR_ERR_OK;
}

static mr_level_t drv_pin_read(mr_pin_t pin, mr_off_t number)
{
    if (number > MR_BSP_PIN_NUMBER)
    {
        return 0;
    }

    return (mr_level_t)GPIO_ReadInputDataBit(PIN_STPORT(number), PIN_STPIN(number));
}

static void drv_pin_write(mr_pin_t pin, mr_off_t number, mr_level_t level)
{
    if (number > MR_BSP_PIN_NUMBER)
    {
        return;
    }

    GPIO_WriteBit(PIN_STPORT(number), PIN_STPIN(number), (BitAction)level);
}

void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[1]);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[2]);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[3]);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        mr_pin_device_isr(&pin_device, irq_mask[4]);
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
        mr_pin_device_isr(&pin_device, irq_mask[5]);
        mr_pin_device_isr(&pin_device, irq_mask[6]);
        mr_pin_device_isr(&pin_device, irq_mask[7]);
        mr_pin_device_isr(&pin_device, irq_mask[8]);
        mr_pin_device_isr(&pin_device, irq_mask[9]);
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
        mr_pin_device_isr(&pin_device, irq_mask[10]);
        mr_pin_device_isr(&pin_device, irq_mask[11]);
        mr_pin_device_isr(&pin_device, irq_mask[12]);
        mr_pin_device_isr(&pin_device, irq_mask[13]);
        mr_pin_device_isr(&pin_device, irq_mask[14]);
        EXTI_ClearITPendingBit(EXTI_Line10 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
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

    ret = mr_pin_device_add(&pin_device, "pin", &drv_ops, MR_NULL);
    MR_ASSERT(ret == MR_ERR_OK);

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_gpio_init);

#endif
