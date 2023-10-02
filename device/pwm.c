/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-12     MacRsh       first version
 */

#include "pwm.h"

#if (MR_CFG_PWM == MR_CFG_ENABLE)

static mr_err_t err_io_pwm_configure(mr_pwm_t pwm, mr_pwm_config_t config)
{
    return -MR_ERR_IO;
}

static void err_io_pwm_write(mr_pwm_t pwm, mr_off_t channel, mr_uint32_t duty)
{

}

static mr_uint32_t err_io_pwm_read(mr_pwm_t pwm, mr_off_t channel)
{
    return 0;
}

static mr_err_t mr_pwm_open(mr_device_t device)
{
    mr_pwm_t pwm = (mr_pwm_t)device;

    return pwm->ops->configure(pwm, &pwm->config);
}

static mr_err_t mr_pwm_close(mr_device_t device)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    struct mr_pwm_config config = {0};

    return pwm->ops->configure(pwm, &config);
}

static mr_err_t mr_pwm_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_pwm_config_t config = (mr_pwm_config_t)args;
                ret = pwm->ops->configure(pwm, config);
                if (ret == MR_ERR_OK)
                {
                    pwm->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_pwm_config_t config = (mr_pwm_config_t)args;
                *config = pwm->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_err_t mr_pwm_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_uint32_t *read_buffer = (mr_uint32_t *)buffer;
    mr_size_t read_size = 0;

    if (pos < 0)
    {
        return -MR_ERR_INVALID;
    }

    while ((read_size += sizeof(*read_buffer)) <= size)
    {
        *read_buffer = pwm->ops->read(pwm, pos);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_err_t mr_pwm_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_pwm_t pwm = (mr_pwm_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_size_t write_size = 0;

    if (pos < 0)
    {
        return -MR_ERR_INVALID;
    }

    while ((write_size += sizeof(*write_buffer)) <= size)
    {
        pwm->ops->write(pwm, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the pwm device.
 *
 * @param pwm The pwm device to be added.
 * @param name The name of the pwm device.
 * @param ops The operations of the pwm device.
 * @param data The private data of the pwm device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_pwm_device_add(mr_pwm_t pwm, const char *name, struct mr_pwm_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
        {
            mr_pwm_open,
            mr_pwm_close,
            mr_pwm_ioctl,
            mr_pwm_read,
            mr_pwm_write,
        };
    struct mr_pwm_config default_config = MR_PWM_CONFIG_DEFAULT;

    MR_ASSERT(pwm != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    pwm->config = default_config;

    /* Protect every operation of the pwm device */
    ops->configure = ops->configure ? ops->configure : err_io_pwm_configure;
    ops->write = ops->write ? ops->write : err_io_pwm_write;
    ops->read = ops->read ? ops->read : err_io_pwm_read;
    pwm->ops = ops;

    /* Add the device */
    return mr_device_add(&pwm->device, name, Mr_Device_Type_PWM, MR_DEVICE_OFLAG_RDWR, &device_ops, data);
}

#endif