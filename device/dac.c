/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "include/device/mr_dac.h"

#ifdef MR_USING_DAC

MR_INLINE int dac_channel_set_configure(struct mr_dac *dac, int channel, struct mr_dac_config config)
{
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dac->dev.drv->ops;

    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    int ret = ops->channel_configure(dac, channel, config.state);
    if (ret < 0)
    {
        return ret;
    }

    /* Enable or disable the channel */
    if (config.state == MR_ENABLE)
    {
        MR_BIT_SET(dac->channel, (1 << channel));
    } else
    {
        MR_BIT_CLR(dac->channel, (1 << channel));
    }
    return MR_EOK;
}

MR_INLINE int dac_channel_get_configure(struct mr_dac *dac, int channel, struct mr_dac_config *config)
{
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Get configure */
    config->state = MR_BIT_IS_SET(dac->channel, (1 << channel));
    return MR_EOK;
}

static int mr_dac_open(struct mr_dev *dev)
{
    struct mr_dac *dac = (struct mr_dac *)dev;
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;

    return ops->configure(dac, MR_ENABLE);
}

static int mr_dac_close(struct mr_dev *dev)
{
    struct mr_dac *dac = (struct mr_dac *)dev;
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;

#ifdef MR_USING_DAC_AUTO_DISABLE
    /* Disable all channels */
    for (size_t i = 0; i < 32; i++)
    {
        if (MR_BIT_IS_SET(dac->channel, (1 << i)) == MR_ENABLE)
        {
            ops->channel_configure(dac, (int)i, MR_DISABLE);
            MR_BIT_CLR(dac->channel, (1 << i));
        }
    }
#endif /* MR_USING_DAC_AUTO_DISABLE */

    return ops->configure(dac, MR_DISABLE);
}

static ssize_t mr_dac_write(struct mr_dev *dev, const void *buf, size_t count)
{
    struct mr_dac *dac = (struct mr_dac *)dev;
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    ssize_t wr_size;

#ifdef MR_USING_DAC_CHANNEL_CHECK
    /* Check if the channel is enabled */
    if (MR_BIT_IS_SET(dac->channel, (1 << dev->position)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_DAC_CHANNEL_CHECK */

    for (wr_size = 0; wr_size < MR_ALIGN_DOWN(count, sizeof(*wr_buf)); wr_size += sizeof(*wr_buf))
    {
        int ret = ops->write(dac, dev->position, *wr_buf);
        if (ret < 0)
        {
            return (wr_size == 0) ? ret : wr_size;
        }
        wr_buf++;
    }
    return wr_size;
}

static int mr_dac_ioctl(struct mr_dev *dev, int cmd, void *args)
{
    struct mr_dac *dac = (struct mr_dac *)dev;

    switch (cmd)
    {
        case MR_IOC_DAC_SET_CHANNEL_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_dac_config config = *((struct mr_dac_config *)args);

                int ret = dac_channel_set_configure(dac, dev->position, config);
                if (ret < 0)
                {
                    return ret;
                }
                return sizeof(config);
            }
            return MR_EINVAL;
        }
        case MR_IOC_DAC_GET_CHANNEL_CONFIG:
        {
            if (args != MR_NULL)
            {
                struct mr_dac_config *config = (struct mr_dac_config *)args;

                int ret = dac_channel_get_configure(dac, dev->position, config);
                if (ret < 0)
                {
                    return ret;
                }
                return sizeof(*config);
            }
            return MR_EINVAL;
        }
        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function registers an dac.
 *
 * @param dac The dac.
 * @param path The path of the dac.
 * @param drv The driver of the dac.
 *
 * @return 0 on success, otherwise an error code.
 */
int mr_dac_register(struct mr_dac *dac, const char *path, struct mr_drv *drv)
{
    static struct mr_dev_ops ops =
        {
            mr_dac_open,
            mr_dac_close,
            MR_NULL,
            mr_dac_write,
            mr_dac_ioctl,
            MR_NULL
        };

    MR_ASSERT(dac != MR_NULL);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    dac->channel = 0;

    /* Register the dac */
    return mr_dev_register(&dac->dev, path, MR_DEV_TYPE_DAC, MR_O_WRONLY, &ops, drv);
}

#endif /* MR_USING_DAC */
