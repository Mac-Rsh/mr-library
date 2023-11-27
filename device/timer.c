/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-15    MacRsh       First version
 */

#include "timer.h"

#ifdef MR_USING_TIMER

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

    if (timer->config.mode == MR_TIMER_MODE_TIMING)
    {
        uint32_t time = 0;


    }
}

static ssize_t mr_timer_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    ssize_t wr_size = 0;

    if (timer->config.mode == MR_TIMER_MODE_TIMING)
    {
        uint32_t timeout = 0;

        /* Only the last valid data will be written */
        mr_bits_clr(size, sizeof(*wr_buf) - 1);
        for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
        {
            timeout = *wr_buf;
            wr_buf++;
        }

        /* Stop timer */
        ops->stop(timer);

        /* Calculate reload value */
        timer->reload = (uint32_t)(((float)timeout / 1000000.0f) * (float)timer->config.freq + 0.5f);
        if (timer->reload == 0)
        {
            return MR_EINVAL;
        }

        /* Start timer */
        ops->start(timer, timer->prescaler, timer->period);
    } else
    {
        mr_bits_clr(size, sizeof(*wr_buf) - 1);
        for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
        {
            wr_buf++;
        }
    }

    return wr_size;
}

static int mr_timer_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_timer *timer = (struct mr_timer *)dev;
    struct mr_timer_ops *ops = (struct mr_timer_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_timer_config *config = (struct mr_timer_config *)args;
                uint32_t count = 0, prescaler = 0;
                int error_min = INT32_MAX;

                /* Too large frequency */
                if (config->freq > timer->info.clk || config->freq == 0)
                {
                    return MR_EINVAL;
                }

                /* Stop timer */
                ops->stop(timer);

                /* Calculate period and prescaler */
                if (timer->config.freq != config->freq)
                {
                    count = timer->info.clk / config->freq;
                    for (prescaler = (count / timer->info.period_max) + 1; prescaler < timer->info.prescaler_max;
                         prescaler++)
                    {
                        uint32_t period = 0;
                        int error = 0;

                        period = count / prescaler;
                        error = (int)(count - (prescaler * period));

                        /* Allowable error <= 1 */
                        if (error <= 1)
                        {
                            timer->period = period;
                            break;
                        }

                        /* Update error */
                        if (error < error_min)
                        {
                            error_min = error;
                            timer->period = period;
                        }
                    }
                    timer->prescaler = count / timer->period;
                }
                timer->config = *config;

                /* Start timer if mode is PWM */
                if (timer->config.mode == MR_TIMER_MODE_TIMING)
                {
                    timer->reload = 0;
                } else
                {
                    ops->start(timer, timer->prescaler, timer->period);
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_TIMER_SET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                int mode = *((int *)args);

                /* Check offset is valid */
                if (off < 0 || off >= 32)
                {
                    return MR_EINVAL;
                }

                /* Check if the channel is enabled */
                if (mode != mr_bits_is_set(timer->channel, (1 << off)))
                {
                    int ret = ops->channel_configure(timer, off, mode);
                    if (ret == MR_EOK)
                    {
                        if (mode == MR_TIMER_STATE_ENABLE)
                        {
                            mr_bits_set(timer->channel, (1 << off));
                        } else
                        {
                            mr_bits_clr(timer->channel, (1 << off));
                        }
                    }
                    return ret;
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTL_TIMER_GET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                int *mode = (int *)args;

                /* Check offset is valid */
                if (off < 0 || off >= 32)
                {
                    return MR_EINVAL;
                }

                *mode = mr_bits_is_set(timer->channel, (1 << off));
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

int mr_timer_register(struct mr_timer *timer, const char *name, struct mr_drv *drv, struct mr_timer_info *info)
{
    static struct mr_dev_ops ops =
        {
            mr_timer_open,
            mr_timer_close,
            MR_NULL,
            mr_timer_write,
            mr_timer_ioctl
        };
    struct mr_timer_config default_config = MR_TIMER_CONFIG_DEFAULT;

    mr_assert(timer != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);
    mr_assert(info != MR_NULL);

    /* Initialize the fields */
    timer->info = *info;
    timer->config = default_config;

    return mr_dev_register(&timer->dev, name, Mr_Dev_Type_Timer, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_TIMER */
