/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-8     MacRsh       first version
 */

#include "drv_timer.h"

#if (MR_CFG_TIMER == MR_CFG_ENABLE)

static struct drv_timer_data drv_timer_data[] =
    {
#ifdef MR_BSP_TIMER_1
        {"timer1", /* ... */},
#endif
#ifdef MR_BSP_TIMER_2
        {"timer2", /* ... */},
#endif
#ifdef MR_BSP_TIMER_3
        {"timer3", /* ... */},
#endif
        /* ... */
    };

static struct mr_timer_data timer_device_data = {/* ... */};

static struct mr_timer timer_device[mr_array_num(drv_timer_data)];

static mr_err_t drv_timer_configure(mr_timer_t timer, mr_state_t state)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->device.data;

    /* ... */

    return MR_ERR_OK;
}

static void drv_timer_start(mr_timer_t timer, mr_uint32_t prescaler, mr_uint32_t period)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->device.data;

    /* ... */
}

static void drv_timer_stop(mr_timer_t timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->device.data;

    /* ... */
}

static mr_uint32_t drv_timer_get_count(mr_timer_t timer)
{
    struct drv_timer_data *timer_data = (struct drv_timer_data *)timer->device.data;
    mr_uint32_t count = 0;

    /* ... */

    return count;
}

mr_err_t drv_timer_init(void)
{
    static struct mr_timer_ops drv_ops =
        {
            drv_timer_configure,
            drv_timer_start,
            drv_timer_stop,
            drv_timer_get_count,
        };
    mr_size_t count = mr_array_num(timer_device);
    mr_err_t ret = MR_ERR_OK;

    while (count--)
    {
        ret = mr_timer_device_add(&timer_device[count],
                                  drv_timer_data[count].name,
                                  &drv_ops,
                                  &timer_device_data,
                                  &drv_timer_data[count]);
        MR_ASSERT(ret == MR_ERR_OK);
    }

    return ret;
}
MR_INIT_DRIVER_EXPORT(drv_timer_init);

#endif
