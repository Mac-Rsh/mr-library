/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#include "timer.h"

#ifdef MR_USING_TIMER

static int timer_calculate(struct mr_timer *timer, uint32_t timeout)
{
    uint32_t clk = timer->info->clk, psc_max = timer->info->prescaler_max, per_max = timer->info->period_max;
    uint32_t psc_best = 0, per_best = 0, reload_best = 0;
    uint32_t psc = 0, per = 0, reload = 0;
    int error = 0, error_min = INT_MAX;

    /* Check the clock */
    if (clk == 0)
    {
        return MR_EINVAL;
    }

    /* Calculate the prescaler */
    for (psc = clk; psc <= (psc_max / 10); psc *= 10)
    {
        /* If the timeout is not a multiple of 10, break */
        if (timeout % 10 != 0)
        {
            break;
        }
        timeout /= 10;
    }

    /* Calculate the Least error reload */
    for (per = (timeout <= per_max) ? timeout : (timeout / (per_max + 1)); per > 0; per--)
    {
        reload = timeout / per;

        /* Calculate the error */
        error = (int)timeout - (int)(reload * per);
        if (error == 0)
        {
            reload_best = reload;
            psc_best = psc;
            per_best = per;
            break;
        }
        if (error <= error_min)
        {
            error_min = error;
            reload_best = reload;
            psc_best = psc;
            per_best = per;
        }
    }

    /* Optimize the prescaler and period */
    uint32_t divisor = 0;
    for (divisor = 2; divisor <= 9; divisor++)
    {
        /* Check if reload value can be divided by current divisor */
        while ((reload_best % divisor) == 0)
        {
            uint32_t per_temp = per_best * divisor;
            uint32_t psc_temp = psc_best * divisor;

            /* Check if new period or prescaler is valid */
            if (per_temp <= per_max)
            {
                per_best = per_temp;
                reload_best /= divisor;
            } else if (psc_temp <= psc_max)
            {
                psc_best = psc_temp;
                reload_best /= divisor;
            } else
            {
                break;
            }

            /* Check if reload can be used as period or prescaler */
            if ((reload_best > per_best) && (reload_best < per_max))
            {
                mr_swap(per_best, reload_best);
            } else if ((reload_best > psc_best) && (reload_best < psc_max))
            {
                mr_swap(psc_best, reload_best);
            }
        }
    }

    timer->prescaler = psc_best;
    timer->period = per_best;
    timer->reload = reload_best;
    timer->count = timer->reload;
    if ((psc_best < UINT16_MAX) && (per_best < UINT16_MAX))
    {
        timer->timeout = psc_best * per_best / clk;
    } else
    {
        timer->timeout = psc_best / clk * per_best;
    }
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
    ssize_t rd_size = 0;

    mr_bits_clr(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        uint32_t count = ops->get_count(timer);

        /* Calculate the passed time */
        *rd_buf = (timer->reload - timer->count) * timer->timeout + count * timer->timeout / timer->period;
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
    ssize_t wr_size = 0;

    /* Only the last write is valid */
    mr_bits_clr(size, sizeof(*wr_buf) - 1);
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
        if (ret != MR_EOK)
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
        case MR_CTL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_timer_config config = *(struct mr_timer_config *)args;

                timer->config = config;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_timer_config *config = (struct mr_timer_config *)args;

                *config = timer->config;
                return MR_EOK;
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

    mr_assert(timer != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);
    mr_assert(info != MR_NULL);

    /* Initialize the fields */
    timer->config = default_config;
    timer->reload = 0;
    timer->count = 0;
    timer->timeout = 0;
    timer->period = 0;
    timer->prescaler = 0;
    timer->info = info;

    return mr_dev_register(&timer->dev, name, Mr_Dev_Type_Timer, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_TIMER */
