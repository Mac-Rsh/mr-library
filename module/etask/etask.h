/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#ifndef _ETASK_H_
#define _ETASK_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_ETASK

/**
* @brief Etask start flag
*/
#define MR_ETASK_SFLAG_EVENT            (0x00)                      /* Event */
#define MR_ETASK_SFLAG_TIMER            (0x01)                      /* Timer */
#define MR_ETASK_SFLAG_HARD             (0x02)                      /* Hard */
#define MR_ETASK_SFLAG_SOFT             (0x00)                      /* Soft */
#define MR_ETASK_SFLAG_ONESHOT          (0x04)                      /* One shot */
#define MR_ETASK_SFLAG_PERIODIC         (0x00)                      /* Periodic */

/**
 * @brief Etask wakeup flag
 */
#define MR_ETASK_WFLAG_NOW              (0x00)                        /* Wakeup immediately */
#define MR_ETASK_WFLAG_DELAY            (0x01)                        /* Wakeup after delay */
#define MR_ETASK_WFLAG_STATE            (0x02)                        /* Wakeup by state */

/**
 * @brief Etask
 */
struct mr_etask
{
    uint32_t magic;                                                 /* Magic */
    uint32_t tick;                                                  /* Tick */
    struct mr_ringbuf queue;                                        /* Queue */
    struct mr_avl *list;                                            /* Event list */
    struct mr_list tlist;                                           /* Timing list */
    void *state;                                                    /* State */
};

/**
 * @addtogroup Etask
 * @{
 */
void mr_etask_init(void *pool, size_t size);
void mr_etask_uninit(void);
void mr_etask_tick_update(void);
void mr_etask_handle(void);
int mr_etask_start(uint32_t id, uint8_t sflags, size_t time, int (*cb)(struct mr_etask *et, void *args), void *args);
int mr_etask_stop(uint32_t id);
int mr_etask_wakeup(uint32_t id, uint8_t wflag);

void etask_init(struct mr_etask *etask, void *pool, size_t size);
void etask_uninit(struct mr_etask *etask);
void etask_tick_update(struct mr_etask *etask);
void etask_handle(struct mr_etask *etask);
int etask_start(struct mr_etask *etask,
                uint32_t id,
                uint8_t sflags,
                size_t time,
                int (*cb)(struct mr_etask *et, void *args),
                void *args);
int etask_stop(struct mr_etask *etask, uint32_t id);
int etask_wakeup(struct mr_etask *etask, uint32_t id, uint8_t wflag);
/** @} */
#endif /* MR_USING_ETASK */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ETASK_H_ */
