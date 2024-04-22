/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#include "../mr-library/device/include/mr_adc.h"

#ifdef MR_USE_ADC

#define _ADC_CHANNEL_IS_VALID(_adc, _channel)                                  \
    (((_channel) >= 0) && ((_channel) < (sizeof((_adc)->channels) * 8)))
#define _ADC_CHANNEL_IS_EXISTED(_data, _channel)                               \
    (((_data)->channels & (1 << (_channel))) != 0)
#define _ADC_CHANNEL_IS_ENABLED(_adc, _channel)                                \
    (MR_BIT_IS_SET((_adc)->channels, (1 << (_channel))))

MR_INLINE int _adc_channel_configure_set(struct mr_adc *adc, int channel,
                                         struct mr_adc_config *config)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET((struct mr_device *)adc);
    struct mr_adc_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    struct mr_adc_driver_data *data = _MR_DRIVER_DATA_GET(driver);

    /* Check if the channel is valid */
    if (_ADC_CHANNEL_IS_VALID(adc, channel) == false)
    {
        return MR_EINVAL;
    }

    /* Check if the channel is existed */
    if ((data != NULL) && (_ADC_CHANNEL_IS_EXISTED(data, channel) == false))
    {
        return MR_EINVAL;
    }

    /* Check if the channel is already configured */
    if (config->channel_state == _ADC_CHANNEL_IS_ENABLED(adc, channel))
    {
        return MR_EOK;
    }

    /* Configure the channel */
    int ret = ops->channel_configure(driver, channel, config->channel_state);
    if (ret < 0)
    {
        return ret;
    }

    /* Enable or disable the channel */
    if (config->channel_state == true)
    {
        MR_BIT_SET(adc->channels, (1 << channel));
    } else
    {
        MR_BIT_CLR(adc->channels, (1 << channel));
    }
    return MR_EOK;
}

MR_INLINE int _adc_channel_configure_get(struct mr_adc *adc, int channel,
                                         struct mr_adc_config *config)
{
    /* Check if the channel is valid */
    if (_ADC_CHANNEL_IS_VALID(adc, channel) == false)
    {
        return MR_EINVAL;
    }

    /* Get the channel configure */
    config->channel_state = _ADC_CHANNEL_IS_ENABLED(adc, channel);
    return MR_EOK;
}

static int adc_open(struct mr_device *device)
{
    struct mr_adc *adc = (struct mr_adc *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_adc_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Recover all enabled channels */
    for (uint32_t i = 0; i < (sizeof(adc->channels) * 8); i++)
    {
        if (_ADC_CHANNEL_IS_ENABLED(adc, i) == true)
        {
            ops->channel_configure(driver, i, true);
        }
    }

    /* Enable ADC */
    return ops->configure(driver, true);
}

static int adc_close(struct mr_device *device)
{
    struct mr_adc *adc = (struct mr_adc *)device;
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_adc_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);

    /* Disable all enabled channels */
    for (uint32_t i = 0; i < (sizeof(adc->channels) * 8); i++)
    {
        if (_ADC_CHANNEL_IS_ENABLED(adc, i) == true)
        {
            /* Just close the channel, without clearing the channel mask, and
             * the channel will be restored when opened */
            ops->channel_configure(driver, i, false);
        }
    }

    /* Disable ADC */
    return ops->configure(driver, false);
}

static ssize_t adc_read(struct mr_device *device, int pos, void *buf,
                        size_t count)
{
    struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
    struct mr_adc_driver_ops *ops = _MR_DRIVER_OPS_GET(driver);
    uint32_t *rbuf = (uint32_t *)buf;
    ssize_t rcount;

#ifdef MR_USE_ADC_CHECK
    struct mr_adc *adc = (struct mr_adc *)device;

    /* Check if the channel is enabled */
    if ((_ADC_CHANNEL_IS_VALID(adc, pos) == false) ||
        (_ADC_CHANNEL_IS_ENABLED(adc, pos) == false))
    {
        return MR_EINVAL;
    }
#endif /* MR_USE_ADC_CHECK */

    /* Read data */
    for (rcount = 0; rcount < MR_ALIGN_DOWN(count, sizeof(*rbuf));
         rcount += sizeof(*rbuf))
    {
        int ret = ops->read(driver, pos, rbuf);
        if (ret < 0)
        {
            /* If no data is read, return the error code */
            return (rcount == 0) ? ret : rcount;
        }
        rbuf++;
    }

    /* Return the number of bytes read */
    return rcount;
}

static int adc_ioctl(struct mr_device *device, int pos, int cmd, void *args)
{
    struct mr_adc *adc = (struct mr_adc *)device;

    switch (cmd)
    {
        case MR_CTRL_SET(MR_CMD_ADC_CHANNEL_STATE):
        {
            struct mr_adc_config *config = (struct mr_adc_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Set the channel configure */
            int ret = _adc_channel_configure_set(adc, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        case MR_CTRL_GET(MR_CMD_ADC_CHANNEL_STATE):
        {
            struct mr_adc_config *config = (struct mr_adc_config *)args;

            if (config == NULL)
            {
                return MR_EINVAL;
            }

            /* Get the channel configure */
            int ret = _adc_channel_configure_get(adc, pos, config);
            if (ret < 0)
            {
                return ret;
            }
            return sizeof(*config);
        }
        case MR_CTRL_GET(MR_CMD_ADC_RESOLUTION):
        {
            struct mr_driver *driver = _MR_DEVICE_DRIVER_GET(device);
            struct mr_adc_driver_data *data = _MR_DRIVER_DATA_GET(driver);
            uint32_t *resolution = (uint32_t *)args;

            if (resolution == NULL)
            {
                return MR_EINVAL;
            }

            /* Driver does not provide the corresponding information */
            if (data == NULL)
            {
                return MR_EPERM;
            }

            /* Get the resolution */
            *resolution = data->resolution;
            return sizeof(*resolution);
        }
        default:
        {
            return MR_EPERM;
        }
    }
}

/**
 * @brief This function register an adc.
 *
 * @param adc The adc.
 * @param path The path of the adc.
 * @param driver The driver of the adc.
 *
 * @return The error code.
 */
int mr_adc_register(struct mr_adc *adc, const char *path,
                    struct mr_driver *driver)
{
    MR_ASSERT(adc != NULL);
    MR_ASSERT(path != NULL);
    MR_ASSERT((driver != NULL) && (driver->ops != NULL));

    static struct mr_device_ops ops = {.open = adc_open,
                                       .close = adc_close,
                                       .read = adc_read,
                                       .ioctl = adc_ioctl};

    /* Initialize the adc */
    adc->channels = 0;

    /* Register the adc device */
    return mr_device_register((struct mr_device *)adc, path, MR_DEVICE_TYPE_ADC,
                              &ops, driver);
}

#endif /* MR_USE_ADC */
