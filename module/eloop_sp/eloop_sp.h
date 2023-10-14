/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-18     MacRsh       first version
 */

#ifndef _ELOOP_SP_H_
#define _ELOOP_SP_H_

#include "mrapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MR_CFG_ELOOP_SP == MR_CFG_ENABLE)

/**
 * @struct simple event loop
 */
struct mr_eloop_sp
{
    struct mr_object object;                                        /* Eloop object */

    struct mr_rb queue;                                             /* Event queue */
    void *list;                                                     /* Event list */
    mr_size_t list_size;                                            /* Event list size */
};
typedef struct mr_eloop_sp *mr_eloop_sp_t;                          /* Type for event loop */

/**
 * @addtogroup Simple eloop
 * @{
 */
mr_eloop_sp_t mr_eloop_sp_find(const char *name);
mr_err_t mr_eloop_sp_add(mr_eloop_sp_t eloop, const char *name, mr_size_t queue_size, mr_size_t event_size);
mr_err_t mr_eloop_sp_remove(mr_eloop_sp_t eloop);
void mr_eloop_sp_handle(mr_eloop_sp_t eloop);
mr_err_t mr_eloop_sp_create_event(mr_eloop_sp_t eloop,
                                  mr_uint8_t id,
                                  mr_err_t (*cb)(mr_eloop_sp_t ep, void *args),
                                  void *args);
mr_err_t mr_eloop_sp_delete_event(mr_eloop_sp_t eloop, mr_uint8_t id);
mr_err_t mr_eloop_sp_notify_event(mr_eloop_sp_t eloop, mr_uint8_t id);
mr_err_t mr_eloop_sp_trigger_event(mr_eloop_sp_t eloop, mr_uint8_t id);
/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* _ELOOP_SP_H_ */
