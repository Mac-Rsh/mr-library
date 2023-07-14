/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-06     MacRsh       first version
 */

#ifndef _C2G4A20S_H_
#define _C2G4A20S_H_

#include "stdint.h"
#include "string.h"
#include "stdio.h"

#define C2G4A20S_DELAY(x)
#define C2G4A20S_RETRY                  10
#define C2G4A20S_BUFSZ                  128
#define C2G4A20S_SLAVE_SIZE             10
#define C2G4A20S_ASSERT(x)

enum c2g4a20s_type
{
    C2G4A20S_TYPE_COORDINATOR,
    C2G4A20S_TYPE_ROUTER,
    C2G4A20S_TYPE_TERMINATOR,
    C2G4A20S_TYPE_NONE,
};

enum c2g4a20s_state
{
    C2G4A20S_STATE_INIT,
    C2G4A20S_STATE_CREATE,
    C2G4A20S_STATE_JOINING,
    C2G4A20S_STATE_JOINED,
};

struct c2g4a20s_address
{
    uint8_t short_mac[2];
    uint8_t mac[8];
};

struct c2g4a20s_parse
{
    uint16_t ack;
    uint16_t retry;
    uint16_t retry_count;
    uint16_t state;
    size_t length;
    uint8_t buffer[C2G4A20S_BUFSZ];
};

struct c2g4a20s_slave
{
    size_t index;
    struct c2g4a20s_address address[C2G4A20S_SLAVE_SIZE];
};

typedef struct c2g4a20s *c2g4a20s_t;

struct c2g4a20s_ops
{
    void (*write)(c2g4a20s_t c2g4a20s, uint8_t *data, size_t size);
};

struct c2g4a20s
{
    uint8_t type;
    uint8_t state;
    struct c2g4a20s_address address;
    struct c2g4a20s_parse parse;
    struct c2g4a20s_slave slave;
    void *data;

    const struct c2g4a20s_ops *ops;
};

#define C2G4A20S_PACKET_HEAD            0x55
#define C2G4A20S_PACKET_TAIL            0xAA

#define C2G4A20S_ACK_MIN_SIZE           7

#define C2G4A20S_ADDRESS_SHORT          0x02
#define C2G4A20S_ADDRESS_IEEE           0x03

#define C2G4A20S_PARSE_STATE_IDLE       0
#define C2G4A20S_PARSE_STATE_BUSY       1
#define C2G4A20S_PARSE_STATE_OK         2

#define C2G4A20S_CMD_RESET              0x0005
#define C2G4A20S_CMD_REBOOT             0x000b
#define C2G4A20S_CMD_READ_ADDRESS       0x01c0
#define C2G4A20S_CMD_READ_TYPE          0x000e
#define C2G4A20S_CMD_WRITE_DATA         0x0044
#define C2G4A20S_CMD_CREATE_NETWORK     0x0001
#define C2G4A20S_CMD_PERMIT_JOIN        0x0034

#define C2G4A20S_ACK_NONE               0x0000
#define C2G4A20S_ACK_OK                 0x8000
#define C2G4A20S_ACK_READ_ADDRESS       0x81c0
#define C2G4A20S_ACK_READ_TYPE          0x800e
#define C2G4A20S_ACK_READ_DATA          0xff01
#define C2G4A20S_ACK_JOIN_NETWORK       0x8043

void c2g4a20s_init(c2g4a20s_t c2g4a20s, const struct c2g4a20s_ops *ops, void *data);
void c2g4a20s_write_slave(c2g4a20s_t c2g4a20s, size_t index, void *buffer, size_t size);
void c2g4a20s_isr(c2g4a20s_t c2g4a20s, uint8_t data);
void c2g4a20s_handle(c2g4a20s_t c2g4a20s);
void c2g4a20s_add_slave(c2g4a20s_t c2g4a20s, size_t index);
void c2g4a20s_write_slave(c2g4a20s_t c2g4a20s, size_t index, void *buffer, size_t size);

#endif /* _C2G4A20S_H_ */