/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-03     MacRsh       first version
 */

#include "event.h"

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#define max(a, b)    ((a) > (b)) ? a:b
#define min(a, b)    ((a) < (b)) ? a:b

static void event_fifo_init(event_fifo_t fifo, void *pool, size_t pool_size)
{
    EVENT_ASSERT(fifo != NULL);
    EVENT_ASSERT(pool != NULL);

    fifo->read_index = 0;
    fifo->write_index = 0;
    fifo->read_mirror = 0;
    fifo->write_mirror = 0;

    fifo->size = pool_size;
    fifo->buffer = pool;
}

static size_t event_fifo_get_data_size(event_fifo_t fifo)
{
    EVENT_ASSERT(fifo != NULL);

    /* Empty or full according to the mirror flag */
    if (fifo->read_index == fifo->write_index)
    {
        if (fifo->read_mirror == fifo->write_mirror)
        {
            return 0;
        } else
        {
            return fifo->size;
        }
    }

    if (fifo->write_index > fifo->read_index)
    {
        return fifo->write_index - fifo->read_index;
    } else
    {
        return fifo->size - fifo->read_index + fifo->write_index;
    }
}

static size_t event_fifo_read(event_fifo_t fifo, void *buffer, size_t size)
{
    uint8_t *buf = (uint8_t *)buffer;
    size_t length = 0;

    EVENT_ASSERT(fifo != NULL);
    EVENT_ASSERT(buffer != NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the data size */
    length = event_fifo_get_data_size(fifo);

    /* If there is no data to sda_read, return 0 */
    if (length == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to sda_read if it exceeds the available data */
    if (size > length)
    {
        size = length;
    }

    /* Copy the data from the fifo to the buffer */
    if ((fifo->size - fifo->read_index) > size)
    {
        memcpy(buf, &fifo->buffer[fifo->read_index], size);
        fifo->read_index += size;

        return size;
    }

    memcpy(buf, &fifo->buffer[fifo->read_index], fifo->size - fifo->read_index);
    memcpy(&buf[fifo->size - fifo->read_index], &fifo->buffer[0], size - (fifo->size - fifo->read_index));

    fifo->read_mirror = ~fifo->read_mirror;
    fifo->read_index = size - (fifo->size - fifo->read_index);

    return size;
}

static size_t event_fifo_write(event_fifo_t fifo, const void *buffer, size_t size)
{
    uint8_t *buf = (uint8_t *)buffer;
    size_t length = 0;

    EVENT_ASSERT(fifo != NULL);
    EVENT_ASSERT(buffer != NULL);

    if (size == 0)
    {
        return 0;
    }

    /* Get the space size */
    length = fifo->size - event_fifo_get_data_size(fifo);

    /* If there is no space to write, return 0 */
    if (length == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to write if it exceeds the available data */
    if (size > length)
    {
        size = length;
    }

    /* Copy the data from the buffer to the fifo */
    if ((fifo->size - fifo->write_index) > size)
    {
        memcpy(&fifo->buffer[fifo->write_index], buf, size);
        fifo->write_index += size;

        return size;
    }

    memcpy(&fifo->buffer[fifo->write_index], buf, fifo->size - fifo->write_index);
    memcpy(&fifo->buffer[0], &buf[fifo->size - fifo->write_index], size - (fifo->size - fifo->write_index));

    fifo->write_mirror = ~fifo->write_mirror;
    fifo->write_index = size - (fifo->size - fifo->write_index);

    return size;
}

static int8_t event_avl_get_height(event_avl_t node)
{
    if (node == NULL)
    {
        return -1;
    }

    return node->height;
}

static int8_t event_avl_get_balance(event_avl_t node)
{
    if (node == NULL)
    {
        return 0;
    }

    return (int8_t)(event_avl_get_height(node->left_child) - event_avl_get_height(node->right_child));
}

static void event_avl_left_rotate(event_avl_t *node)
{
    event_avl_t right_child = (*node)->right_child;

    EVENT_ASSERT(node != NULL);

    (*node)->right_child = right_child->left_child;
    right_child->left_child = (*node);

    (*node)->height = max(event_avl_get_height((*node)->left_child), event_avl_get_height((*node)->right_child)) +
                                                                                                                       1;
    right_child->height =
                    max(event_avl_get_height(right_child->left_child),
                        event_avl_get_height(right_child->right_child)) + 1;

    (*node) = right_child;
}

static void event_avl_right_rotate(event_avl_t *node)
{
    event_avl_t left_child = (*node)->left_child;

    EVENT_ASSERT(node != NULL);

    (*node)->left_child = left_child->right_child;
    left_child->right_child = (*node);

    (*node)->height = max(event_avl_get_height((*node)->left_child), event_avl_get_height((*node)->right_child)) +
                                                                                                                       1;
    left_child->height =
                    max(event_avl_get_height(left_child->left_child),
                        event_avl_get_height(left_child->right_child)) + 1;

    (*node) = left_child;
}

static void event_avl_init(event_avl_t node, uint32_t value)
{
    EVENT_ASSERT(node != NULL);

    node->height = 0;
    node->value = value;
    node->left_child = NULL;
    node->right_child = NULL;
}

static void event_avl_insert(event_avl_t *tree, event_avl_t node)
{
    int8_t balance = 0;

    if ((*tree) == NULL)
    {
        (*tree) = node;
    }

    if (node->value < (*tree)->value)
    {
        event_avl_insert(&(*tree)->left_child, node);
    } else if (node->value > (*tree)->value)
    {
        event_avl_insert(&(*tree)->right_child, node);
    } else
    {
        return;
    }

    (*tree)->height = max(event_avl_get_height((*tree)->left_child), event_avl_get_height((*tree)->right_child)) +
                                                                                                                       1;

    balance = event_avl_get_balance((*tree));
    if (balance > 1 && node->value < (*tree)->left_child->value)
    {
        event_avl_right_rotate(&(*tree));
        return;
    }

    if (balance < -1 && node->value > (*tree)->right_child->value)
    {
        event_avl_left_rotate(&(*tree));
        return;
    }

    if (balance > 1 && node->value > (*tree)->left_child->value)
    {
        event_avl_left_rotate(&(*tree)->left_child);
        event_avl_right_rotate(&(*tree));
        return;
    }

    if (balance < -1 && node->value < (*tree)->right_child->value)
    {
        event_avl_right_rotate(&(*tree)->right_child);
        event_avl_left_rotate(&(*tree));
        return;
    }
}

static void event_avl_remove(event_avl_t *tree, event_avl_t node)
{

}

static event_avl_t event_avl_find(event_avl_t tree, uint32_t value)
{
    if (tree == NULL)
    {
        return tree;
    }

    if (tree->value == value)
    {
        return tree;
    }

    if (value < tree->value)
    {
        return event_avl_find(tree->left_child, value);
    } else if (value > tree->value)
    {
        return event_avl_find(tree->right_child, value);
    }

    return NULL;
}

static size_t event_avl_get_length(event_avl_t tree)
{
    size_t length = 1;

    if (tree == NULL)
    {
        return 0;
    }

    if (tree->left_child != NULL)
    {
        length += event_avl_get_length(tree->left_child);
    }

    if (tree->right_child != NULL)
    {
        length += event_avl_get_length(tree->right_child);
    }

    return length;
}

/**
 * @brief This function init a event server.
 *
 * @param server The event server to be inited.
 * @param queue_length The length of the client queue.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int event_server_init(event_server_t server, size_t queue_length)
{
    uint8_t *pool = NULL;

    EVENT_ASSERT(server != NULL);
    EVENT_ASSERT(name != NULL);
    EVENT_ASSERT(queue_length > 0 && queue_length <= 256);

    /* Check if the queue is busy */
    if (server->queue.size != 0)
    {
        return -EVENT_ERR_BUSY;
    }

    /* Allocate the queue memory */
    pool = malloc(queue_length);
    if (pool == NULL)
    {
        return -EVENT_ERR_NO_MEMORY;
    }

    /* Initialize the private fields */
    event_fifo_init(&server->queue, pool, queue_length);
    server->list = NULL;

    return EVENT_ERR_OK;
}

/**
 * @brief This function uninit a event server.
 *
 * @param server The event server to be uninited.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int event_server_uninit(event_server_t server)
{
    EVENT_ASSERT(server != NULL);
    EVENT_ASSERT(server->list != NULL);

    /* Free the queue memory */
    free(server->queue.buffer);

    /* Reset the private fields */
    server->list = NULL;
    event_fifo_init(&server->queue, NULL, 0);

    return EVENT_ERR_OK;
}

/**
 * @brief This function notify the event server to wake up a client.
 *
 * @param server The event server to be notified.
 *
 * @param id The id of the client to be wake up.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int event_server_notify(event_server_t server, uint8_t id)
{
    EVENT_ASSERT(server != NULL);

    /* Write the event id to the queue */
    if (!event_fifo_write(&server->queue, &id, sizeof(id)))
    {
        return -EVENT_ERR_NO_MEMORY;
    }

    return EVENT_ERR_OK;
}

/**
 * @brief This function handle the event server.
 *
 * @param server The event server to be handled.
 */
void event_server_handle(event_server_t server)
{
    uint8_t id = 0;
    event_avl_t node = NULL;
    event_client_t client = NULL;

    EVENT_ASSERT(server != NULL);

    /* Read the event id from the queue */
    while (event_fifo_read(&server->queue, &id, sizeof(id)))
    {
        node = event_avl_find(server->list, id);
        if (node == NULL)
        {
            continue;
        }

        /* Get the client from the list */
        client = container_of(node, struct event_client, list);

        /* Call the client callback */
        client->cb(server, client->args);
    }
}

/**
 * @brief This function find the event client.
 *
 * @param id The id of the event client.
 * @param server The event server to which the event client belongs.
 *
 * @return A handle to the found event client, or NULL if not found.
 */
event_client_t event_client_find(uint8_t id, event_server_t server)
{
    EVENT_ASSERT(server != NULL);

    /* Find the event client from the server */
    return (event_client_t)event_avl_find(server->list, id);
}

static int _err_io_event_client_cb(event_server_t server, void *args)
{
    return -EVENT_ERR_IO;
}

/**
 * @brief This function creates a new event client.
 *
 * @param id The id of the event client.
 * @param cb The event client callback function.
 * @param args The arguments of the callback function.
 * @param server The event server to which the event client belong.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int event_client_create(uint8_t id,
                        int (*cb)(event_server_t server, void *args),
                        void *args,
                        event_server_t server)
{
    event_client_t client = NULL;

    EVENT_ASSERT(server != NULL);

    /* Check if the client is already exists in the server */
    if (event_avl_find(server->list, id) != NULL)
    {
        return -EVENT_ERR_GENERIC;
    }

    /* Allocate the client object */
    client = (event_client_t)malloc(sizeof(struct event_client));
    if (client == NULL)
    {
        return -EVENT_ERR_NO_MEMORY;
    }
    memset(client, 0, sizeof(struct event_client));

    /* Initialize the private fields */
    event_avl_init(&client->list, id);
    client->cb = cb ? cb : _err_io_event_client_cb;
    client->args = args;

    /* Insert the client into the manager's list */
    event_avl_insert(&server->list, &client->list);

    return EVENT_ERR_OK;
}

/**
 * @brief This function delete an event client.
 *
 * @param id The id of the event client.
 * @param server The event server to which the event client belongs.
 *
 * @return EVENT_ERR_OK on success, otherwise an error code.
 */
int client_delete(uint8_t id, event_server_t server)
{
    event_client_t client = NULL;

    EVENT_ASSERT(server != NULL);

    /* Find the event client from the server */
    client = event_client_find(id, server);
    if (client == NULL)
    {
        return -EVENT_ERR_NOT_FOUND;
    }

    /* Remove the client from the manager's list */
    event_avl_remove(&server->list, &client->list);

    /* Free the client */
    free(client);

    return EVENT_ERR_OK;
}