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

#if (MR_CFG_DAC == MR_CFG_ENABLE)

static struct ch32_dac_data ch32_dac_data[] =
    {
#ifdef MR_BSP_DAC_1
        {"dac1", DAC_Channel_1, RCC_APB1Periph_DAC},
#endif
#ifdef MR_BSP_DAC_2
        {"dac2", DAC_Channel_2, RCC_APB1Periph_DAC},
#endif
    };

static struct mr_dac dac_device[mr_array_num(ch32_dac_data)];

static mr_err_t ch32_dac_configure(mr_dac_t dac, mr_state_t state)
{
    struct ch32_dac_data *dac_data = (struct ch32_dac_data *)dac->device.data;
    DAC_InitTypeDef DAC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(dac_data->periph_clock, (FunctionalState)state);

    switch (dac_data->channel)
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

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(dac_data->channel, &DAC_InitStructure);
    DAC_Cmd(dac_data->channel, (FunctionalState)state);

    return MR_ERR_OK;
}

static mr_err_t ch32_dac_channel_configure(mr_dac_t dac, mr_dac_config_t config)
{
    struct ch32_dac_data *dac_data = (struct ch32_dac_data *)dac->device.data;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    switch (dac_data->channel)
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

    if (config->channel._mask != 0)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);
    } else
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOA, GPIO_InitStructure.GPIO_Pin);
    }

    return MR_ERR_OK;
}

static void ch32_dac_write(mr_dac_t dac, mr_off_t channel, mr_uint32_t value)
{
    struct ch32_dac_data *dac_data = (struct ch32_dac_data *)dac->device.data;

    switch (dac_data->channel)
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

mr_err_t drv_dac_init(void)
{
    static struct mr_dac_ops drv_ops =
        {
            ch32_dac_configure,
            ch32_dac_channel_configure,
            ch32_dac_write,
        };
    mr_size_t count = mr_array_num(dac_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_dac_device_add(&dac_device[count], ch32_dac_data[count].name, &drv_ops, &ch32_dac_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return MR_ERR_OK;
}
MR_INIT_DRIVER_EXPORT(drv_dac_init);

#endif
