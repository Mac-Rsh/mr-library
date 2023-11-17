/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#include "etask.h"

#ifdef MR_USING_ETASK

static struct mr_etask _etask = {0};

struct mr_event
{
    struct mr_avl list;
    struct mr_list tlist;
    union
    {
        struct
        {
            uint32_t timer: 1;
            uint32_t hard: 1;
            uint32_t oneshot: 1;
        };
        uint32_t _sflags: 3;
    } sflags;
    uint32_t interval: 29;
    uint32_t timeout;

    int (*cb)(struct mr_etask *et, void *args);
    void *args;
};

static void etask_timing(struct mr_etask *etask, struct mr_event *event, uint32_t time)
{
    struct mr_list *list = MR_NULL;

    if (event->sflags.timer == MR_DISABLE)
    {
        return;
    }

    if (time == 0)
    {
        /* Disable interrupt */
        mr_interrupt_disable();

        /* Insert the event into the etask list */
        mr_avl_remove(&etask->list, &event->list);
        if (mr_list_is_empty(&etask->tlist) == MR_TRUE)
        {
            mr_list_remove(&event->tlist);
        }

        /* Enable interrupt */
        mr_interrupt_enable();

        /* Free the event */
        mr_free(event);
        return;
    }

    event->interval = (event->sflags.oneshot == MR_ENABLE) ? 0 : time;
    event->timeout = etask->tick + time;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask timer-list */
    for (list = etask->tlist.next; list != &etask->tlist; list = list->next)
    {
        struct mr_event *be_insert = (struct mr_event *)mr_container_of(list, struct mr_event, tlist);

        if (event->timeout < be_insert->timeout)
        {
            mr_list_insert_before(&be_insert->tlist, &event->tlist);
            break;
        }
    }
    if (mr_list_is_empty(&event->tlist) == MR_TRUE)
    {
        mr_list_insert_before(&etask->tlist, &event->tlist);
    }

    /* Enable interrupt */
    mr_interrupt_enable();
}

static void mr_etask_free_event(struct mr_avl *tree)
{
    if (tree->left_child != MR_NULL)
    {
        mr_etask_free_event(tree->left_child);
        tree->left_child = MR_NULL;
    }

    if (tree->right_child != MR_NULL)
    {
        mr_etask_free_event(tree->right_child);
        tree->right_child = MR_NULL;
    }

    struct mr_event *event = (struct mr_event *)mr_container_of(tree, struct mr_event, list);
    mr_free(event);
}

void etask_init(struct mr_etask *etask, void *pool, size_t size)
{
    mr_assert(etask->magic != MR_MAGIC_NUMBER);
    mr_assert((pool != MR_NULL) || (size == 0));

    /* Initialize the fields */
    etask->magic = MR_MAGIC_NUMBER;
    etask->tick = 0;
    mr_ringbuf_init(&etask->queue, pool, size);
    etask->list = MR_NULL;
    mr_list_init(&etask->tlist);
    etask->state = MR_NULL;
}

void etask_uninit(struct mr_etask *etask)
{
    mr_assert(etask->magic == MR_MAGIC_NUMBER);

    /* Free the queue */
    mr_free(etask->queue.buffer);
    mr_ringbuf_init(&etask->queue, MR_NULL, 0);
    mr_etask_free_event(etask->list);
    etask->list = MR_NULL;
    mr_list_init(&etask->tlist);
    etask->state = MR_NULL;
}

void etask_tick_update(struct mr_etask *etask)
{
    struct mr_list *list = MR_NULL;

    etask->tick++;

    for (list = etask->tlist.next; list != &etask->tlist; list = list->next)
    {
        struct mr_event *event = (struct mr_event *)mr_container_of(list, struct mr_event, tlist);

        /* Check whether the current tick is larger than the timeout */
        if ((etask->tick - event->timeout) >= UINT16_MAX)
        {
            break;
        }

        /* Remove a timeout event */
        list = list->prev;
        mr_list_remove(&event->tlist);

        /* Call the callback */
        if (event->sflags.hard == MR_ENABLE)
        {
            event->cb(etask, event->args);
        } else
        {
            etask_wakeup(etask, event->list.value, MR_ETASK_WFLAG_DELAY);
        }

        /* Update the timing */
        etask_timing(etask, event, event->interval);
    }
}

void etask_handle(struct mr_etask *etask)
{
    size_t count = 0;
    uint32_t id = 0;

    mr_assert(etask != MR_NULL);

    /* Get the number of current events */
    count = mr_ringbuf_get_data_size(&etask->queue);

    /* Read the event id from the queue */
    while (count != 0)
    {
        /* Read the event id */
        count -= mr_ringbuf_read(&etask->queue, &id, sizeof(id));

        /* Find the event */
        struct mr_event *event = (struct mr_event *)mr_avl_find(etask->list, id);
        if (event == MR_NULL)
        {
            continue;
        }

        /* Call the event callback */
        event->cb(etask, event->args);
    }

    if (etask->state != MR_NULL)
    {
        struct mr_event *event = (struct mr_event *)etask->state;

        /* Call the state callback */
        event->cb(etask, event->args);
    }
}

int etask_start(struct mr_etask *etask,
                uint32_t id,
                uint8_t sflags,
                size_t time,
                int (*cb)(struct mr_etask *et, void *args),
                void *args)
{
    struct mr_event *event = MR_NULL;

    mr_assert(etask != MR_NULL);
    mr_assert(((sflags & MR_ETASK_SFLAG_TIMER) != MR_ETASK_SFLAG_TIMER) || time != 0);
    mr_assert(cb != MR_NULL);

    /* Check if the event already exists */
    if (mr_avl_find(etask->list, id) != MR_NULL)
    {
        return MR_EBUSY;
    }

    /* Allocate the event */
    event = (struct mr_event *)mr_malloc(sizeof(struct mr_event));
    if (event == MR_NULL)
    {
        return MR_ENOMEM;
    }

    /* Initialize the fields */
    mr_avl_init(&event->list, id);
    mr_list_init(&event->tlist);
    event->sflags._sflags = sflags;
    event->cb = cb;
    event->args = args;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask list */
    mr_avl_insert(&(etask->list), &(event->list));

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Start the timer */
    etask_timing(etask, event, time);
    return MR_EOK;
}

int etask_stop(struct mr_etask *etask, uint32_t id)
{
    struct mr_event *event = MR_NULL;

    mr_assert(etask != MR_NULL);

    /* Check if the event already exists */
    event = (struct mr_event *)mr_avl_find(etask->list, id);
    if (event == MR_NULL)
    {
        return MR_ENOTFOUND;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the event into the etask list */
    mr_avl_remove(&etask->list, &event->list);
    if (mr_list_is_empty(&etask->tlist) == MR_FALSE)
    {
        mr_list_remove(&event->tlist);
    }

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the event */
    mr_free(event);
    return MR_EOK;
}

int etask_wakeup(struct mr_etask *etask, uint32_t id, uint8_t wflag)
{
    mr_assert(etask != MR_NULL);
    mr_assert(wflag == MR_ETASK_WFLAG_DELAY || wflag == MR_ETASK_WFLAG_NOW || wflag == MR_ETASK_WFLAG_STATE);

    if (wflag == MR_ETASK_WFLAG_DELAY)
    {
        if (mr_ringbuf_write(&etask->queue, &id, sizeof(id)) != sizeof(id))
        {
            return MR_EBUSY;
        }
    } else
    {
        struct mr_event *event = (struct mr_event *)mr_avl_find(etask->list, id);
        if (event == MR_NULL)
        {
            return MR_ENOTFOUND;
        }

        if (wflag == MR_ETASK_WFLAG_NOW)
        {
            /* Call the event callback */
            event->cb(etask, event->args);
        } else
        {
            /* Set the state machine */
            etask->state = (void *)event;
        }
    }
    return MR_EOK;
}

/**
 * @brief This function initializes the etask module.
 *
 * @param pool The queue pool of the etask.
 * @param size The size of the queue pool.
 */
void mr_etask_init(void *pool, size_t size)
{
    etask_init(&_etask, pool, size);
}

/**
 * @brief This function uninitializes the etask module.
 */
void mr_etask_uninit(void)
{
    etask_uninit(&_etask);
}

/**
 * @brief This function updates the etask module tick.
 */
void mr_etask_tick_update(void)
{
    etask_tick_update(&_etask);
}

/**
 * @brief This function handles events of the etask queue.
 */
void mr_etask_handle(void)
{
    etask_handle(&_etask);
}

/**
 * @brief This function starts an event.
 *
 * @param id The id of the event.
 * @param sflags The start flags of the event.
 * @param time The time of the event.
 * @param cb The callback function of the event.
 * @param args The arguments of the callback function.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_etask_start(uint32_t id, uint8_t sflags, size_t time, int (*cb)(struct mr_etask *et, void *args), void *args)
{
    return etask_start(&_etask, id, sflags, time, cb, args);
}

/**
 * @brief This function stops an event.
 *
 * @param id The id of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_etask_stop(uint32_t id)
{
    return etask_stop(&_etask, id);
}

/**
 * @brief This function wakes up an event.
 *
 * @param id The id of the event.
 * @param wflag The wakeup flag of the event.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_etask_wakeup(uint32_t id, uint8_t wflag)
{
    return etask_wakeup(&_etask, id, wflag);
}

#endif /* MR_USING_ETASK */