/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/dac/dac.h"

#if (MR_CONF_DAC == MR_CONF_ENABLE)

static mr_err_t mr_dac_open(mr_device_t device)
{
    mr_dac_t dac = (mr_dac_t)device;

    return dac->ops->configure(dac, MR_ENABLE);
}

static mr_err_t mr_dac_close(mr_device_t device)
{
    mr_dac_t dac = (mr_dac_t)device;

    /* Disable all channel */
    dac->config._channel_mask = 0;

    return dac->ops->configure(dac, MR_DISABLE);
}

static mr_err_t mr_dac_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_dac_t dac = (mr_dac_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                struct mr_dac_config *config = (struct mr_dac_config *)args;
                ret = dac->ops->channel_configure(dac, config);
                if (ret == MR_ERR_OK)
                {
                    dac->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args)
            {
                struct mr_dac_config *config = (struct mr_dac_config *)args;
                *config = dac->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_dac_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_dac_t dac = (mr_dac_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_size_t write_size = 0;

    if (size < sizeof(*write_buffer))
    {
        return -MR_ERR_INVALID;
    }

    /* Check whether the channel is enabled */
    if (!((1 << pos) & dac->config._channel_mask))
    {
        return -MR_ERR_INVALID;
    }

    for (write_size = 0; write_size < size; write_size += sizeof(*write_buffer))
    {
        dac->ops->write(dac, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

static mr_err_t _err_io_dac_configure(mr_dac_t dac, mr_state_t state)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static mr_err_t _err_io_dac_channel_configure(mr_dac_t dac, struct mr_dac_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static void _err_io_dac_write(mr_dac_t dac, mr_pos_t channel, mr_uint32_t value)
{
    MR_ASSERT(0);
}

mr_err_t mr_dac_device_add(mr_dac_t dac, const char *name, void *data, struct mr_dac_ops *ops)
{
    const static struct mr_device_ops device_ops =
            {
                    mr_dac_open,
                    mr_dac_close,
                    mr_dac_ioctl,
                    MR_NULL,
                    mr_dac_write,
            };

    MR_ASSERT(dac != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    dac->device.type = MR_DEVICE_TYPE_DAC;
    dac->device.data = data;
    dac->device.ops = &device_ops;

    dac->config._channel_mask = 0;

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_dac_configure;
    ops->channel_configure = ops->channel_configure ? ops->channel_configure : _err_io_dac_channel_configure;
    ops->write = ops->write ? ops->write : _err_io_dac_write;
    dac->ops = ops;

    /* Add to the container */
    return mr_device_add(&dac->device, name, MR_OPEN_WRONLY);
}

#endif  /* MR_CONF_DAC */