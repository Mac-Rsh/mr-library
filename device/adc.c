/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-06    MacRsh       First version
 */

#include "include/device/mr_adc.h"

#ifdef MR_USING_ADC

MR_INLINE int adc_channel_set_configure(struct mr_adc *adc,
                                        int channel,
                                        struct mr_adc_config config)
{
    struct mr_adc_ops *ops = (struct mr_adc_ops *)adc->dev.drv->ops;

    if ((channel < 0) || (channel >= (sizeof(adc->channels) * 8))) {
        return MR_EINVAL;
    }

    int ret = ops->channel_configure(adc, channel, config.state);
    if (ret < 0) {
        return ret;
    }

    /* Enable or disable the channel */
    if (config.state == MR_ENABLE) {
        MR_BIT_SET(adc->channels, (1 << channel));
    } else {
        MR_BIT_CLR(adc->channels, (1 << channel));
    }
    return MR_EOK;
}

MR_INLINE int adc_channel_get_configure(struct mr_adc *adc,
                                        int channel,
                                        struct mr_adc_config *config)
{
    if ((channel < 0) || (channel >= (sizeof(adc->channels) * 8))) {
        return MR_EINVAL;
    }

    /* Get configure */
    config->state = MR_BIT_IS_SET(adc->channels, (1 << channel));
    return MR_EOK;
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

#ifdef MR_USING_ADC_AUTO_DISABLE
    /* Disable all channels */
    for (size_t i = 0; i < (sizeof(adc->channels) * 8); i++) {
        if (MR_BIT_IS_SET(adc->channels, (1 << i)) == MR_ENABLE) {
            ops->channel_configure(adc, (int)i, MR_DISABLE);
            MR_BIT_CLR(adc->channels, (1 << i));
        }
    }
#endif /* MR_USING_ADC_AUTO_DISABLE */

    return ops->configure(adc, MR_DISABLE);
}

static ssize_t mr_adc_read(struct mr_dev *dev, void *buf, size_t count)
{
    struct mr_adc *adc = (struct mr_adc *)dev;
    struct mr_adc_ops *ops = (struct mr_adc_ops *)dev->drv->ops;
    uint32_t *rd_buf = (uint32_t *)buf;
    ssize_t rd_size;

#ifdef MR_USING_ADC_CHANNEL_CHECK
    /* Check if the channel is enabled */
    if ((dev->position < 0) || (MR_BIT_IS_SET(adc->channels, (1 << dev->position)) == MR_DISABLE)) {
        return MR_EINVAL;
    }
#endif /* MR_USING_ADC_CHANNEL_CHECK */

    for (rd_size = 0; rd_size < MR_ALIGN_DOWN(count, sizeof(*rd_buf)); rd_size += sizeof(*rd_buf)) {
        int ret = ops->read(adc, dev->position, rd_buf);
        if (ret < 0) {
            return (rd_size == 0) ? ret : rd_size;
        }
        rd_buf++;
    }
    return rd_size;
}

static int mr_adc_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_adc *adc = (struct mr_adc *)dev;

    switch (cmd) {
        case MR_IOC_ADC_SET_CHANNEL_CONFIG: {
            if (args != MR_NULL) {
                struct mr_adc_config config = *((struct mr_adc_config *)args);

                int ret = adc_channel_set_configure(adc, dev->position, config);
                if (ret < 0) {
                    return ret;
                }
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_ADC_GET_CHANNEL_CONFIG: {
            if (args != MR_NULL) {
                struct mr_adc_config *config = (struct mr_adc_config *)args;

                int ret = adc_channel_get_configure(adc, dev->position, config);
                if (ret < 0) {
                    return ret;
                }
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers an adc.
 *
 * @param adc The adc.
 * @param path The path of the adc.
 * @param drv The driver of the adc.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_adc_register(struct mr_adc *adc, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops = {mr_adc_open,
                                    mr_adc_close,
                                    mr_adc_read,
                                    MR_NULL,
                                    mr_adc_ioctl,
                                    MR_NULL};

    MR_ASSERT(adc != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    adc->channels = 0;

    /* Register the adc */
    return mr_dev_register(&adc->dev, path, MR_DEV_TYPE_ADC, MR_O_RDONLY, &ops, drv);
}

#endif /* MR_USING_ADC */
