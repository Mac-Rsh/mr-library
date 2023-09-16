/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-16     MacRsh       first version
 */

#ifndef _ELOOP_H_
#define _ELOOP_H_

#include "mrapi.h"

#if (MR_CFG_ELOOP == MR_CFG_ENABLE)

/**
 * @struct Event loop
 */
struct mr_eloop
{
    struct mr_object object;                                        /* Eloop object */

    struct mr_rb queue;                                             /* Event queue */
    mr_avl_t list;                                                  /* Event list */
};
typedef struct mr_eloop *mr_eloop_t;                                /* Type for event loop */

/**
 * @addtogroup Eloop
 * @{
 */
mr_eloop_t mr_eloop_find(const char *name);
mr_err_t mr_eloop_add(mr_eloop_t eloop, const char *name, mr_size_t queue_size);
mr_err_t mr_eloop_remove(mr_eloop_t eloop);
void mr_eloop_handle(mr_eloop_t eloop);
mr_err_t mr_eloop_create_event(mr_eloop_t eloop, mr_uint32_t id, mr_err_t (*cb)(mr_eloop_t ep, void *args), void *args);
mr_err_t mr_eloop_delete_event(mr_eloop_t eloop, mr_uint32_t id);
mr_err_t mr_eloop_notify_event(mr_eloop_t eloop, mr_uint32_t id);
mr_err_t mr_eloop_trigger_event(mr_eloop_t eloop, mr_uint32_t id);
mr_uint32_t mr_eloop_string_to_id(const char *string, mr_size_t size);
/** @} */

#endif

#endif /* _ELOOP_H_ */
