/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#include "include/device/adc.h"

#ifdef MR_USING_ADC

static int adc_channel_set_state(struct mr_adc *adc, int channel, int state)
{
    struct mr_adc_ops *ops = (struct mr_adc_ops *)adc->dev.drv->ops;

    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Configure the channel */
    int ret = ops->channel_configure(adc, channel, state);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Enable or disable the channel */
    if (state == MR_ADC_STATE_ENABLE)
    {
        mr_bits_set(adc->channel, (1 << channel));
    } else
    {
        mr_bits_clr(adc->channel, (1 << channel));
    }
    return MR_EOK;
}

static int adc_channel_get_state(struct mr_adc *adc, int channel)
{
    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    return mr_bits_is_set(adc->channel, (1 << channel));
}

static int mr_adc_open(struct mr_dev *dev)
{
    struct mr_adc *adc = (struct mr_adc *)dev;
    struct mr_adc_ops *ops = (struct mr_adc_ops *)dev->drv->ops;

    return ops->configure(adc, MR_ENABLE);
}

static int mr_adc_close(struct mr_dev *dev)
{
    struct mr_adc *adc = (struct mr_adc *)dev;
    struct mr_adc_ops *ops = (struct mr_adc_ops *)dev->drv->ops;

    /* Disable all channels */
    int i = 0;
    for (i = 0; i < 32; i++)
    {
        if (mr_bits_is_set(adc->channel, (1 << i)) == MR_ENABLE)
        {
            ops->channel_configure(adc, i, MR_DISABLE);
            mr_bits_clr(adc->channel, (1 << i));
        }
    }

    return ops->configure(adc, MR_DISABLE);
}

static ssize_t mr_adc_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_adc *adc = (struct mr_adc *)dev;
    struct mr_adc_ops *ops = (struct mr_adc_ops *)dev->drv->ops;
    uint32_t *rd_buf = (uint32_t *)buf;
    ssize_t rd_size = 0;

    /* Check if the channel is enabled */
    if (mr_bits_is_set(adc->channel, (1 << off)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }

    mr_bits_clr(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        *rd_buf = ops->read(adc, off);
        rd_buf++;
    }
    return rd_size;
}

static int mr_adc_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_adc *adc = (struct mr_adc *)dev;

    switch (cmd)
    {
        case MR_CTL_ADC_SET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                struct mr_adc_config config = *((struct mr_adc_config *)args);

                return adc_channel_set_state(adc, off, config.state);
            }
            return MR_EINVAL;
        }

        case MR_CTL_ADC_GET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                struct mr_adc_config *config = (struct mr_adc_config *)args;

                int ret = adc_channel_get_state(adc, off);
                if (ret < 0)
                {
                    return ret;
                }
                config->state = ret;
                return MR_EOK;
            }
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers an adc.
 *
 * @param adc The adc.
 * @param name The name of the adc.
 * @param drv The driver of the adc.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_adc_register(struct mr_adc *adc, const char *name, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            mr_adc_open,
            mr_adc_close,
            mr_adc_read,
            MR_NULL,
            mr_adc_ioctl,
            MR_NULL
        };

    mr_assert(adc != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Initialize the fields */
    adc->channel = 0;

    /* Register the adc */
    return mr_dev_register(&adc->dev, name, Mr_Dev_Type_ADC, MR_SFLAG_RDONLY, &ops, drv);
}

#endif /* MR_USING_ADC */
