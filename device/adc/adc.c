/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/adc/adc.h"

#if (MR_CONF_ADC == MR_CONF_ENABLE)

static mr_err_t mr_adc_open(mr_device_t device)
{
    mr_adc_t adc = (mr_adc_t)device;

    return adc->ops->configure(adc, MR_ENABLE);
}

static mr_err_t mr_adc_close(mr_device_t device)
{
    mr_adc_t adc = (mr_adc_t)device;

    /* Disable all channel */
    adc->config._channel_mask = 0;

    return adc->ops->configure(adc, MR_DISABLE);
}

static mr_err_t mr_adc_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_adc_t adc = (mr_adc_t)device;
    struct mr_adc_config *config = MR_NULL;
    mr_err_t ret = MR_ERR_OK;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                config = (struct mr_adc_config *)args;
                ret = adc->ops->channel_configure(adc, (struct mr_adc_config *)args);
                if (ret == MR_ERR_OK)
                {
                    adc->config = *config;
                }
                return ret;
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args)
            {
                config = (struct mr_adc_config *)args;
                *config = adc->config;
                return MR_ERR_OK;
            }
            return -MR_ERR_INVALID;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_adc_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_adc_t adc = (mr_adc_t)device;
    mr_uint32_t *recv_buffer = (mr_uint32_t *)buffer;
    mr_size_t recv_size = 0;

    if (size < sizeof(*recv_buffer))
    {
        return -MR_ERR_INVALID;
    }

    /* Check whether the channel is enabled */
    if (!((1 << pos) & adc->config._channel_mask))
    {
        return -MR_ERR_INVALID;
    }

    for (recv_size = 0; recv_size < size; recv_size += sizeof(*recv_buffer))
    {
        *recv_buffer = adc->ops->read(adc, pos);
        recv_buffer++;
    }

    return (mr_ssize_t)recv_size;
}

static mr_err_t _err_io_adc_configure(mr_adc_t adc, mr_state_t state)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static mr_err_t _err_io_adc_channel_configure(mr_adc_t adc, struct mr_adc_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static mr_uint32_t _err_io_adc_read(mr_adc_t adc, mr_pos_t channel)
{
    MR_ASSERT(0);
    return 0;
}

mr_err_t mr_adc_device_add(mr_adc_t adc, const char *name, void *data, struct mr_adc_ops *ops)
{
    const static struct mr_device_ops device_ops =
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
    adc->device.type = MR_DEVICE_TYPE_ADC;
    adc->device.data = data;
    adc->device.ops = &device_ops;

    adc->config._channel_mask = 0;

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_adc_configure;
    ops->channel_configure = ops->channel_configure ? ops->channel_configure : _err_io_adc_channel_configure;
    ops->read = ops->read ? ops->read : _err_io_adc_read;
    adc->ops = ops;

    /* Add to the container */
    return mr_device_add(&adc->device, name, MR_OPEN_RDONLY);
}

#endif  /* MR_CONF_ADC */