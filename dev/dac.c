/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-08    MacRsh       First version
 */

#include "dac.h"

#ifdef MR_USING_DAC

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
    int i = 0;
    for (i = 0; i < 32; i++)
    {
        if (mr_bits_is_set(dac->channel, (1 << i)) == MR_ENABLE)
        {
            ops->channel_configure(dac, i, MR_DISABLE);
            mr_bits_clr(dac->channel, (1 << i));
        }
    }

    return ops->configure(dac, MR_DISABLE);
}

static ssize_t mr_dac_write(struct mr_dev *dev, int off, const void *buf, size_t size, int sync_or_async)
{
    struct mr_dac *dac = (struct mr_dac *)dev;
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;
    uint32_t *wr_buf = (uint32_t *)buf;
    ssize_t wr_size = 0;

    /* Check if the channel is enabled */
    if (mr_bits_is_set(dac->channel, (1 << off)) == MR_DISABLE)
    {
        return MR_EINVAL;
    }

    mr_bits_clr(size, sizeof(*wr_buf) - 1);
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
    struct mr_dac_ops *ops = (struct mr_dac_ops *)dev->drv->ops;

    switch (cmd)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args != MR_NULL)
            {
                /* Check offset is valid */
                if (off < 0 || off >= 32)
                {
                    return MR_EINVAL;
                }

                struct mr_dac_config *config = (struct mr_dac_config *)args;
                int ret = ops->channel_configure(dac, off, config->enable);
                if (ret == MR_EOK)
                {
                    if (config->enable == MR_DAC_ENABLE)
                    {
                        mr_bits_set(dac->channel, (1 << off));
                    } else
                    {
                        mr_bits_clr(dac->channel, (1 << off));
                    }
                }
                return ret;
            }
            return MR_EINVAL;
        }

        case MR_CTRL_GET_CONFIG:
        {
            if (args != MR_NULL)
            {
                /* Check offset is valid */
                if (off < 0 || off >= 32)
                {
                    return MR_EINVAL;
                }

                struct mr_dac_config *config = (struct mr_dac_config *)args;
                config->enable = mr_bits_is_set(dac->channel, (1 << off));
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_EINVAL;
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

    mr_assert(dac != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(drv != MR_NULL);
    mr_assert(drv->ops != MR_NULL);

    /* Initialize the fields */
    dac->channel = 0;

    /* Register the dac */
    return mr_dev_register(&dac->dev, name, Mr_Dev_Type_Dac, MR_SFLAG_WRONLY, &ops, drv);
}

#endif /* MR_USING_DAC */
