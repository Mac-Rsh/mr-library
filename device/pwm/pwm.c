/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-12     MacRsh       first version
 */

#include "device/pwm/pwm.h"

#if (MR_CONF_PWM == MR_CONF_ENABLE)

static mr_err_t mr_pwm_open(mr_device_t device)
{
    mr_pwm_t pwm = (mr_pwm_t)device;

    /* Setting pwm to default config, if the frequency not set */
    if (pwm->config.freq == 0)
    {
        struct mr_pwm_config config = MR_PWM_CONFIG_DEFAULT;
        pwm->config = config;
    }

    /* Check the frequency */
    mr_limit(pwm->config.freq, pwm->info.min_freq, pwm->info.max_freq);

    return pwm->ops->configure(pwm, &pwm->config);
}

static mr_err_t mr_pwm_close(mr_device_t device)
{
    mr_pwm_t pwm = (mr_pwm_t)device;

    /* Setting pwm to close config */
    pwm->config.freq = 0;

    return pwm->ops->configure(pwm, &pwm->config);
}

static mr_err_t mr_pwm_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                struct mr_pwm_config *config = (struct mr_pwm_config *)args;

                /* Check the frequency */
                mr_limit(config->freq, pwm->info.min_freq, pwm->info.max_freq);

                ret = pwm->ops->configure(pwm, config);
                if (ret == MR_ERR_OK)
                {
                    pwm->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_err_t mr_pwm_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_uint32_t *read_buffer = (mr_uint32_t *)buffer;
    mr_size_t read_size = 0;

    if (size < sizeof(*read_buffer))
    {
        return -MR_ERR_INVALID;
    }

    for (read_size = 0; read_size < size; read_size += sizeof(*read_buffer))
    {
        *read_buffer = pwm->ops->read(pwm, pos);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_err_t mr_pwm_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_size_t write_size = 0;

    if (size < sizeof(*write_buffer))
    {
        return -MR_ERR_INVALID;
    }

    for (write_size = 0; write_size < size; write_size += sizeof(*write_buffer))
    {
        pwm->ops->write(pwm, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

static mr_err_t _err_io_pwm_configure(mr_pwm_t pwm, struct mr_pwm_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static mr_err_t _err_io_pwm_write(mr_pwm_t pwm, mr_pos_t channel, mr_uint32_t duty)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static mr_uint32_t _err_io_pwm_read(mr_pwm_t pwm, mr_pos_t channel)
{
    MR_ASSERT(0);
    return 0;
}

mr_err_t mr_pwm_device_add(mr_pwm_t pwm, const char *name, void *data, struct mr_pwm_ops *ops, struct mr_pwm_info *info)
{
    const static struct mr_device_ops device_ops =
            {
                    mr_pwm_open,
                    mr_pwm_close,
                    mr_pwm_ioctl,
                    mr_pwm_read,
                    mr_pwm_write,
            };

    MR_ASSERT(pwm != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);
    MR_ASSERT(info != MR_NULL);
    MR_ASSERT(info->min_freq > 0);
    MR_ASSERT(info->max_freq >= info->min_freq);

    /* Initialize the private fields */
    pwm->device.type = MR_DEVICE_TYPE_PWM;
    pwm->device.data = data;
    pwm->device.ops = &device_ops;

    pwm->config.freq = 0;
    pwm->info = *info;

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_pwm_configure;
    ops->write = ops->write ? ops->write : _err_io_pwm_write;
    ops->read = ops->read ? ops->read : _err_io_pwm_read;
    pwm->ops = ops;

    /* Add to the container */
    return mr_device_add(&pwm->device, name, MR_OPEN_RDWR);
}

#endif  /* MR_CONF_PWM */