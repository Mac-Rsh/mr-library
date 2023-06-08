/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-25     MacRsh       first version
 */

#ifndef _LIST_H_
#define _LIST_H_

#include "stdint.h"

#define LIST_ASSERT(x)

struct list
{
    struct list *next;                                              /**< Point to next node */
    struct list *prev;                                              /**< Point to prev node */
};
typedef struct list *list_t;                                        /**< Type for list */

struct slist
{
    struct slist *next;                                             /**< Point to next node */
};
typedef struct slist *slist_t;                                      /**< Type for slist */

#define slist_container_of(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

#define list_container_of(node, type, member) \
    slist_container_of(node, type, member)

void slist_init(slist_t list);
void slist_append(slist_t list, slist_t node);
void slist_insert_after(slist_t list, slist_t node);
void slist_remove(slist_t list, slist_t node);
size_t slist_get_length(slist_t list);
slist_t slist_get_tail(slist_t list);
int slist_is_empty(slist_t list);

void list_init(list_t list);
void list_insert_after(list_t list, list_t node);
void list_insert_before(list_t list, list_t node);
void list_remove(list_t node);
size_t list_get_length(list_t list);
int list_is_empty(list_t list);

#endif