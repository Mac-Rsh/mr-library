/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-08     MacRsh       first version
 */

#include "mrlib.h"

#if (MR_CONF_AT_COMMAND == MR_CONF_ENABLE)

#define MR_AT_COMMAND_CMD_BLOCK_SIZE    5

static void mr_at_command_closure(mr_uint8_t *buffer)
{
    mr_uint32_t cmd = 0;

    /* Save cmd to the cmd block buffer */
    cmd = mr_str2hash((const char *)buffer + 1, mr_strlen((char *)(buffer) + 1));
    mr_memcpy(buffer + 1, &cmd, sizeof(cmd));

    /* Erase the block buffer */
    mr_memset(buffer + MR_AT_COMMAND_CMD_BLOCK_SIZE, 0, MR_CONF_AT_COMMAND_BUFSZ);
}

mr_at_command_server_t mr_at_command_server_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the at-command server object from the server container */
    return (mr_at_command_server_t)mr_object_find(name, MR_OBJECT_TYPE_SERVER);
}

mr_err_t mr_at_command_server_add(mr_at_command_server_t server, const char *name, mr_size_t queue_length)
{
    mr_err_t ret = MR_ERR_OK;
    mr_uint8_t *pool = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(queue_length > 0);

    /* Allocate the queue memory */
    pool = mr_malloc(queue_length * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE));
    if (pool == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(pool, 0, queue_length * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE));
    server->buffer = pool;
    server->queue_size = queue_length;

    /* Add the object to the container */
    ret = mr_object_add(&server->object, name, MR_OBJECT_TYPE_SERVER);
    if (ret != MR_ERR_OK)
    {
        /* Free the queue memory */
        mr_free(pool);
        return ret;
    }

    /* Initialize the private fields */
    server->list = MR_NULL;

    return MR_ERR_OK;
}

mr_err_t mr_at_command_remove(mr_at_command_server_t server)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->list != MR_NULL);

    /* Remove the object from the container */
    ret = mr_object_remove(&server->object);
    if (ret != MR_ERR_OK)
    {
        return ret;
    }

    /* Free the queue memory */
    mr_free(server->buffer);

    /* Reset the private fields */
    server->list = MR_NULL;
    server->buffer = MR_NULL;
    server->queue_size = 0;

    return MR_ERR_OK;
}

void mr_at_command_server_handle(mr_at_command_server_t server)
{
    mr_size_t count = 0;
    mr_uint8_t *state = 0;
    mr_uint8_t *buffer = 0;
    mr_uint32_t cmd = 0;
    mr_avl_t node = MR_NULL;
    mr_at_command_client_t client = MR_NULL;

    MR_ASSERT(server != MR_NULL);

    buffer = (mr_uint8_t *)server->buffer;
    /* Find the ready buffer block */
    while (count < server->queue_size)
    {
        state = buffer + count * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);
        if (*state == MR_AT_COMMAND_STATE_END)
        {
            mr_memcpy(&cmd, state + 1, sizeof(cmd));
            node = mr_avl_find(server->list, cmd);
            if (node == MR_NULL)
            {
                count++;
                continue;
            }

            /* Get the client from the list */
            client = mr_container_of(node, struct mr_at_command_client, list);

            /* Call the client callback */
            client->cb(client, state + MR_AT_COMMAND_CMD_BLOCK_SIZE);

            /* Erase the block */
            mr_memset(state, 0, MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);
        }
        count++;
    }
}

void mr_at_command_server_isr(mr_at_command_server_t server, mr_uint8_t data)
{
    mr_size_t count = 0;
    mr_uint8_t *state = 0;
    mr_uint8_t *buffer = 0;

    MR_ASSERT(server != MR_NULL);

    /* Find the idle buffer block */
    buffer = (mr_uint8_t *)server->buffer;
    for (count = 0; count < server->queue_size; count++)
    {
        state = buffer + count * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);
        if (*state < MR_AT_COMMAND_STATE_END)
        {
            break;
        }
    }
    /* Not find the idle buffer block */
    if (count == server->queue_size)
    {
        return;
    }

    switch (*state)
    {
        case MR_AT_COMMAND_STATE_NONE:
        {
            if (data == 'A')
            {
                *state = MR_AT_COMMAND_STATE_START;
            }
            break;
        }

        case MR_AT_COMMAND_STATE_START:
        {
            if (data == 'T')
            {
                *state = MR_AT_COMMAND_STATE_FLAG;
            } else
            {
                *state = MR_AT_COMMAND_STATE_NONE;
            }
            break;
        }

        case MR_AT_COMMAND_STATE_FLAG:
        {
            if (data == '+')
            {
                *state = MR_AT_COMMAND_STATE_CMD;
            } else
            {
                *state = MR_AT_COMMAND_STATE_NONE;
            }
            break;
        }

        case MR_AT_COMMAND_STATE_CMD:
        {
            /* Closure */
            if (data == '\r' || data == '\n' || data == ';' || data == '\0')
            {
                mr_at_command_closure(state);
                *state = MR_AT_COMMAND_STATE_END;
                break;
            }

            /* Add the cmd to the buffer */
            for (count = 1; count < MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE - 1; count++)
            {
                if (*(state + count) == 0)
                {
                    *(state + count) = data;
                    break;
                }
            }

            /* Parameterized cmd */
            if (data == '=')
            {
                mr_at_command_closure(state);
                *state = MR_AT_COMMAND_STATE_DATA;
            }
            break;
        }

        case MR_AT_COMMAND_STATE_DATA:
        {
            /* End the data */
            if (data == '\r' || data == '\n' || data == ';' || data == '\0')
            {
                *state = MR_AT_COMMAND_STATE_END;
                break;
            }

            /* Add the data to the buffer */
            for (count = MR_AT_COMMAND_CMD_BLOCK_SIZE; count < MR_CONF_AT_COMMAND_BUFSZ - 1; count++)
            {
                if (*(state + count) == 0)
                {
                    *(state + count) = data;
                    break;
                }
            }
            break;
        }
    }
}

mr_at_command_client_t mr_at_command_client_find(const char *at_command, mr_at_command_server_t server)
{
    mr_uint32_t cmd = 0;

    MR_ASSERT(at_command != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(at_command[0] == 'A' && at_command[1] == 'T' && at_command[2] == '+');

    cmd = mr_str2hash(at_command + 3, mr_strlen(at_command) - 3);

    /* Find the at-command client from the server */
    return (mr_at_command_client_t)mr_avl_find(server->list, cmd);
}

mr_err_t mr_at_command_client_create(const char *at_command,
                                     mr_err_t (*cb)(mr_at_command_client_t client, void *args),
                                     mr_at_command_server_t server)
{
    mr_uint32_t cmd = 0;
    mr_at_command_client_t client = MR_NULL;

    MR_ASSERT(at_command != MR_NULL);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(at_command[0] == 'A' && at_command[1] == 'T' && at_command[2] == '+');
    MR_ASSERT(mr_strlen(at_command) - 3 < MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);

    cmd = mr_str2hash(at_command + 3, mr_strlen(at_command) - 3);
    /* Check if the client is already exists in the server */
    if (mr_avl_find(server->list, cmd) != MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Allocate the client object */
    client = (mr_at_command_client_t)mr_malloc(sizeof(struct mr_at_command_client));
    if (client == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(client, 0, sizeof(struct mr_at_command_client));

    /* Initialize the private fields */
    mr_avl_init(&client->list, cmd);
    client->server = server;
    client->cmd = at_command + 3;
    client->cb = cb;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the client into the server's list */
    mr_avl_insert(&server->list, &client->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

mr_err_t mr_at_command_client_delete(const char *at_command, mr_at_command_server_t server)
{
    mr_at_command_client_t client = MR_NULL;

    MR_ASSERT(at_command != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(at_command[0] == 'A' && at_command[1] == 'T' && at_command[2] == '+');

    /* Find the at-command client from the server */
    client = mr_at_command_client_find(at_command, server);
    if (client == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the client from the server's list */
    mr_avl_remove(&server->list, &client->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the client */
    mr_free(client);

    return MR_ERR_OK;
}

#endif