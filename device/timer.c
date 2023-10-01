/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "timer.h"

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

static mr_err_t err_io_timer_configure(mr_timer_t timer, mr_state_t state)
{
    return -MR_ERR_IO;
}

static void err_io_timer_start(mr_timer_t timer, mr_uint32_t prescaler, mr_uint32_t period)
{

}

static void err_io_timer_stop(mr_timer_t timer)
{

}

static mr_uint32_t err_io_timer_get_count(mr_timer_t timer)
{
    return 0;
}

static mr_err_t mr_timer_calculate(mr_timer_t timer, mr_uint32_t timeout, mr_uint32_t *prescaler, mr_uint32_t *period)
{
    mr_uint32_t clk_mhz = 0, temp_period = 0, temp_reload = 0, error = 0, error_min = timeout;
    mr_uint32_t i = 0;

    /* Check the clock */
    clk_mhz = timer->data->clk / 1000000u;
    if (clk_mhz == 0)
    {
        return -MR_ERR_GENERIC;
    }

    /* Calculate the prescaler */
    *prescaler = clk_mhz;
    for (; *prescaler <= (timer->data->prescaler_max / 10); *prescaler *= 10)
    {
        if (timeout % 10 != 0)
        {
            break;
        }
        timeout /= 10;
    }

    /* Calculate the Least error reload */
    if (timeout <= timer->data->period_max)
    {
        temp_period = timeout;
    } else
    {
        temp_period = (timeout / timer->data->period_max);
    }

    for (; temp_period > 0; temp_period--)
    {
        temp_reload = timeout / temp_period;

        error = timeout - temp_reload * temp_period;
        if (error == 0)
        {
            timer->reload = temp_reload;
            *period = temp_period;
            break;
        }
        if (error <= error_min)
        {
            error_min = error;
            timer->reload = temp_reload;
            *period = temp_period;
        }
    }

    /* Calculate the optimal reload */
    for (i = 2; i <= 9; i++)
    {
        while ((timer->reload % i) == 0)
        {
            mr_uint32_t temp = *period * i;
            if (temp <= timer->data->period_max)
            {
                *period = temp;
                timer->reload /= i;
            } else
            {
                break;
            }
            if (timer->reload > *period && timer->reload < timer->data->period_max)
            {
                timer->reload = timer->reload ^ *period;
                *period = timer->reload ^ *period;
                timer->reload = timer->reload ^ *period;
            }
        }
    }

    timer->count = *period;
    timer->timeout = (*prescaler) * (*period) / clk_mhz;

    return MR_ERR_OK;
}

static mr_err_t mr_timer_open(mr_device_t device)
{
    mr_timer_t timer = (mr_timer_t)device;

    return timer->ops->configure(timer, MR_ENABLE);
}

static mr_err_t mr_timer_close(mr_device_t device)
{
    mr_timer_t timer = (mr_timer_t)device;

    return timer->ops->configure(timer, MR_DISABLE);
}

static mr_err_t mr_timer_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_timer_t timer = (mr_timer_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_timer_config_t config = (mr_timer_config_t)args;
                timer->config = *config;
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_timer_config_t config = (mr_timer_config_t)args;
                *config = timer->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_SET_RX_CB:
        {
            device->rx_cb = (mr_device_cb_t)args;
            return MR_ERR_OK;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_timer_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_timer_t timer = (mr_timer_t)device;
    mr_uint32_t *read_buffer = (mr_uint32_t *)buffer;
    mr_size_t read_size = 0;

    while ((read_size += sizeof(*read_buffer)) <= size)
    {
        /* Get current count */
        mr_uint32_t count = timer->ops->get_count(timer);
        if (timer->data->count_mode == MR_TIMER_COUNT_MODE_DOWN)
        {
            count = timer->count - count;
        }

        *read_buffer = timer->overflow * timer->timeout + count * timer->timeout / timer->count;

        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_timer_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_timer_t timer = (mr_timer_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_uint32_t timeout = 0, prescaler = 0, period = 0;
    mr_size_t write_size = 0;
    mr_err_t ret = MR_ERR_OK;

    while ((write_size += sizeof(*write_buffer)) <= size)
    {
        timeout = *write_buffer;
        write_buffer++;
    }

    timer->ops->stop(timer);
    timer->overflow = 0;
    if (timeout != 0)
    {
        ret = mr_timer_calculate(timer, timeout, &prescaler, &period);
        if (ret != MR_ERR_OK)
        {
            return ret;
        }
        timer->ops->start(timer, prescaler, period);
    }

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the timer device.
 *
 * @param timer The timer device to be added.
 * @param name The name of the timer device.
 * @param ops The operations of the timer device.
 * @param timer_data The data of the timer device.
 * @param data The private data of the timer device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_timer_device_add(mr_timer_t timer,
                             const char *name,
                             struct mr_timer_ops *ops,
                             struct mr_timer_data *timer_data,
                             void *data)
{
    static struct mr_device_ops device_ops =
        {
            mr_timer_open,
            mr_timer_close,
            mr_timer_ioctl,
            mr_timer_read,
            mr_timer_write,
        };
    struct mr_timer_config default_config = MR_TIMER_CONFIG_DEFAULT;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);
    MR_ASSERT(timer_data != MR_NULL);
    MR_ASSERT(timer_data->clk > 0);
    MR_ASSERT(timer_data->prescaler_max > 0);
    MR_ASSERT(timer_data->period_max > 0);

    /* Initialize the private fields */
    timer->config = default_config;
    timer->reload = 0;
    timer->overflow = 0;
    timer->count = 1;
    timer->timeout = 0;
    timer->data = timer_data;

    /* Protect every operation of the timer device */
    ops->configure = ops->configure ? ops->configure : err_io_timer_configure;
    ops->start = ops->start ? ops->start : err_io_timer_start;
    ops->stop = ops->stop ? ops->stop : err_io_timer_stop;
    ops->get_count = ops->get_count ? ops->get_count : err_io_timer_get_count;
    timer->ops = ops;

    /* Add the container */
    return mr_device_add(&timer->device, name, Mr_Device_Type_Timer, MR_DEVICE_OFLAG_RDWR, &device_ops, data);
}

/**
 * @brief This function service interrupt routine of the timer device.
 *
 * @param timer The timer device.
 * @param event The interrupt event.
 */
void mr_timer_device_isr(mr_timer_t timer, mr_uint32_t event)
{
    MR_ASSERT(timer != MR_NULL);

    switch (event & MR_TIMER_EVENT_MASK)
    {
        case MR_TIMER_EVENT_PIT_INT:
        {
            timer->overflow++;

            /* Timeout */
            if (timer->overflow == timer->reload)
            {
                timer->overflow = 0;

                /* Stop the timer if it is a one-shot timer */
                if (timer->config.mode == MR_TIMER_MODE_ONE_SHOT)
                {
                    timer->ops->stop(timer);
                }

                /* Call the receiving completion function */
                if (timer->device.rx_cb != MR_NULL)
                {
                    timer->device.rx_cb(&timer->device, MR_NULL);
                }
            }
            break;
        }

        default:
            break;
    }
}

#endif