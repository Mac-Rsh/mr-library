/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/timer/timer.h"

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

static mr_err_t err_io_timer_configure(mr_timer_t timer, struct mr_timer_config *config)
{
    return -MR_ERR_IO;
}

static void err_io_timer_start(mr_timer_t timer, mr_uint32_t period_reload)
{

}

static void err_io_timer_stop(mr_timer_t timer)
{

}

static mr_uint32_t err_io_timer_get_count(mr_timer_t timer)
{
    return 0;
}

static mr_uint32_t mr_timer_timeout_calculate(mr_timer_t timer, mr_uint32_t timeout)
{
    mr_uint32_t count = 0, timer_period = 0;
    mr_uint32_t error = 0, error_min = 0, reload_best = 0;
    mr_uint32_t i = 0;

    /* Calculate the timer-period and timeout total count */
    timer_period = 1000000u / timer->config.freq;
    count = timeout / timer_period;

    if (count == 0)
    {
        count = 1;
    }

    if (count < timer->data->count_max)
    {
        timer->reload = 1;
        timer->cycles = timer->reload;
        timer->timeout = count * timer_period;
        return count;
    }

    if (count % timer->data->count_max == 0)
    {
        timer->reload = count / timer->data->count_max;
        timer->cycles = timer->reload;
        timer->timeout = timer->data->count_max * timer_period;
        return timer->data->count_max;
    }

    /* Calculate the Least error reload */
    error_min = count / timer->data->count_max + 1;
    for (i = error_min; i < count / 5; i++)
    {
        mr_uint32_t reload = count / i;
        error = count - reload * i;
        if (error <= 1)
        {
            reload_best = i;
            break;
        } else if (error < error_min)
        {
            error_min = error;
            reload_best = i;
        }
    }

    timer->reload = reload_best;
    timer->cycles = timer->reload;
    timer->timeout = count / reload_best * timer_period;

    return count / reload_best;
}

static mr_err_t mr_timer_open(mr_device_t device)
{
    mr_timer_t timer = (mr_timer_t)device;
    struct mr_timer_config default_config = MR_TIMER_CONFIG_DEFAULT;

    /* Enable timer using the default config */
    if (timer->config.freq == 0)
    {
        timer->config = default_config;
    }

    /* Limit the frequency */
    mr_limit_of(timer->config.freq, timer->data->freq_min, timer->data->freq_max);

    return timer->ops->configure(timer, &timer->config);
}

static mr_err_t mr_timer_close(mr_device_t device)
{
    mr_timer_t timer = (mr_timer_t)device;

    /* Disable timer */
    timer->config.freq = 0;

    return timer->ops->configure(timer, &timer->config);
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
                mr_limit_of(config->freq, timer->data->freq_min, timer->data->freq_max);
                ret = timer->ops->configure(timer, config);
                if (ret == MR_ERR_OK)
                {
                    timer->config = *config;
                }
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

//        case MR_DEVICE_CTRL_REBOOT:
//        {
//            timer->overflow = 0;
//            timer->cycles = timer->reload;
//
//            /* When the time is not less than one time, the timer is started */
//            if (timer->cycles == 0)
//            {
//                return -MR_ERR_INVALID;
//            }
//            timer->ops->start(timer, timer->timeout / (1000000u / timer->config.freq));
//            return MR_ERR_OK;
//        }

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
            count = timer->timeout / (1000000u / timer->config.freq) - count;
        }

        *read_buffer = timer->overflow * timer->timeout + count * (1000000u / timer->config.freq);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_timer_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_timer_t timer = (mr_timer_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_size_t write_size = 0;

    while ((write_size += sizeof(*write_buffer)) <= size)
    {
        timer->ops->stop(timer);
        timer->overflow = 0;

        mr_uint32_t period_reload = mr_timer_timeout_calculate(timer, *write_buffer);
        write_buffer++;

        /* When the time is not less than one time, the timer is started */
        if (timer->cycles == 0)
        {
            return -MR_ERR_INVALID;
        }
        timer->ops->start(timer, period_reload);
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

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);
    MR_ASSERT(timer_data != MR_NULL);
    MR_ASSERT(timer_data->freq_min > 0);
    MR_ASSERT(timer_data->freq_max >= timer_data->freq_min);
    MR_ASSERT(timer_data->count_max > 0);

    /* Initialize the private fields */
    timer->config.freq = 0;
    timer->reload = 0;
    timer->cycles = 0;
    timer->overflow = 0;
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
            timer->cycles--;

            /* Timeout */
            if (timer->cycles == 0)
            {
                timer->cycles = timer->reload;
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