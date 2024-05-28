/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-11    MacRsh       First version
 */

#include "../mr-library/driver/include/mr_pin.h"

#ifdef MR_USE_PIN

static IRQn_Type _pin_driver_irq[] = _DRIVER_PIN_IRQ_CONFIG;
static int _pin_irq_mask[] = {-1, -1, -1, -1, -1, -1, -1, -1,
                              -1, -1, -1, -1, -1, -1, -1, -1};
static struct mr_pin_driver_port _pin_driver_port[] = _DRIVER_PIN_PORT_CONFIG;
static struct mr_pin_driver_pin _pin_driver_pin[] = _DRIVER_PIN_PIN_CONFIG;

static struct mr_driver _pin_driver;
static struct mr_pin _pin_device;

MR_INLINE struct mr_pin_driver_port *_pin_port_get(uint32_t number)
{
    number >>= 4;
    if ((number >= MR_ARRAY_NUM(_pin_driver_port)) ||
        (_pin_driver_port[number].port == NULL))
    {
        return NULL;
    }
    return &_pin_driver_port[number];
}

MR_INLINE struct mr_pin_driver_pin *_pin_pin_get(uint32_t number)
{
    number &= 0x0f;
    if (number >= MR_ARRAY_NUM(_pin_driver_pin))
    {
        return NULL;
    }
    return &_pin_driver_pin[number];
}

static int pin_driver_configure(struct mr_driver *driver, uint32_t number,
                                uint32_t mode)
{
    struct mr_pin_driver_port *pin_port = _pin_port_get(number);
    struct mr_pin_driver_pin *pin_pin = _pin_pin_get(number);
    uint32_t exti_line = number & 0x0f;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    if (pin_port == NULL || pin_pin == NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
#ifdef GPIOA
    if (pin_port->port == GPIOA)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
#endif /* GPIOA */
#ifdef GPIOB
    if (pin_port->port == GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
#endif /* GPIOB */
#ifdef GPIOC
    if (pin_port->port == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
#endif /* GPIOC */
#ifdef GPIOD
    if (pin_port->port == GPIOD)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
#endif /* GPIOD */
#ifdef GPIOE
    if (pin_port->port == GPIOE)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    }
#endif /* GPIOE */

    switch (mode)
    {
        case MR_PIN_MODE_NONE:
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }
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
            return MR_EINVAL;
        }
    }

    /* Configure EXTI */
    if (mode >= MR_PIN_MODE_IRQ_RISING)
    {
        if ((_pin_irq_mask[exti_line] != -1) &&
            (_pin_irq_mask[exti_line] != number))
        {
            return MR_EBUSY;
        }
        _pin_irq_mask[exti_line] = number;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        EXTI_InitStructure.EXTI_Line = pin_pin->pin;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        GPIO_EXTILineConfig(number >> 4, exti_line);
        EXTI_Init(&EXTI_InitStructure);

        /* Configure NVIC */
        NVIC_InitStructure.NVIC_IRQChannel = _pin_driver_irq[exti_line];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } else if (number == _pin_irq_mask[exti_line])
    {
#ifdef MR_USING_CH32V00X
        if ((_pin_irq_mask[0] == -1) && (_pin_irq_mask[1] == -1) &&
            (_pin_irq_mask[2] == -1) && (_pin_irq_mask[3] == -1) &&
            (_pin_irq_mask[4] == -1) && (_pin_irq_mask[5] == -1) &&
            (_pin_irq_mask[6] == -1) && (_pin_irq_mask[7] == -1))
        {
            EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        } else
        {
            EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        }
#else
        if ((exti_line >= 5) && (exti_line <= 9))
        {
            if ((_pin_irq_mask[5] == -1) && (_pin_irq_mask[6] == -1) &&
                (_pin_irq_mask[7] == -1) && (_pin_irq_mask[8] == -1) &&
                (_pin_irq_mask[9] == -1))
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        } else
        {
            if ((_pin_irq_mask[10] == -1) && (_pin_irq_mask[11] == -1) &&
                (_pin_irq_mask[12] == -1) && (_pin_irq_mask[13] == -1) &&
                (_pin_irq_mask[14] == -1) && (_pin_irq_mask[15] == -1))
            {
                EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            } else
            {
                EXTI_InitStructure.EXTI_LineCmd = ENABLE;
            }
        }
#endif /* MR_USING_CH32V00X */
        _pin_irq_mask[exti_line] = -1;

        EXTI_InitStructure.EXTI_Line = pin_pin->pin;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        GPIO_EXTILineConfig(number >> 4, exti_line);
        EXTI_Init(&EXTI_InitStructure);
    }

    /* Configure GPIO */
    GPIO_InitStructure.GPIO_Pin = pin_pin->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(pin_port->port, &GPIO_InitStructure);
    return MR_EOK;
}

static int pin_driver_read(struct mr_driver *driver, uint32_t number,
                           uint8_t *value)
{
    struct mr_pin_driver_port *pin_port = _pin_port_get(number);
    struct mr_pin_driver_pin *pin_pin = _pin_pin_get(number);

    if (pin_port == NULL || pin_pin == NULL)
    {
        return MR_EINVAL;
    }

    *value = GPIO_ReadInputDataBit(pin_port->port, pin_pin->pin);
    return MR_EOK;
}

static int pin_driver_write(struct mr_driver *driver, uint32_t number,
                            uint8_t value)
{
    struct mr_pin_driver_port *pin_port = _pin_port_get(number);
    struct mr_pin_driver_pin *pin_pin = _pin_pin_get(number);

    if (pin_port == NULL || pin_pin == NULL)
    {
        return MR_EINVAL;
    }

    GPIO_WriteBit(pin_port->port, pin_pin->pin, value);
    return MR_EOK;
}

#ifdef MR_USING_CH32V00X
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[1]);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[2]);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[3]);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[4]);
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[5]);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[6]);
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[7]);
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
}
#else
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[1]);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[2]);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[3]);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[4]);
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[5]);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[6]);
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[7]);
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[8]);
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[9]);
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}

void EXTI15_10_IRQHandler(void)
    __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[10]);
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[11]);
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[12]);
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[13]);
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[14]);
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        mr_device_isr((struct mr_device *)&_pin_device, MR_EVENT_PIN_EXTI_INT,
                      &_pin_irq_mask[15]);
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}
#endif /* MR_USING_CH32V00X */

static void pin_driver_init(void)
{
    static struct mr_pin_driver_ops ops = {.configure = pin_driver_configure,
                                           .read = pin_driver_read,
                                           .write = pin_driver_write};

    _pin_driver.ops = &ops;
    mr_pin_register(&_pin_device, "pin", &_pin_driver);
}
MR_INIT_DRIVER_EXPORT(pin_driver_init);

#endif /* MR_USE_PIN */
