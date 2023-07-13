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

static void mr_at_command_server_parse_host(mr_at_command_server_t server, mr_uint8_t data)
{
    mr_size_t count = 0;
    mr_uint8_t *state = 0;
    mr_uint8_t *buffer = 0;

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
            if (data == '+')
            {
                *state = MR_AT_COMMAND_STATE_CMD;
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
            if (data == ':')
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

static void mr_at_command_server_parse_slave(mr_at_command_server_t server, mr_uint8_t data)
{
    mr_size_t count = 0;
    mr_uint8_t *state = 0;
    mr_uint8_t *buffer = 0;

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

/**
 * @brief This function finds a at-command server object.
 *
 * @param name The name of the at-command server.
 *
 * @return A handle to the found at-command server, or MR_NULL if not found.
 */
mr_at_command_server_t mr_at_command_server_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the at-command server object from the server container */
    return (mr_at_command_server_t)mr_object_find(name, MR_OBJECT_TYPE_SERVER);
}

/**
 * @brief This function adds a at-command server to the container.
 *
 * @param server The at-command server to be added.
 * @param name The name of the at-command server.
 * @param type The type of the at-command server.
 * @param queue_length The length of the queue.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_at_command_server_add(mr_at_command_server_t server,
                                  const char *name,
                                  mr_uint8_t type,
                                  mr_size_t queue_length)
{
    mr_err_t ret = MR_ERR_OK;
    mr_uint8_t *pool = MR_NULL;

    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(type == MR_AT_COMMAND_SERVER_TYPE_HOST || type == MR_AT_COMMAND_SERVER_TYPE_SLAVE);
    MR_ASSERT(queue_length > 0);

    /* Allocate the queue memory */
    pool = mr_malloc(queue_length * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE));
    if (pool == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(pool, 0, queue_length * (MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE));

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
    server->buffer = pool;
    server->queue_size = queue_length;
    server->type = type;

    return MR_ERR_OK;
}

/**
 * @brief This function remove a at-command server object from the container.
 *
 * @param server The at-command server to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_at_command_server_remove(mr_at_command_server_t server)
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
    server->type = MR_AT_COMMAND_SERVER_TYPE_NONE;

    return MR_ERR_OK;
}

/**
 * @brief This function handle the at-command server.
 *
 * @param server The at-command server to be handled.
 */
void mr_at_command_server_handle(mr_at_command_server_t server)
{
    mr_size_t count = 0;
    mr_uint8_t *state = 0;
    mr_uint8_t *buffer = 0;
    mr_uint32_t cmd = 0;
    mr_avl_t node = MR_NULL;
    mr_at_command_t at_command = MR_NULL;

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

            /* Get the at-command from the list */
            at_command = mr_container_of(node, struct mr_at_command, list);

            /* Call the at-command callback */
            at_command->cb(at_command, state + MR_AT_COMMAND_CMD_BLOCK_SIZE);

            /* Erase the block */
            mr_memset(state, 0, MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);
        }
        count++;
    }
}

/**
 * @brief This function parse the at-command server.
 *
 * @param server The at-command server to be handled.
 * @param data The data to be parsed.
 *
 * @note It is recommended to place this function in the receive interrupt.
 */
void mr_at_command_server_parse(mr_at_command_server_t server, mr_uint8_t data)
{
    MR_ASSERT(server != MR_NULL);

    switch (server->type)
    {
        case MR_AT_COMMAND_SERVER_TYPE_HOST:
        {
            mr_at_command_server_parse_host(server, data);
            break;
        }

        case MR_AT_COMMAND_SERVER_TYPE_SLAVE:
        {
            mr_at_command_server_parse_slave(server, data);
            break;
        }

        default:
            break;
    }
}

/**
 * @brief This function creates a new at-command.
 *
 * @param command The command to be created.
 * @param cb The at-command callback function.
 * @param server The at-command server to which the at-command belong.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_at_command_create(const char *command,
                              mr_err_t (*cb)(mr_at_command_t at_command, void *args),
                              mr_at_command_server_t server)
{
    mr_uint32_t cmd = 0;
    mr_uint8_t index = 0;
    mr_at_command_t at_command = MR_NULL;

    MR_ASSERT(command != MR_NULL);
    MR_ASSERT(cb != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->type != MR_AT_COMMAND_SERVER_TYPE_NONE);
    MR_ASSERT((server->type == MR_AT_COMMAND_SERVER_TYPE_HOST && mr_strncmp("+", command, mr_strlen("+")) == 0 ||
               (server->type == MR_AT_COMMAND_SERVER_TYPE_SLAVE && mr_strncmp("AT+", command, mr_strlen("AT+")) == 0)));
    MR_ASSERT(mr_strlen(command) < MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);

    switch (server->type)
    {
        case MR_AT_COMMAND_SERVER_TYPE_HOST:
        {
            index = 1;
            break;
        }

        case MR_AT_COMMAND_SERVER_TYPE_SLAVE:
        {
            index = 3;
            break;
        }

        default:
            break;
    }

    cmd = mr_str2hash(command + index, mr_strlen(command) - index);
    /* Check if the command is already exists in the server */
    if (mr_avl_find(server->list, cmd) != MR_NULL)
    {
        return -MR_ERR_GENERIC;
    }

    /* Allocate the at_command object */
    at_command = (mr_at_command_t)mr_malloc(sizeof(struct mr_at_command));
    if (at_command == MR_NULL)
    {
        return -MR_ERR_NO_MEMORY;
    }
    mr_memset(at_command, 0, sizeof(struct mr_at_command));

    /* Initialize the private fields */
    mr_avl_init(&at_command->list, cmd);
    at_command->server = server;
    at_command->cmd = command;
    at_command->cb = cb;

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the at_command into the server's list */
    mr_avl_insert(&server->list, &at_command->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    return MR_ERR_OK;
}

/**
 * @brief This function delete an at-command.
 *
 * @param command The command to be deleted.
 * @param server The at-command server to which the at-command belongs.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_at_command_delete(const char *command, mr_at_command_server_t server)
{
    mr_avl_t node = MR_NULL;
    mr_uint8_t index = 0;
    mr_at_command_t at_command = MR_NULL;

    MR_ASSERT(command != MR_NULL);
    MR_ASSERT(server != MR_NULL);
    MR_ASSERT(server->type != MR_AT_COMMAND_SERVER_TYPE_NONE);
    MR_ASSERT((server->type == MR_AT_COMMAND_SERVER_TYPE_HOST && mr_strncmp("+", command, mr_strlen("+")) == 0 ||
               (server->type == MR_AT_COMMAND_SERVER_TYPE_SLAVE && mr_strncmp("AT+", command, mr_strlen("AT+")) == 0)));
    MR_ASSERT(mr_strlen(command) < MR_CONF_AT_COMMAND_BUFSZ + MR_AT_COMMAND_CMD_BLOCK_SIZE);

    switch (server->type)
    {
        case MR_AT_COMMAND_SERVER_TYPE_HOST:
        {
            index = 1;
            break;
        }

        case MR_AT_COMMAND_SERVER_TYPE_SLAVE:
        {
            index = 3;
            break;
        }

        default:
            break;
    }

    /* Find the at-command from the server */
    node = mr_avl_find(server->list, mr_str2hash(command + index, mr_strlen(command) - index));
    if (node == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the at-command from the list */
    at_command = mr_container_of(node, struct mr_at_command, list);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the at_command from the server's list */
    mr_avl_remove(&server->list, &at_command->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Free the at_command */
    mr_free(at_command);

    return MR_ERR_OK;
}

/**
 * @brief This function edit an at-command.
 *
 * @param command The command to be edited.
 * @param new_command The new command.
 * @param server The at-command server to which the at-command belongs.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_at_command_edit(const char *command, const char *new_command, mr_at_command_server_t server)
{
    mr_avl_t node = MR_NULL;
    mr_at_command_t at_command = MR_NULL;
    mr_err_t (*cb)(mr_at_command_t at_command, void *args);

    MR_ASSERT(command != MR_NULL);
    MR_ASSERT(new_command != MR_NULL);
    MR_ASSERT(server != MR_NULL);

    /* Find the at-command from the server */
    node = mr_avl_find(server->list, mr_str2hash(command + 3, mr_strlen(command) - 3));
    if (node == MR_NULL)
    {
        return -MR_ERR_NOT_FOUND;
    }

    /* Get the at-command from the list */
    at_command = mr_container_of(node, struct mr_at_command, list);

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Remove the at_command from the server's list */
    mr_avl_remove(&server->list, &at_command->list);

    /* Enable interrupt */
    mr_interrupt_enable();

    /* Get the callback function */
    cb = at_command->cb;

    /* Free the at_command */
    mr_free(at_command);

    return mr_at_command_create(new_command, cb, server);
}

/**
 * @brief This function get the command from the at-command.
 *
 * @param at_command The at-command to be get.
 *
 * @return The command.
 */
const char *mr_at_command_get_cmd(mr_at_command_t at_command)
{
    MR_ASSERT(at_command != MR_NULL);

    return at_command->cmd;
}

#endif /* MR_CONF_AT_COMMAND */