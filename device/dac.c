/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "dac.h"

#if (MR_CFG_DAC == MR_CFG_ENABLE)

static mr_err_t err_io_dac_configure(mr_dac_t dac, mr_state_t state)
{
    return MR_ERR_IO;
}

static mr_err_t err_io_dac_channel_configure(mr_dac_t dac, struct mr_dac_config *config)
{
    return MR_ERR_IO;
}

static void err_io_dac_write(mr_dac_t dac, mr_off_t channel, mr_uint32_t value)
{

}

static mr_err_t mr_dac_open(mr_device_t device)
{
    mr_dac_t dac = (mr_dac_t)device;

    return dac->ops->configure(dac, MR_ENABLE);
}

static mr_err_t mr_dac_close(mr_device_t device)
{
    mr_dac_t dac = (mr_dac_t)device;

    /* Disable all channel */
    dac->config.channel._mask = 0;

    return dac->ops->configure(dac, MR_DISABLE);
}

static mr_err_t mr_dac_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_dac_t dac = (mr_dac_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_dac_config_t config = (mr_dac_config_t)args;
                ret = dac->ops->channel_configure(dac, config);
                if (ret == MR_ERR_OK)
                {
                    dac->config = *config;
                }
                return ret;
            }
            return MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_dac_config_t config = (mr_dac_config_t)args;
                *config = dac->config;
                return MR_ERR_OK;
            }
            return MR_ERR_INVALID;
        }

        default:
            return MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_dac_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_dac_t dac = (mr_dac_t)device;
    mr_uint32_t *write_buffer = (mr_uint32_t *)buffer;
    mr_size_t write_size = 0;

    if (pos < 0)
    {
        return MR_ERR_INVALID;
    }

    while ((write_size += sizeof(*write_buffer)) <= size)
    {
        dac->ops->write(dac, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the dac device.
 *
 * @param dac The dac device to be added.
 * @param name The name of the dac device.
 * @param ops The operations of the dac device.
 * @param data The private data of the dac device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_dac_device_add(mr_dac_t dac, const char *name, struct mr_dac_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
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
    dac->config.channel._mask = 0;

    /* Protect every operation of the dac device */
    ops->configure = ops->configure ? ops->configure : err_io_dac_configure;
    ops->channel_configure = ops->channel_configure ? ops->channel_configure : err_io_dac_channel_configure;
    ops->write = ops->write ? ops->write : err_io_dac_write;
    dac->ops = ops;

    /* Add the device */
    return mr_device_add(&dac->device, name, Mr_Device_Type_DAC, MR_DEVICE_OFLAG_WRONLY, &device_ops, data);
}

#endif