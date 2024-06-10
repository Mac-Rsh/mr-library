/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-10    MacRsh       First version
 */

#include <include/device/mr_pwm.h>

#ifdef MR_USE_PWM

#define _PWM_CHANNEL_IS_VALID(_pwm, _channel)                                                      \
    (((_channel) >= 0) && ((_channel) < (sizeof((_pwm)->channels) * 4)))
#define _PWM_CHANNEL_IS_EXISTED(_data, _channel)                                                   \
    (((_data)->channels & (0x1 << (_channel))) != 0)
#define _PWM_CHANNEL_STATE_GET(_pwm, _channel)                                                     \
    (MR_BIT_IS_SET((_pwm)->channels, (0x1 << ((_channel) * 2))))
#define _PWM_CHANNEL_STATE_SET(_pwm, _channel, _state)                                             \
    do                                                                                             \
    {                                                                                              \
        MR_BIT_CLR(pwm->channels, (0x1 << ((_channel) * 2)));                                      \
        MR_BIT_SET(pwm->channels, ((_state) << ((_channel) * 2)));                                 \
    } while (0)
#define _PWM_CHANNEL_POLARITY_GET(_pwm, _channel)                                                  \
    (MR_BIT_IS_SET((_pwm)->channels, (0x2 << ((_channel) * 2))))
#define _PWM_CHANNEL_POLARITY_SET(_pwm, _channel, _polarity)                                       \
    do                                                                                             \
    {                                                                                              \
        MR_BIT_CLR(pwm->channels, (0x2 << ((_channel) * 2 + 1)));                                  \
        MR_BIT_SET(pwm->channels, ((_polarity) << ((_channel) * 2 + 1)));                          \
    } while (0)
#define _PWM_CHANNEL_IS_ENABLED(_pwm, _channel)                                                    \
    (_PWM_CHANNEL_STATE_GET((_pwm), (_channel)) != MR_PWM_CHANNEL_STATE_DISABLE)

static int _pwm_channel_configure_set(struct mr_pwm *pwm, int channel,
                                      const struct mr_pwm_config *config)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET((struct mr_device *)pwm);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    struct mr_pwm_driver_data *data = _MR_DRIVER_DATA_GET(driver);

    /* Check if the channel is valid */
    if (_PWM_CHANNEL_IS_VALID(pwm, channel) == false)
    {
        return MR_EINVAL;
    }

    /* Check if the channel is existed */
    if ((data != NULL) && (_PWM_CHANNEL_IS_EXISTED(data, channel) == false))
    {
        return MR_EINVAL;
    }

    /* Check if the channel is already configured */
    if (config->channel_state == _PWM_CHANNEL_IS_ENABLED(pwm, channel))
    {
        return MR_EOK;
    }

    /* Configure the channel */
    int ret = ops->channel_configure(driver, channel, config->channel_state, config->polarity);
    if (ret < 0)
    {
        return ret;
    }

    /* Set the channel state and polarity */
    _PWM_CHANNEL_STATE_SET(pwm, channel, config->channel_state);
    _PWM_CHANNEL_POLARITY_SET(pwm, channel, config->polarity);
    return MR_EOK;
}

static int _pwm_channel_configure_get(const struct mr_pwm *pwm, int channel,
                                      struct mr_pwm_config *config)
{
    /* Check if the channel is valid */
    if (_PWM_CHANNEL_IS_VALID(pwm, channel) == false)
    {
        return MR_EINVAL;
    }

    /* Get the channel state and polarity */
    config->channel_state = _PWM_CHANNEL_STATE_GET(pwm, channel);
    config->polarity = _PWM_CHANNEL_POLARITY_GET(pwm, channel);
    return MR_EOK;
}

static int _pwm_freq_calculate(struct mr_pwm *pwm, uint32_t freq)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET((struct mr_device *)pwm);
    struct mr_pwm_driver_data *data = _MR_DRIVER_DATA_GET(driver);
    uint32_t clk = data->clk, psc_max = data->prescaler_max, per_max = data->period_max;
    uint32_t psc_best = 1, per_best = 1;

    /* Check driver clock is valid */
    if (clk == 0)
    {
        return MR_EIO;
    }

    /* Check frequency is valid */
    if (freq == 0)
    {
        return MR_EINVAL;
    }

    /* Calculate the prescaler and period product */
    uint32_t product = clk / freq;

    /* If the product is within the maximum period, set it as the period */
    if (product <= per_max)
    {
        psc_best = 1;
        per_best = MR_BOUND(product, 1, per_max);
    } else
    {
        int error_min = INT32_MAX;

        /* Calculate the least error prescaler and period */
        for (uint32_t psc = MR_BOUND(product / per_max, 1, psc_max); psc < psc_max; psc++)
        {
            uint32_t per = MR_BOUND(product / psc, 1, per_max);
            int error = (clk / psc / per) - freq;

            /* Found a valid and optimal solution */
            if (error == 0)
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
    }

    /* Set pwm parameters */
    pwm->prescaler = psc_best;
    pwm->period = per_best;
    pwm->freq = clk / psc_best / per_best;
    return MR_EOK;
}

static int pwm_open(struct mr_device *device)
{
    struct mr_pwm *pwm = (struct mr_pwm *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Recover all enabled channels */
    for (uint32_t i = 0; i < (sizeof(pwm->channels) * 4); i++)
    {
        if (_PWM_CHANNEL_IS_ENABLED(pwm, i) == true)
        {
            ops->channel_configure(driver, i, true, _PWM_CHANNEL_POLARITY_GET(pwm, i));
        }
    }

    /* Enable pwm */
    return ops->configure(driver, true);
}

static int pwm_close(struct mr_device *device)
{
    struct mr_pwm *pwm = (struct mr_pwm *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable all enabled channels */
    for (uint32_t i = 0; i < (sizeof(pwm->channels) * 4); i++)
    {
        if (_PWM_CHANNEL_IS_ENABLED(pwm, i) == true)
        {
            /* Just close the channel, without clearing the channel mask, and the channel will be 
             * restored when opened */
            ops->channel_configure(driver, i, false, MR_PWM_POLARITY_NORMAL);
        }
    }

    /* Disable pwm */
    return ops->configure(driver, false);
}

static ssize_t pwm_read(struct mr_device *device, int pos, void *buf, size_t count)
{
    struct mr_pwm *pwm = (struct mr_pwm *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t *rbuf = (uint32_t *)buf;
    ssize_t rcount;

#ifdef MR_USE_PWM_CHECK
    /* Check if the channel is enabled */
    if ((_PWM_CHANNEL_IS_VALID(pwm, pos) == false) || (_PWM_CHANNEL_IS_ENABLED(pwm, pos) == false))
    {
        return MR_EINVAL;
    }
#endif /* MR_USE_PWM_CHECK */

    /* Read data */
    for (rcount = 0; rcount < MR_ALIGN_DOWN(count, sizeof(*rbuf)); rcount += sizeof(*rbuf))
    {
        uint32_t pwm_compare;

        /* Get the pwm compare */
        int ret = ops->get(driver, pos, &pwm_compare);
        if (ret < 0)
        {
            /* If no data is read, return the error code */
            return (rcount == 0) ? ret : rcount;
        }

        /* Calculate the fequency */
        *rbuf = ((float)pwm_compare / (float)pwm->period) * 1000000.0f;
        rbuf++;
    }

    /* Return the number of bytes read */
    return rcount;
}

static ssize_t pwm_write(struct mr_device *device, int pos, const void *buf, size_t count)
{
    struct mr_pwm *pwm = (struct mr_pwm *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t *wbuf = (uint32_t *)buf;
    ssize_t wcount;

#ifdef MR_USE_PWM_CHECK
    /* Check if the channel is enabled */
    if ((_PWM_CHANNEL_IS_VALID(pwm, pos) == false) || (_PWM_CHANNEL_IS_ENABLED(pwm, pos) == false))
    {
        return MR_EINVAL;
    }
#endif /* MR_USE_PWM_CHECK */

    /* Write data */
    for (wcount = 0; wcount < MR_ALIGN_DOWN(count, sizeof(*wbuf)); wcount += sizeof(*wbuf)) 
    {
        /* Calculate the compare value */
        uint32_t compare_value =
            MR_BOUND(((float)*wbuf / 1000000.0f) * (float)(pwm->period), 0, pwm->period);
        int ret = ops->set(driver, pos, compare_value);
        if (ret < 0) 
        {
            /* If no data is written, return the error code */
            return (wcount == 0) ? ret : wcount;
        }
        wbuf++;
    }

    /* Return the number of bytes written */
    return wcount;
}

static int pwm_ioctl(struct mr_device *device, int pos, int cmd, void *args)
{
    struct mr_pwm *pwm = (struct mr_pwm *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_pwm_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    switch(cmd)
    {
        case MR_CMD_PWM_CONFIG:
        {
            struct mr_pwm_config *config = (struct mr_pwm_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Set the pwm channel configure */
            int ret = _pwm_channel_configure_set(pwm, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        case (-MR_CMD_PWM_CONFIG):
        {
            struct mr_pwm_config *config = (struct mr_pwm_config *)args;
            
            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get the pwm channel configure */
            int ret = _pwm_channel_configure_get(pwm, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        case MR_CMD_PWM_FREQ:
        {
            uint32_t *freq = (uint32_t *)args;
            uint32_t old_period = pwm->period;

            if (freq == NULL)
            {
                return MR_EINVAL;
            }

            /* Calculate the frequency prescaler and period */
            int ret = _pwm_freq_calculate(pwm, *freq);
            if (ret < 0)
            {
                return ret;
            }

            /* Start the pwm */
            ret = ops->start(driver, pwm->prescaler, pwm->period);
            if (ret < 0)
            {
                return ret;
            }

            /* Refresh all channels compare value */
            for (uint32_t i = 0; i < (sizeof(pwm->channels) * 4); i++)
            {
                if (_PWM_CHANNEL_IS_ENABLED(pwm, i) == true)
                {
                    uint32_t pwm_compare;

                    /* Get the old compare value */
                    ret = ops->get(driver, i, &pwm_compare);
                    if (ret < 0)
                    {
                        continue;
                    }

                    /* Calculate the new compare value */
                    pwm_compare = ((float)pwm_compare / (float)old_period) * (float)(pwm->period);
                    ops->set(driver, i, pwm_compare);
                }
            }
            return sizeof(*freq);
        }
        case (-MR_CMD_PWM_FREQ):
        {
            uint32_t *freq = (uint32_t *)args;

            if (freq == NULL)
            {
                return MR_EINVAL;
            }

            /* Get the current pwm frequency */
            *freq = pwm->freq;
            return sizeof(*freq);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function registers a pwm.
 * 
 * @param pwm The pwm.
 * @param path The path of the pwm.
 * @param driver The driver of the pwm.
 * 
 * @return The error code. 
 */
int mr_pwm_register(struct mr_pwm *pwm, const char *path, const struct mr_driver *driver)
{
    MR_ASSERT(pwm != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL) && (driver->data != NULL));

    static struct mr_device_ops ops = {.open = pwm_open,
                                       .close = pwm_close,
                                       .read = pwm_read,
                                       .write = pwm_write,
                                       .ioctl = pwm_ioctl};

    /* Initialize the pwm */
    pwm->freq = 0;
    pwm->prescaler = 0;
    pwm->period = 0;
    pwm->channels = 0;

    /* Register the pwm */
    return mr_device_register((struct mr_device *)pwm, path, MR_DEVICE_TYPE_PWM, &ops, driver);
}

#endif /* MR_USE_PWM */
