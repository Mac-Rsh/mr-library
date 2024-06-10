/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#include <include/device/mr_timer.h>

#ifdef MR_USE_TIMER

static int _timer_timeout_calculate(struct mr_timer *timer, uint32_t timeout)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET((struct mr_device *)timer);
    struct mr_timer_driver_data *data = _MR_DRIVER_DATA_GET(driver);
    uint32_t clk = data->clk, psc_max = data->prescaler_max, per_max = data->period_max;
    uint32_t psc_best = 1, per_best = 1, reload_best = 1;
    uint32_t product = timeout;

    /* Check driver clock is valid */
    if (clk == 0)
    {
        return MR_EIO;
    }

    /* Check timeout is valid */
    if (timeout == 0)
    {
        return MR_EINVAL;
    }

    /* If the product is within the maximum period, set it as the period */
    if (product <= per_max)
    {
        psc_best = clk / 1000000;
        per_best = MR_BOUND(product, 1, per_max);
    } else
    {
        int error_min = INT32_MAX;

        /* Calculate the least error prescaler and period */
        for (uint32_t psc = MR_BOUND((product / per_max), 1, product); psc < UINT32_MAX; psc++)
        {
            uint32_t per = MR_BOUND(product / psc, 1, per_max);
            int error = (int)(timeout - (per * psc));

            /* Allowable error is less than or equal to 1us */
            if (error <= 1)
            {
                psc_best = psc;
                per_best = per;
                break;
            }

            /* Found a valid and optimal solution */
            if (error < error_min)
            {
                error_min = error;
                psc_best = psc;
                per_best = per;
            }
        }

        /* Calculate the reload and prescaler product */
        product = psc_best * (clk / 1000000);
        error_min = INT32_MAX;

        /* Calculate the least error reload and prescaler */
        for (uint32_t reload = MR_BOUND(product / psc_max, 1, product); reload < product; reload++)
        {
            uint32_t psc = MR_BOUND(product / reload, 1, psc_max);
            int error = (int)product - (int)(reload * psc);

            /* Allowable error is less than or equal to 1us */
            if (error <= 1)
            {
                reload_best = reload;
                psc_best = psc;
                break;
            }

            /* Found a valid and optimal solution */
            if (error < error_min)
            {
                error_min = error;
                reload_best = reload;
                psc_best = psc;
            }
        }

        /* If period can take reload value, lower interrupts by loading reload to period */
        if (per_best <= (per_max / reload_best))
        {
            per_best *= reload_best;
            reload_best = 1;

            /* If the reload is less than the prescaler, swap them */
        } else if ((reload_best > per_best) && (reload_best < per_max))
        {
            MR_SWAP(reload_best, per_best, uint32_t);
        }
    }

    /* Set timer parameters */
    timer->prescaler = psc_best;
    timer->period = per_best;
    timer->reload = reload_best;
    timer->count = timer->reload;
    timer->timeout = timeout / timer->reload;
    return MR_EOK;
}

static int timer_open(struct mr_device *device)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_timer_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Enable timer */
    return ops->configure(driver, true);
}

static int timer_close(struct mr_device *device)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_timer_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable timer */
    return ops->configure(driver, false);
}

static ssize_t timer_read(struct mr_device *device, int pos, void *buf, size_t count)
{
    struct mr_timer *timer = (struct mr_timer *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_timer_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t *rbuf = (uint32_t *)buf;
    ssize_t rcount;

    /* Read data */
    for (rcount = 0; rcount < MR_ALIGN_DOWN(count, sizeof(*rbuf)); rcount += sizeof(*rbuf))
    {
        uint32_t timer_count;

        /* Get the timer count */
        int ret = ops->get(driver, &timer_count);
        if (ret < 0)
        {
            /* If no data is read, return the error code */
            return (rcount == 0) ? ret : rcount;
        }

        /* Calculate the time */
        *rbuf = (timer->reload - timer->count) * timer->timeout + (
                    ((float)timer_count / (float)timer->period) * (float)timer->timeout);
        rbuf++;
    }

    /* Return the number of bytes read */
    return rcount;
}

static ssize_t timer_write(struct mr_device *device, int pos, const void *buf, size_t count)
{
    struct mr_timer *timer = (struct mr_timer *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_timer_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t *wbuf = (uint32_t *)buf;
    uint32_t timeout = 0;
    ssize_t wcount;

    /* Only the last value can be written */
    for (wcount = 0; wcount < MR_ALIGN_DOWN(count, sizeof(*wbuf)); wcount += sizeof(*wbuf))
    {
        timeout = *wbuf;
        wbuf++;
    }

    /* No data is written */
    if (wcount == 0)
    {
        return wcount;
    }

    /* Stop the timer */
    int ret = ops->stop(driver);
    if (ret < 0)
    {
        return ret;
    }

    /* Calculate the timeout prescaler and period */
    ret = _timer_timeout_calculate(timer, timeout);
    if (ret < 0)
    {
        return ret;
    }

    /* Start the timer */
    ret = ops->start(driver, timer->prescaler, timer->period);
    if (ret < 0)
    {
        return ret;
    }

    /* Return the number of bytes written */
    return wcount;
}

static int timer_ioctl(struct mr_device *device, int pos, int cmd, void *args)
{
    struct mr_timer *timer = (struct mr_timer *)device;

    switch (cmd)
    {
        case MR_CMD_TIMER_CONFIG:
        {
            struct mr_timer_config *config = (struct mr_timer_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Set the timer configure */
            timer->config = *config;
            return sizeof(*config);
        }
        case (-MR_CMD_TIMER_CONFIG):
        {
            struct mr_timer_config *config = (struct mr_timer_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get the timer configure */
            *config = timer->config;
            return sizeof(*config);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

static int timer_isr(struct mr_device *device, uint32_t event, void *args)
{
    struct mr_timer *timer = (struct mr_timer *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_timer_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    switch (event)
    {
        case MR_EVENT_TIMER_TIMEOUT_INT:
        {
            /* Decrement the timer count */
            timer->count--;

            /* If the timer count does not reach 0, continue running */
            if (timer->count > 0)
            {
                return MR_EBUSY;
            }

            /* Reset the timer count */
            timer->count = timer->reload;

            /* If the timer is in oneshot mode, stop the timer */
            if (timer->config.mode == MR_TIMER_MODE_ONESHOT)
            {
                int ret = ops->stop(driver);
                if (ret < 0)
                {
                    return ret;
                }
            }
            return MR_EOK;
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function registers a timer.
 *
 * @param timer The timer.
 * @param path The path of the timer.
 * @param driver The driver of the timer.
 *
 * @return The error code.
 */
int mr_timer_register(struct mr_timer *timer, const char *path, const struct mr_driver *driver)
{
    MR_ASSERT(timer != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL) && (driver->data != NULL));

    static struct mr_device_ops ops = {.open = timer_open,
                                       .close = timer_close,
                                       .read = timer_read,
                                       .write = timer_write,
                                       .ioctl = timer_ioctl,
                                       .isr = timer_isr};
    struct mr_timer_config default_config = MR_TIMER_CONFIG_DEFAULT;

    /* Initialize the timer */
    timer->config = default_config;
    timer->reload = 0;
    timer->count = 0;
    timer->timeout = 0;
    timer->prescaler = 0;
    timer->period = 0;

    /* Register the timer */
    return mr_device_register((struct mr_device *)timer, path, MR_DEVICE_TYPE_TIMER, &ops, driver);
}

#endif /* MR_USE_TIMER */
