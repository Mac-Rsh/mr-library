/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "include/device/mr_pin.h"

#ifdef MR_USING_PIN

/**
 * @brief Pin irq structure.
 */
struct pin_irq
{
    int number;                                                     /**< Pin number */
    int desc;                                                       /**< Device descriptor */
    int (*call)(int desc, void *args);                              /**< Callback function */
    struct mr_list list;                                            /**< List */
};

static int pin_set_mode(struct mr_pin *pin, int number, int mode)
{
    struct mr_pin_ops *ops = (struct mr_pin_ops *)pin->dev.drv->ops;

    /* Check number is valid */
    if (number < 0)
    {
        return MR_EINVAL;
    }

    /* Configure pin mode */
    int ret = ops->configure(pin, number, mode);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* If the irq exists, update it */
    for (struct mr_list *list = pin->irq_list.next; list != &pin->irq_list; list = list->next)
    {
        struct pin_irq *irq = (struct pin_irq *)MR_CONTAINER_OF(list, struct pin_irq, list);
        if (irq->number == number)
        {
            if (mode < MR_PIN_MODE_IRQ_RISING)
            {
                /* Remove irq */
                mr_list_remove(list);
                mr_free(irq);
            } else
            {
                /* Update irq */
                irq->desc = pin->dev.rd_call.desc;
                irq->call = pin->dev.rd_call.call;
            }

            /* Enable interrupt */
            mr_interrupt_enable();
            return MR_EOK;
        }
    }

    /* If not exist, allocate new irq */
    if (mode >= MR_PIN_MODE_IRQ_RISING)
    {
        /* Allocate irq */
        struct pin_irq *irq = (struct pin_irq *)mr_malloc(sizeof(struct pin_irq));
        if (irq == MR_NULL)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return MR_ENOMEM;
        }
        irq->number = number;
        irq->desc = pin->dev.rd_call.desc;
        irq->call = pin->dev.rd_call.call;
        mr_list_init(&irq->list);
        mr_list_insert_before(&pin->irq_list, &irq->list);

        /* Clear call */
        pin->dev.rd_call.call = MR_NULL;
    }

    /* Enable interrupt */
    mr_interrupt_enable();
    return MR_EOK;
}

static ssize_t mr_pin_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *rd_buf = (uint8_t *)buf;
    ssize_t rd_size;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = (uint8_t)ops->read(pin, off);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_pin_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_pin *pin = (struct mr_pin *)dev;
    struct mr_pin_ops *ops = (struct mr_pin_ops *)dev->drv->ops;
    uint8_t *wr_buf = (uint8_t *)buf;
    ssize_t wr_size;

    /* Check offset is valid */
    if (off < 0)
    {
        return MR_EINVAL;
    }

    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        ops->write(pin, off, *wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_pin_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)dev;

    switch (cmd)
    {
        case MR_CTL_PIN_SET_MODE:
        {
            if (args != MR_NULL)
            {
                struct mr_pin_config config = *((struct mr_pin_config *)args);

                return pin_set_mode(pin, off, config.mode);
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_pin_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_pin *pin = (struct mr_pin *)dev;

    switch (event)
    {
        case MR_ISR_PIN_EXTI_INT:
        {
            ssize_t number = *(int *)args;

            /* If the irq exists, call it */
            for (struct mr_list *list = pin->irq_list.next; list != &pin->irq_list; list = list->next)
            {
                struct pin_irq *irq = (struct pin_irq *)MR_CONTAINER_OF(list, struct pin_irq, list);
                if (irq->number == number)
                {
                    irq->call(irq->desc, &number);
                    return MR_EEXIST;
                }
            }
            return MR_ENOTFOUND;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers a pin.
 *
 * @param pin The pin.
 * @param name The name of the pin.
 * @param drv The driver of the pin.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_pin_register(struct mr_pin *pin, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            MR_NULL,
            MR_NULL,
            mr_pin_read,
            mr_pin_write,
            mr_pin_ioctl,
            mr_pin_isr
        };

    MR_ASSERT(pin != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    mr_list_init(&pin->irq_list);

    /* Register the pin */
    return mr_dev_register(&pin->dev, name, Mr_Dev_Type_Pin, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_PIN */
