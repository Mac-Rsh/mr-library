/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-03     MacRsh       first version
 */

#ifndef _EVENT_H_
#define _EVENT_H_

#include "stdint.h"
#include "malloc.h"
#include "string.h"

#define EVENT_ASSERT(x)

struct event_fifo
{
    uint16_t read_mirror: 1;
    uint16_t read_index: 15;
    uint16_t write_mirror: 1;
    uint16_t write_index: 15;

    uint16_t size;
    uint8_t *buffer;
};
typedef struct event_fifo *event_fifo_t;

struct event_avl
{
    int8_t height;
    uint32_t value;

    struct event_avl *left_child;
    struct event_avl *right_child;
};
typedef struct event_avl *event_avl_t;

struct event_server
{
    struct event_fifo queue;
    event_avl_t list;
};
typedef struct event_server *event_server_t;

struct event
{
    struct event_avl list;

    int (*cb)(event_server_t server, void *args);
    void *args;
};
typedef struct event *event_t;

#define EVENT_ERR_OK                 0
#define EVENT_ERR_GENERIC            1
#define EVENT_ERR_NO_MEMORY          2
#define EVENT_ERR_BUSY               5
#define EVENT_ERR_NOT_FOUND          6

int event_server_init(event_server_t server, size_t queue_length);
int event_server_uninit(event_server_t server);
void event_server_handle(event_server_t server);
int event_create(uint8_t id,
                 int (*cb)(event_server_t server, void *args),
                 void *args,
                 event_server_t server);
int event_delete(uint8_t id, event_server_t server);
int event_notify(uint8_t id, event_server_t server);

#endif /* _EVENT_H_ */
