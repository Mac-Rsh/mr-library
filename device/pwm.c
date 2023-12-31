/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-10    MacRsh       First version
 */

#include "include/device/mr_pwm.h"

#ifdef MR_USING_PWM

static int pwm_channel_set_configure(struct mr_pwm *pwm, int channel, struct mr_pwm_config config)
{
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)pwm->dev.drv->ops;

    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Configure the channel */
    int ret = ops->channel_configure(pwm, channel, config.state, config.polarity);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Enable or disable the channel */
    if (config.state == MR_PWM_ENABLE)
    {
        MR_BIT_SET(pwm->channel, (1 << channel));

        /* Configure the polarity */
        if (config.polarity == MR_PWM_POLARITY_NORMAL)
        {
            MR_BIT_CLR(pwm->channel_polarity, (1 << channel));
        } else
        {
            MR_BIT_SET(pwm->channel_polarity, (1 << channel));
        }
    } else
    {
        MR_BIT_CLR(pwm->channel, (1 << channel));
        MR_BIT_CLR(pwm->channel_polarity, (1 << channel));
    }
    return MR_EOK;
}

static int pwm_channel_get_configure(struct mr_pwm *pwm, int channel, struct mr_pwm_config *config)
{
    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Get configure */
    config->state = MR_BIT_IS_SET(pwm->channel, (1 << channel));
    config->polarity = MR_BIT_IS_SET(pwm->channel_polarity, (1 << channel));

    return config->state;
}

static int pwm_calculate(struct mr_pwm *pwm, uint32_t freq)
{
    uint32_t clk = pwm->info->clk, psc_max = pwm->info->prescaler_max, per_max = pwm->info->period_max;
    uint32_t psc_best = 0, per_best = 0;
    int error_min = INT32_MAX;

    /* Check the clock */
    if (clk == 0 || freq == 0)
    {
        return MR_EINVAL;
    }

    /* Calculate the timeout */
    uint32_t timeout = (clk * 1000000) / freq;

    /* Calculate the Least error period */
    for (uint32_t per = (timeout <= per_max) ? timeout : (timeout / (per_max + 1)); per > 0; per--)
    {
        uint32_t psc = timeout / per;

        /* Calculate the error */
        int error = (int)timeout - (int)(psc * per);
        if (error == 0)
        {
            psc_best = psc;
            per_best = per;
            break;
        }
        if (error <= error_min)
        {
            error_min = error;
            psc_best = psc;
            per_best = per;
        }
    }

    /* Optimize the prescaler and period */
    for (uint32_t divisor = 9; divisor > 1; divisor--)
    {
        /* Check if reload value can be divided by current divisor */
        while ((psc_best % divisor) == 0)
        {
            uint32_t per_temp = per_best * divisor;

            /* Check if new period or prescaler is valid */
            if (per_temp <= per_max)
            {
                per_best = per_temp;
                psc_best /= divisor;
            } else
            {
                break;
            }

            /* Check if prescaler can be used as period */
            if ((psc_best > per_best) && (psc_best < per_max))
            {
                MR_SWAP(per_best, psc_best);
            }
        }
    }

    /* Check prescaler is valid */
    if (psc_best > psc_max)
    {
        return MR_EINVAL;
    }

    pwm->prescaler = psc_best;
    pwm->period = per_best;
    pwm->freq = clk / psc_best / per_best;
    return MR_EOK;
}

static int mr_pwm_open(struct mr_dev *dev)
{
    struct mr_pwm *pwm = (struct mr_pwm *)dev;
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)dev->drv->ops;

    return ops->configure(pwm, MR_ENABLE);
}

static int mr_pwm_close(struct mr_dev *dev)
{
    struct mr_pwm *pwm = (struct mr_pwm *)dev;
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)dev->drv->ops;

    /* Disable all channels */
    for (size_t i = 0; i < 32; i++)
    {
        if (MR_BIT_IS_SET(pwm->channel, (1 << i)) == MR_ENABLE)
        {
            ops->channel_configure(pwm, (int)i, MR_DISABLE, MR_PWM_POLARITY_NORMAL);
            MR_BIT_CLR(pwm->channel, (1 << i));
        }
    }

    return ops->configure(pwm, MR_DISABLE);
}

static ssize_t mr_pwm_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_pwm *pwm = (struct mr_pwm *)dev;
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)dev->drv->ops;
    uint32_t *rd_buf = (uint32_t *)buf;
    ssize_t rd_size;

    /* Check if the channel is enabled */
    if (MR_BIT_IS_SET(pwm->channel, (1 << off)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }

    MR_BIT_CLR(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = ops->read(pwm, off);
        rd_buf++;
    }
    return rd_size;
}

static ssize_t mr_pwm_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_pwm *pwm = (struct mr_pwm *)dev;
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    ssize_t wr_size;

    /* Check if the channel is enabled */
    if (MR_BIT_IS_SET(pwm->channel, (1 << off)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }

    MR_BIT_CLR(size, sizeof(*wr_buf) - 1);
    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        ops->write(pwm, off, *wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_pwm_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_pwm *pwm = (struct mr_pwm *)dev;
    struct mr_pwm_ops *ops = (struct mr_pwm_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTL_PWM_SET_CHANNEL_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_pwm_config config = *((struct mr_pwm_config *)args);

                return pwm_channel_set_configure(pwm, off, config);
            }
            return MR_EINVAL;
        }
        case MR_CTL_PWM_SET_FREQ:
        {
            if (args != MR_NULL)
            {
                uint32_t freq = *((uint32_t *)args);

                /* Calculate prescaler and period */
                int ret = pwm_calculate(pwm, freq);
                if (ret != MR_EOK)
                {
                    return ret;
                }

                /* Start pwm */
                ops->start(pwm, pwm->prescaler, pwm->period);
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTL_PWM_GET_CHANNEL_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_pwm_config *config = ((struct mr_pwm_config *)args);

                int ret = pwm_channel_get_configure(pwm, off, config);
                if (ret < 0)
                {
                    return ret;
                }
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_PWM_GET_FREQ:
        {
            if (args != MR_NULL)
            {
                uint32_t *freq = (uint32_t *)args;

                *freq = pwm->freq;
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

/**
 * @brief This function registers a pwm.
 *
 * @param pwm The pwm.
 * @param name The name of the pwm.
 * @param drv The driver of the pwm.
 * @param info The information of the pwm.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_pwm_register(struct mr_pwm *pwm, const char *name, struct mr_drv *drv, struct mr_pwm_info *info)
{
    static struct mr_dev_ops ops =
        {
            mr_pwm_open,
            mr_pwm_close,
            mr_pwm_read,
            mr_pwm_write,
            mr_pwm_ioctl,
            MR_NULL
        };

    MR_ASSERT(pwm != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);
    MR_ASSERT(info != MR_NULL);

    /* Initialize the fields */
    pwm->freq = 0;
    pwm->prescaler = 0;
    pwm->period = 0;
    pwm->channel = 0;
    pwm->channel_polarity = 0;
    pwm->info = info;

    /* Register the pwm */
    return mr_dev_register(&pwm->dev, name, Mr_Dev_Type_PWM, MR_SFLAG_RDWR, &ops, drv);
}

#endif /* MR_USING_PWM */
