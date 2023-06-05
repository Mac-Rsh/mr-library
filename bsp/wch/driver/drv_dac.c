/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     MacRsh       first version
 */

#include "drv_dac.h"

#if (MR_CONF_DAC == MR_CONF_ENABLE)

static struct ch32_dac ch32_dac[] =
        {
#ifdef BSP_DAC_1
                {"dac1",
                 DAC_Channel_1,
                 RCC_APB1Periph_DAC},
#endif
#ifdef BSP_DAC_2
                {"dac2",
                 DAC_Channel_2,
                 RCC_APB1Periph_DAC},
#endif
        };

static struct mr_dac hw_dac[mr_array_get_length(ch32_dac)];

mr_err_t ch32_dac_configure(mr_dac_t dac, mr_uint8_t state)
{
    struct ch32_dac *hw = (struct ch32_dac *)dac->device.data;
    DAC_InitTypeDef DAC_InitType = {0};

    RCC_APB1PeriphClockCmd(hw->hw_dac.dac_periph_clock, (FunctionalState)state);

    switch (hw->hw_dac.dac_channel)
    {
        case DAC_Channel_1:
            DAC_SetChannel1Data(DAC_Align_12b_R, 0);
            break;
        case DAC_Channel_2:
            DAC_SetChannel2Data(DAC_Align_12b_R, 0);
            break;

        default:
            return -MR_ERR_INVALID;
    }

    DAC_InitType.DAC_Trigger = DAC_Trigger_None;
    DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(hw->hw_dac.dac_channel, &DAC_InitType);
    DAC_Cmd(hw->hw_dac.dac_channel, (FunctionalState)state);

    return MR_ERR_OK;
}

mr_err_t ch32_dac_channel_configure(mr_dac_t dac, struct mr_dac_config *config)
{
    struct ch32_dac *hw = (struct ch32_dac *)dac->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    switch (hw->hw_dac.dac_channel)
    {
        case DAC_Channel_1:
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
            break;
        case DAC_Channel_2:
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            break;

        default:
            return -MR_ERR_INVALID;
    }

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);

    return MR_ERR_OK;
}

void ch32_dac_write(mr_dac_t dac, mr_uint16_t channel, mr_uint32_t value)
{
    struct ch32_dac *hw = (struct ch32_dac *)dac->device.data;

    switch (hw->hw_dac.dac_channel)
    {
        case DAC_Channel_1:
            DAC_SetChannel1Data(DAC_Align_12b_R, value);
            break;
        case DAC_Channel_2:
            DAC_SetChannel2Data(DAC_Align_12b_R, value);
            break;
        default:
            return;
    }
}

mr_err_t ch32_dac_init(void)
{
    mr_err_t ret = MR_ERR_OK;
    mr_size_t count = mr_array_get_length(hw_dac);
    static struct mr_dac_ops ops =
            {
                    ch32_dac_configure,
                    ch32_dac_channel_configure,
                    ch32_dac_write,
            };

    while (count--)
    {
        ret = mr_dac_device_add(&hw_dac[count], ch32_dac[count].name, &ops, &ch32_dac[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}

AUTO_INIT_DRIVER_EXPORT(ch32_dac_init);

#endif