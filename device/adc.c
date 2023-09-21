/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "adc.h"

#if (MR_CFG_ADC == MR_CFG_ENABLE)

static mr_err_t err_io_adc_configure(mr_adc_t adc, mr_state_t state)
{
    return -MR_ERR_IO;
}

static mr_err_t err_io_adc_channel_configure(mr_adc_t adc, mr_adc_config_t config)
{
    return -MR_ERR_IO;
}

static mr_uint32_t err_io_adc_read(mr_adc_t adc, mr_off_t channel)
{
    return 0;
}

static mr_err_t mr_adc_open(mr_device_t device)
{
    mr_adc_t adc = (mr_adc_t)device;

    return adc->ops->configure(adc, MR_ENABLE);
}

static mr_err_t mr_adc_close(mr_device_t device)
{
    mr_adc_t adc = (mr_adc_t)device;

    /* Disable all channel */
    adc->config.channel._mask = 0;

    return adc->ops->configure(adc, MR_DISABLE);
}

static mr_err_t mr_adc_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_adc_t adc = (mr_adc_t)device;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_adc_config_t config = (mr_adc_config_t)args;
                ret = adc->ops->channel_configure(adc, config);
                if (ret == MR_ERR_OK)
                {
                    adc->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_GET_CONFIG:
        {
            if (args)
            {
                mr_adc_config_t config = (mr_adc_config_t)args;
                *config = adc->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_adc_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_adc_t adc = (mr_adc_t)device;
    mr_uint32_t *read_buffer = (mr_uint32_t *)buffer;
    mr_size_t read_size = 0;

    if (pos < 0)
    {
        return -MR_ERR_INVALID;
    }

    while ((read_size += sizeof(*read_buffer)) <= size)
    {
        *read_buffer = adc->ops->read(adc, pos);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

/**
 * @brief This function adds the adc device.
 *
 * @param adc The adc device to be added.
 * @param name The name of the adc device.
 * @param ops The operations of the adc device.
 * @param data The private data of the adc device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_adc_device_add(mr_adc_t adc, const char *name, struct mr_adc_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
        {
            mr_adc_open,
            mr_adc_close,
            mr_adc_ioctl,
            mr_adc_read,
            MR_NULL,
        };

    MR_ASSERT(adc != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    adc->config.channel._mask = 0;

    /* Protect every operation of the adc device */
    ops->configure = ops->configure ? ops->configure : err_io_adc_configure;
    ops->channel_configure = ops->channel_configure ? ops->channel_configure : err_io_adc_channel_configure;
    ops->read = ops->read ? ops->read : err_io_adc_read;
    adc->ops = ops;

    /* Add the device */
    return mr_device_add(&adc->device, name, Mr_Device_Type_ADC, MR_DEVICE_OFLAG_RDONLY, &device_ops, data);
}

#endif