/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-08     MacRsh       first version
 */

#ifndef _AT_COMMAND_H_
#define _AT_COMMAND_H_

#include "mrlib.h"

#define MR_CONF_AT_COMMAND_BUFSZ        32

enum mr_at_command_state
{
    MR_AT_COMMAND_STATE_NONE = 0,                                   /* No command */
    MR_AT_COMMAND_STATE_START,                                      /* Start state */
    MR_AT_COMMAND_STATE_FLAG,                                       /* Flag state */
    MR_AT_COMMAND_STATE_CMD,                                        /* Command state */
    MR_AT_COMMAND_STATE_DATA,                                       /* Data state */
    MR_AT_COMMAND_STATE_END,                                        /* End state */
};

enum mr_at_command_server_type
{
    MR_AT_COMMAND_SERVER_TYPE_NONE,                                 /* No command server */
    MR_AT_COMMAND_SERVER_TYPE_HOST,                                 /* Host command server */
    MR_AT_COMMAND_SERVER_TYPE_SLAVE,                                /* Slave command server */
};

struct mr_at_command_server
{
    struct mr_object object;                                        /* At-command server object */

    mr_uint8_t type;                                                /* At-command server type */
    mr_avl_t list;                                                  /* At-command server list */
    void *buffer;                                                   /* At-command server buffer */
    size_t queue_size;                                              /* At-command server queue size */
};
typedef struct mr_at_command_server *mr_at_command_server_t;        /* Type for at-command server */

typedef struct mr_at_command *mr_at_command_t;                      /* Type for at-command */

struct mr_at_command
{
    struct mr_avl list;                                             /* At-command list */
    mr_at_command_server_t server;                                  /* At-command owner server */
    const char *cmd;                                                /* At-command command */

    mr_err_t (*cb)(mr_at_command_t at_command, void *args);         /* At-command callback */
};

mr_at_command_server_t mr_at_command_server_find(const char *name);
mr_err_t mr_at_command_server_add(mr_at_command_server_t server,
                                  const char *name,
                                  mr_uint8_t type,
                                  mr_size_t queue_length);
mr_err_t mr_at_command_server_remove(mr_at_command_server_t server);
void mr_at_command_server_handle(mr_at_command_server_t server);
void mr_at_command_server_parse(mr_at_command_server_t server, mr_uint8_t data);
mr_err_t mr_at_command_create(const char *command,
                              mr_err_t (*cb)(mr_at_command_t at_command, void *args),
                              mr_at_command_server_t server);
mr_err_t mr_at_command_delete(const char *command, mr_at_command_server_t server);
mr_err_t mr_at_command_edit(const char *command, const char *new_command, mr_at_command_server_t server);
const char *mr_at_command_get_cmd(mr_at_command_t at_command);
#define mr_at_command_get_args  mr_sscanf
#define mr_at_command_set_response  mr_snprintf
#define mr_at_command_set_response_ok(buffer, bufsz)  mr_snprintf(buffer, bufsz, "OK\r\n")
#define mr_at_command_set_response_error(buffer, bufsz)  mr_snprintf(buffer, bufsz, "ERROR\r\n")

#endif  /* _AT_COMMAND_H_ */