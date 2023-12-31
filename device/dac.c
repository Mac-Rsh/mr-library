/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "include/device/dac.h"

#ifdef MR_USING_DAC

static int dac_channel_set_state(struct mr_dac *dac, int channel, int state)
{
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dac->dev.drv->ops;

    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Configure the channel */
    int ret = ops->channel_configure(dac, channel, state);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Enable or disable the channel */
    if (state == MR_DAC_STATE_ENABLE)
    {
        MR_BIT_SET(dac->channel, (1 << channel));
    } else
    {
        MR_BIT_CLR(dac->channel, (1 << channel));
    }
    return MR_EOK;
}

static int dac_channel_get_state(struct mr_dac *dac, int channel)
{
    /* Check channel is valid */
    if (channel < 0 || channel >= 32)
    {
        return MR_EINVAL;
    }

    /* Check if the channel is enabled */
    return MR_BIT_IS_SET(dac->channel, (1 << channel));
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

    /* Disable all channels */
    for (size_t i = 0; i < 32; i++)
    {
        if (MR_BIT_IS_SET(dac->channel, (1 << i)) == MR_ENABLE)
        {
            ops->channel_configure(dac, (int)i, MR_DISABLE);
            MR_BIT_CLR(dac->channel, (1 << i));
        }
    }

    return ops->configure(dac, MR_DISABLE);
}

static ssize_t mr_dac_write(struct mr_dev *dev, int off, const void *buf, size_t size, int async)
{
    struct mr_dac *dac = (struct mr_dac *)dev;
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    ssize_t wr_size;

    /* Check if the channel is enabled */
    if (MR_BIT_IS_SET(dac->channel, (1 << off)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }

    MR_BIT_CLR(size, sizeof(*wr_buf) - 1);
    for (wr_size = 0; wr_size < size; wr_size += sizeof(*wr_buf))
    {
        ops->write(dac, off, *wr_buf);
        wr_buf++;
    }
    return wr_size;
}

static int mr_dac_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_dac *dac = (struct mr_dac *)dev;

    switch (cmd)
    {
        case MR_CTL_DAC_SET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                struct mr_dac_config config = *((struct mr_dac_config *)args);

                return dac_channel_set_state(dac, off, config.state);
            }
            return MR_EINVAL;
        }

        case MR_CTL_DAC_GET_CHANNEL_STATE:
        {
            if (args != MR_NULL)
            {
                int *state = (int *)args;

                int ret = dac_channel_get_state(dac, off);
                if (ret < 0)
                {
                    return ret;
                }
                *state = ret;
                return MR_EOK;
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
 * @param name The name of the dac.
 * @param drv The driver of the dac.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_dac_register(struct mr_dac *dac, const char *name, struct mr_drv *drv)
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
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(drv != MR_NULL);
    MR_ASSERT(drv->ops != MR_NULL);

    /* Initialize the fields */
    dac->channel = 0;

    /* Register the dac */
    return mr_dev_register(&dac->dev, name, Mr_Dev_Type_DAC, MR_SFLAG_WRONLY, &ops, drv);
}

#endif /* MR_USING_DAC */
