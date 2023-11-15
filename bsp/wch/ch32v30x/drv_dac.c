/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-14    MacRsh       First version
 */

#include "drv_dac.h"

#ifdef MR_USING_DAC

#if !defined(MR_USING_DAC1)
#error "Please define at least one DAC macro like MR_USING_DAC1. Otherwise undefine MR_USING_DAC."
#else

static struct drv_dac_data dac_drv_data[] =
    {
#ifdef MR_USING_DAC1
        {
            RCC_APB1Periph_DAC,
        }
#endif /* MR_USING_DAC1 */
    };

static struct drv_dac_channel_data dac_channel_drv_data[] =
    {
        {DAC_Channel_1, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_4},
        {DAC_Channel_2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_5},
    };

static struct mr_dac dac_dev;

static struct drv_dac_channel_data *drv_dac_get_channel_data(int channel)
{
    if (channel >= mr_array_num(dac_channel_drv_data))
    {
        return NULL;
    }
    return &dac_channel_drv_data[channel];
}

static int drv_dac_configure(struct mr_dac *dac, int state)
{
    struct drv_dac_data *dac_data = (struct drv_dac_data *)dac->dev.drv->data;

    /* Configure clock */
    RCC_APB1PeriphClockCmd(dac_data->clock, state);
    return MR_EOK;
}

static int drv_dac_channel_configure(struct mr_dac *dac, int channel, int state)
{
    struct drv_dac_channel_data *dac_channel_data = drv_dac_get_channel_data(channel);
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    DAC_InitTypeDef DAC_InitStructure = {0};

    /* Check channel is valid */
    if (dac_channel_data == NULL)
    {
        return MR_EINVAL;
    }

    /* Configure clock */
    RCC_APB2PeriphClockCmd(dac_channel_data->gpio_clock, ENABLE);

    switch (dac_channel_data->channel)
    {
        case DAC_Channel_1:
        {
            DAC_SetChannel1Data(DAC_Align_12b_R, 0);
            break;
        }

        case DAC_Channel_2:
        {
            DAC_SetChannel2Data(DAC_Align_12b_R, 0);
            break;
        }
    }

    /* Configure pin */
    if (dac_channel_data->port != MR_NULL)
    {
        if (state == MR_ENABLE)
        {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_InitStructure.GPIO_Pin = dac_channel_data->pin;
            GPIO_Init(dac_channel_data->port, &GPIO_InitStructure);
        }
    }

    /* Configure DAC */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(dac_channel_data->channel, &DAC_InitStructure);
    DAC_Cmd(dac_channel_data->channel, state);
    return MR_EOK;
}

static void drv_dac_write(struct mr_dac *dac, int channel, uint32_t data)
{
    struct drv_dac_channel_data *dac_channel_data = drv_dac_get_channel_data(channel);

    /* Check channel is valid */
    if (dac_channel_data == NULL)
    {
        return;
    }

    /* Write data */
    switch (dac_channel_data->channel)
    {
        case DAC_Channel_1:
        {
            DAC_SetChannel1Data(DAC_Align_12b_R, data);
            break;
        }
        case DAC_Channel_2:
        {
            DAC_SetChannel2Data(DAC_Align_12b_R, data);
            break;
        }
    }
}

static struct mr_dac_ops dac_drv_ops =
    {
        drv_dac_configure,
        drv_dac_channel_configure,
        drv_dac_write,
    };

static struct mr_drv dac_drv =
    {
        Mr_Dev_Type_Dac,
        &dac_drv_ops,
        &dac_drv_data,
    };

int drv_dac_init(void)
{
    return mr_dac_register(&dac_dev, "dac1", &dac_drv);
}
MR_INIT_DRV_EXPORT(drv_dac_init);

#endif /* !defined(MR_USING_DAC1) */

#endif /* MR_USING_DAC */
