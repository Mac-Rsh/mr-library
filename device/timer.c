/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#include "include/device/mr_timer.h"

#ifdef MR_USING_TIMER

static int timer_calculate(struct mr_timer *timer, uint32_t timeout)
{
    uint32_t clk = timer->info->clk, psc_max = timer->info->prescaler_max, per_max = timer->info->period_max;
    uint32_t psc_best = 1, per_best = 1, reload_best = 1;
    int error_min = INT32_MAX;

    /* Check the clock and timeout */
    if ((clk == 0) || (timeout == 0))
    {
        return MR_EINVAL;
    }

    /* Take the timeout as the product of the prescaler and period */
    uint32_t product = timeout;

    /* If the product is within the maximum period, set it as the period */
    if (product <= per_max)
    {
        psc_best = clk / 1000000;
        per_best = MR_BOUND(product, 1, per_max);
    } else
    {
        /* Calculate the least error prescaler and period */
        for (uint32_t psc = MR_BOUND((product / per_max), 1, product); psc < UINT32_MAX; psc++)
        {
            uint32_t per = MR_BOUND(product / psc, 1, per_max);

            /* Calculate the timeout error */
            int error = (int)(timeout - (per * psc));

            /* Found a valid and optimal solution */
            if (error <= 1)
            {
                psc_best = psc;
                per_best = per;
                break;
            } else if (error < error_min)
            {
                error_min = error;
                psc_best = psc;
                per_best = per;
            }
        }

        /* Calculate the reload and prescaler product */
        product = psc_best * (clk / 1000000);

        /* Calculate the least error reload and prescaler */
        for (uint32_t reload = MR_BOUND(product / psc_max, 1, product); reload < product; reload++)
        {
            uint32_t psc = MR_BOUND(product / reload, 1, psc_max);

            /* Calculate the product error */
            int error = (int)product - (int)(reload * psc);

            /* Found a valid and optimal solution */
            if (error <= 1)
            {
                reload_best = reload;
                psc_best = psc;
                break;
            } else if (error < error_min)
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
            MR_SWAP(reload_best, per_best);
        }
    }

    timer->prescaler = psc_best;
    timer->period = per_best;
    timer->reload = reload_best;
    timer->count = timer->reload;
    timer->timeout = timeout / timer->reload;
    return MR_EOK;
}

static int mr_timer_open(struct mr_dev *dev)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;

    return ops->configure(timer, MR_ENABLE);
}

static int mr_timer_close(struct mr_dev *dev)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;

    return ops->configure(timer, MR_DISABLE);
}

static ssize_t mr_timer_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;
    uint32_t *rd_buf = (uint32_t *)buf;
    ssize_t rd_size;

    MR_BIT_CLR(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        uint32_t count = ops->get_count(timer);

        /* Calculate the passed time */
        *rd_buf = (timer->reload - timer->count) * timer->timeout +
                  (uint32_t)(((float)count / (float)timer->period) * (float)timer->timeout);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_timer_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    uint32_t timeout = 0;
    ssize_t wr_size;

    /* Only the last write is valid */
    MR_BIT_CLR(size, sizeof(*wr_buf) - 1);
    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        timeout = *wr_buf;
        wr_buf++;
    }

    /* Reset timer */
    ops->stop(timer);
    timer->count = timer->reload;

    if (timeout != 0)
    {
        /* Calculate prescaler and period */
        int ret = timer_calculate(timer, timeout);
        if (ret < 0)
        {
            return ret;
        }

        /* Start timer */
        ops->start(timer, timer->prescaler, timer->period);
    }
    return wr_size;
}

static int mr_timer_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_timer *timer = (struct mr_timer *)dev;

    switch (cmd)
    {
        case MR_CTL_TIMER_SET_MODE:
        {
            if (args != MR_NULL)
            {
                struct mr_timer_config config = *(struct mr_timer_config *)args;

                timer->config = config;
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_CTL_TIMER_GET_MODE:
        {
            if (args != MR_NULL)
            {
                struct mr_timer_config *config = (struct mr_timer_config *)args;

                *config = timer->config;
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        default:
        {
            return MR_ENOTSUP;
        }
    }
}

static ssize_t mr_timer_isr(struct mr_dev *dev, int event, void *args)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;

    switch (event)
    {
        case MR_ISR_TIMER_TIMEOUT_INT:
        {
            timer->count--;

            if (timer->count == 0)
            {
                timer->count = timer->reload;

                if (timer->config.mode == MR_TIMER_MODE_ONESHOT)
                {
                    ops->stop(timer);
                }
                return MR_EOK;
            }
            return MR_EBUSY;
        }
        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function register a timer.
 *
 * @param timer The timer.
 * @param name The name of the timer.
 * @param drv The driver of the timer.
 * @param info The information of the timer.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_timer_register(struct mr_timer *timer, const char *name, struct mr_drv *drv, struct mr_timer_info *info)
{
    static struct mr_dev_ops ops =
        {
            mr_timer_open,
            mr_timer_close,
            mr_timer_read,
            mr_timer_write,
            mr_timer_ioctl,
            mr_timer_isr
        };
    struct mr_timer_config default_config = MR_TIMER_CONFIG_DEFAULT;

    MR_ASSERT(timer != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);
    MR_ASSERT(info != MR_NULL);

    /* Initialize the fields */
    timer->config = default_config;
    timer->reload = 0;
    timer->count = 0;
    timer->timeout = 0;
    timer->period = 0;
    timer->prescaler = 0;
    timer->info = info;

    /* Register the timer */
    return mr_dev_register(&timer->dev, name, Mr_Dev_Type_Timer, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_TIMER */
