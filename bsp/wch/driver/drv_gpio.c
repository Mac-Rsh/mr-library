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

#if (MR_CONF_PIN == MR_CONF_ENABLE)

#define PIN_PORT(pin)       ((uint8_t)(((pin) >> 4) & 0x0Fu))
#define PIN_STPORT(pin)     ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)      ((uint16_t)(1u << (mr_uint8_t)(pin & 0x0Fu)))
#define PIN_RCC(pin)        (RCC_APB2Periph_GPIOA << PIN_PORT(pin))

static IRQn_Type irqno[] =
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

static mr_int32_t mask[16] = {-1,
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

static mr_err_t ch32_pin_configure(mr_pin_t pin, struct mr_pin_config *config)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(PIN_RCC(config->number), ENABLE);

    /* Configure GPIO_InitStructure */
    GPIO_InitStructure.GPIO_Pin = PIN_STPIN(config->number);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

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
            return -MR_ERR_GENERIC;
    }

    if (config->mode >= MR_PIN_MODE_IRQ_RISING)
    {
        if ((mask[config->number % 16] != -1 && mask[config->number % 16] != config->number))
        {
            return -MR_ERR_BUSY;
        }

        mask[config->number % 16] = config->number;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

        EXTI_InitStructure.EXTI_Line = PIN_STPIN(config->number);
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;

        GPIO_EXTILineConfig(PIN_PORT(config->number), config->number % 16);
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = irqno[config->number % 16];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } else if (config->number == mask[config->number % 16])
    {
        if (config->number % 16 >= 5 && config->number % 16 <= 9)
        {
            if (mask[5] == -1 && mask[6] == -1 && mask[7] == -1 && mask[8] == -1 && mask[9] == -1)
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }

        } else if (config->number % 16 >= 10 && config->number % 16 <= 15)
        {
            if (mask[10] == -1 && mask[11] == -1 && mask[12] == -1 && mask[13] == -1 && mask[14] == -1
                && mask[15] == -1)
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        } else
        {
            EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        }

        mask[config->number % 16] = -1;

        EXTI_InitStructure.EXTI_Line = PIN_STPIN(config->number);
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

        GPIO_EXTILineConfig(PIN_PORT(config->number), config->number % 16);
        EXTI_Init(&EXTI_InitStructure);
    }

    GPIO_Init(PIN_STPORT(config->number), &GPIO_InitStructure);

    return MR_ERR_OK;
}

static void ch32_pin_write(mr_pin_t pin, mr_pos_t number, mr_level_t level)
{
    GPIO_WriteBit(PIN_STPORT(number), PIN_STPIN(number), level);
}

static mr_level_t ch32_pin_read(mr_pin_t pin, mr_pos_t number)
{
    return GPIO_ReadInputDataBit(PIN_STPORT(number), PIN_STPIN(number));
}

void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[1]);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[2]);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[3]);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[4]);
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
        mr_pin_device_isr(&pin_device, mask[5]);
        mr_pin_device_isr(&pin_device, mask[6]);
        mr_pin_device_isr(&pin_device, mask[7]);
        mr_pin_device_isr(&pin_device, mask[8]);
        mr_pin_device_isr(&pin_device, mask[9]);
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
        mr_pin_device_isr(&pin_device, mask[10]);
        mr_pin_device_isr(&pin_device, mask[11]);
        mr_pin_device_isr(&pin_device, mask[12]);
        mr_pin_device_isr(&pin_device, mask[13]);
        mr_pin_device_isr(&pin_device, mask[14]);
        EXTI_ClearITPendingBit(EXTI_Line10 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
    }
}

mr_err_t ch32_gpio_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    static struct mr_pin_ops driver =
            {
                    ch32_pin_configure,
                    ch32_pin_write,
                    ch32_pin_read,
            };

    ret = mr_pin_device_add(&pin_device, "pin", MR_NULL, &driver);
    MR_ASSERT(ret == MR_ERR_OK);

    return MR_ERR_OK;
}
AUTO_INIT_DRIVER_EXPORT(ch32_gpio_init);

#endif /* MR_CONF_PIN */