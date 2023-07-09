/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-03     MacRsh       first version
 */

#ifndef _SOFT_TIMER_H_
#define _SOFT_TIMER_H_

#include "stdint.h"

#define SOFT_TIMER_ASSERT(x)

struct soft_timer_list
{
    struct soft_timer_list *next;
    struct soft_timer_list *prev;
};
typedef struct soft_timer_list *soft_timer_list_t;

struct soft_timer_server
{
    uint32_t time;
    struct soft_timer_list list;
};
typedef struct soft_timer_server *soft_timer_server_t;

typedef struct soft_timer *soft_timer_t;

struct soft_timer
{
    struct soft_timer_list list;
    soft_timer_server_t server;
    uint32_t interval;
    uint32_t timeout;

    int (*cb)(soft_timer_t timer, void *args);
    void *args;
};

#define SOFT_TIMER_ERR_OK               0
#define SOFT_TIMER_ERR_BUSY             5

void soft_timer_server_init(soft_timer_server_t server);
void soft_timer_server_update(soft_timer_server_t server, uint32_t time);
void soft_timer_server_handle(soft_timer_server_t server);
int soft_timer_add(soft_timer_t timer,
                   uint32_t time,
                   int (*cb)(soft_timer_t timer, void *args),
                   void *args,
                   soft_timer_server_t server);
int soft_timer_remove(soft_timer_t timer);
int soft_timer_start(soft_timer_t timer);
int soft_timer_stop(soft_timer_t timer);
int soft_timer_add_then_start(soft_timer_t timer,
                              uint32_t time,
                              int (*cb)(soft_timer_t client, void *args),
                              void *args,
                              soft_timer_server_t server);

#endif /* _SOFT_TIMER_H_ */