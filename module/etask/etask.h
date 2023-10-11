/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-16     MacRsh       first version
 */

#ifndef _ETASK_H_
#define _ETASK_H_

#include "mrapi.h"
#define MR_CFG_ETASK 1
#if (MR_CFG_ETASK == MR_CFG_ENABLE)

/**
 * @def Etask wakeup flag
 */
#define MR_ETASK_WFLAG_NOW              0x00                        /* Wakeup immediately */
#define MR_ETASK_WFLAG_DELAY            0x01                        /* Wakeup after delay */

/**
 * @def Etask start flag
 */
#define MR_ETASK_SFLAG_EVENT            0x00                        /* Event */
#define MR_ETASK_SFLAG_TIMER            0x01                        /* Timer */
#define MR_ETASK_SFLAG_HARD             0x02                        /* Hard */
#define MR_ETASK_SFLAG_ONESHOT          0x04                        /* Oneshot */

/**
 * @struct Etask
 */
struct mr_etask
{
    struct mr_object object;                                        /* Etask object */

    mr_uint32_t tick;                                               /* Tick */
    struct mr_rb queue;                                             /* Queue */
    mr_avl_t list;                                                  /* Event list */
    struct mr_list tlist;                                           /* Timing list */
};
typedef struct mr_etask *mr_etask_t;                                /* Type for etask */

/**
 * @addtogroup Etask
 * @{
 */
mr_etask_t mr_etask_find(const char *name);
mr_err_t mr_etask_add(mr_etask_t etask, const char *name, mr_size_t size);
mr_err_t mr_etask_remove(mr_etask_t etask);
void mr_etask_tick_update(mr_etask_t etask);
void mr_etask_handle(mr_etask_t etask);
mr_err_t mr_etask_start(mr_etask_t etask,
                        mr_uint32_t id,
                        mr_uint8_t sflags,
                        mr_uint32_t time,
                        mr_err_t (*cb)(mr_etask_t et, void *args),
                        void *args);
mr_err_t mr_etask_stop(mr_etask_t etask, mr_uint32_t id);
mr_err_t mr_etask_wakeup(mr_etask_t etask, mr_uint32_t id, mr_uint8_t wflag);
mr_uint32_t mr_etask_str_to_id(const char *string);
/** @} */

#endif

#endif /* _ETASK_H_ */
