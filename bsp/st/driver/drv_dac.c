/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-22    MacRsh       First version
 */

#include "drv_dac.h"

#ifdef MR_USING_DAC

#if !defined(MR_USING_DAC1) && !defined(MR_USING_DAC2) && !defined(MR_USING_DAC3)
#warning "Please enable at least one DAC driver"
#endif /* !defined(MR_USING_DAC1) && !defined(MR_USING_DAC2) && !defined(MR_USING_DAC3) */

enum drv_dac_index
{
#ifdef MR_USING_DAC1
    DRV_INDEX_DAC1,
#endif /* MR_USING_DAC1 */
#ifdef MR_USING_DAC2
    DRV_INDEX_DAC2,
#endif /* MR_USING_DAC2 */
#ifdef MR_USING_DAC3
    DRV_INDEX_DAC3,
#endif /* MR_USING_DAC3 */
    DRV_INDEX_DAC_MAX
};

static const char *dac_name[] =
    {
#ifdef MR_USING_DAC1
        "dac1",
#endif /* MR_USING_DAC1 */
#ifdef MR_USING_DAC2
        "dac2",
#endif /* MR_USING_DAC2 */
#ifdef MR_USING_DAC3
        "dac3",
#endif /* MR_USING_DAC3 */
    };

static struct drv_dac_data dac_drv_data[] =
    {
#ifdef MR_USING_DAC1
        {{0}, DAC1},
#endif /* MR_USING_DAC1 */
#ifdef MR_USING_DAC2
        {{0}, DAC2},
#endif /* MR_USING_DAC2 */
#ifdef MR_USING_DAC3
        {{0}, DAC3},
#endif /* MR_USING_DAC3 */
    };

static struct drv_dac_channel_data dac_channel_drv_data[] = DRV_DAC_CHANNEL_CONFIG;

static struct mr_dac dac_dev[MR_ARRAY_NUM(dac_drv_data)];

static struct drv_dac_channel_data *drv_dac_get_channel_data(int channel)
{
    if (channel >= MR_ARRAY_NUM(dac_channel_drv_data))
    {
        return NULL;
    }
    return &dac_channel_drv_data[channel];
}

static int drv_dac_configure(struct mr_dac *dac, int state)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->dev.drv->data;
    dac_data->handle.Instance = dac_data->instance;

    if (state == ENABLE)
    {
        /* Configure DAC */
        HAL_DAC_Init(&dac_data->handle);
    } else
    {
        /* Configure DAC */
        HAL_DAC_DeInit(&dac_data->handle);
    }
    return MR_EOK;
}

static int drv_dac_channel_configure(struct mr_dac *dac, int channel, int state)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->dev.drv->data;
    struct drv_dac_channel_data *dac_channel_data = drv_dac_get_channel_data(channel);
    DAC_ChannelConfTypeDef sConfig = {0};

#ifdef MR_USING_DAC_CHANNEL_CHECK
    /* Check channel is valid */
    if (dac_channel_data == NULL)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_DAC_CHANNEL_CHECK */

    /* Configure Channel */
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
    HAL_DAC_ConfigChannel(&dac_data->handle, &sConfig, dac_channel_data->channel);
    if (state == ENABLE)
    {
        HAL_DAC_SetValue(&dac_data->handle, dac_channel_data->channel, DAC_ALIGN_12B_R, 0);
        HAL_DAC_Start(&dac_data->handle, dac_channel_data->channel);
    } else
    {
        HAL_DAC_Stop(&dac_data->handle, dac_channel_data->channel);
    }
    return MR_EOK;
}

static int drv_dac_write(struct mr_dac *dac, int channel, uint32_t data)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->dev.drv->data;
    struct drv_dac_channel_data *dac_channel_data = drv_dac_get_channel_data(channel);

#ifdef MR_USING_DAC_CHANNEL_CHECK
    /* Check channel is valid */
    if (dac_channel_data == NULL)
    {
        return MR_EINVAL;
    }
#endif /* MR_USING_DAC_CHANNEL_CHECK */

    /* Write data */
    HAL_DAC_SetValue(&dac_data->handle, dac_channel_data->channel, DAC_ALIGN_12B_R, (data & 0xFFF));
}

static struct mr_dac_ops dac_drv_ops =
    {
        drv_dac_configure,
        drv_dac_channel_configure,
        drv_dac_write,
    };

static struct mr_drv dac_drv[] =
    {
#ifdef MR_USING_DAC1
        {
            MR_DRV_TYPE_DAC,
            &dac_drv_ops,
            &dac_drv_data[DRV_INDEX_DAC1],
        },
#endif /* MR_USING_DAC1 */
#ifdef MR_USING_DAC2
        {
            MR_DRV_TYPE_DAC,
            &dac_drv_ops,
            &dac_drv_data[DRV_INDEX_DAC2],
        }
#endif /* MR_USING_DAC2 */
#ifdef MR_USING_DAC3
        {
            MR_DRV_TYPE_DAC,
            &dac_drv_ops,
            &dac_drv_data[DRV_INDEX_DAC3],
        }
#endif /* MR_USING_DAC3 */
    };

static void drv_dac_init(void)
{
    for (size_t i = 0; i < MR_ARRAY_NUM(dac_dev); i++)
    {
        mr_dac_register(&dac_dev[i], dac_name[i], &dac_drv[i]);
    }
}
MR_INIT_DRV_EXPORT(drv_dac_init);

#endif /* MR_USING_DAC */
