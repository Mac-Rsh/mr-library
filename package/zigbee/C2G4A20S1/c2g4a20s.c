/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-06     MacRsh       first version
 */

#include "c2g4a20s.h"

static void c2g4a20s_push_data(c2g4a20s_t c2g4a20s, uint8_t data)
{
    if (c2g4a20s->parse.length < C2G4A20S_BUFSZ)
    {
        c2g4a20s->parse.buffer[c2g4a20s->parse.length++] = data;
    } else
    {
        c2g4a20s->parse.length = 0;
        c2g4a20s->state = C2G4A20S_PARSE_STATE_IDLE;
    }
}

static void c2g4a20s_clear_parse_buffer(c2g4a20s_t c2g4a20s)
{
    c2g4a20s->parse.length = 0;
}

static int c2g4a20s_check_address(c2g4a20s_t c2g4a20s)
{
    size_t i = 0;

    for (i = 0; i < sizeof(c2g4a20s->address.mac); i++)
    {
        if (c2g4a20s->address.mac[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}

static int c2g4a20s_check_ack(c2g4a20s_t c2g4a20s)
{
    uint16_t ack = 0;

    ack = c2g4a20s->parse.buffer[1] << 8 | c2g4a20s->parse.buffer[2];
    if (ack == c2g4a20s->parse.ack)
    {
        return 1;
    }

    return 0;
}

static void c2g4a20s_write_cmd(c2g4a20s_t c2g4a20s, uint16_t cmd, uint16_t ack)
{
    uint8_t buffer[10] = {0};
    size_t length = 0;

    buffer[length++] = C2G4A20S_PACKET_HEAD;

    /* Command */
    buffer[length++] = cmd >> 8;
    buffer[length++] = cmd;

    /* Length */
    buffer[length++] = 0;
    buffer[length++] = 0;

    /* Crc */
    buffer[length++] = 0;

    /* Tail */
    buffer[length++] = C2G4A20S_PACKET_TAIL;

    c2g4a20s->ops->write(c2g4a20s, buffer, length);
    c2g4a20s->parse.retry = cmd;
    c2g4a20s->parse.ack = ack;
}

static void c2g4a20s_write_join(c2g4a20s_t c2g4a20s, uint8_t time)
{
    uint8_t buffer[10] = {0};
    size_t length = 0;

    buffer[length++] = C2G4A20S_PACKET_HEAD;

    /* Command */
    buffer[length++] = C2G4A20S_CMD_PERMIT_JOIN >> 8;
    buffer[length++] = C2G4A20S_CMD_PERMIT_JOIN;

    /* Length */
    buffer[length++] = 0;
    buffer[length++] = 4;

    /* Crc */
    buffer[length++] = 0;

    buffer[length++] = 0xff;
    buffer[length++] = 0xfc;
    buffer[length++] = time;
    buffer[length++] = 0x01;

    /* Tail */
    buffer[length++] = C2G4A20S_PACKET_TAIL;

    c2g4a20s->ops->write(c2g4a20s, buffer, length);

    if (time != 0)
    {
        c2g4a20s->parse.ack = C2G4A20S_ACK_JOIN_NETWORK;
    }
}

static void c2g4a20s_write_data(c2g4a20s_t c2g4a20s, uint16_t address, uint8_t *data, size_t size)
{
    uint8_t buffer[100] = {0};
    size_t length = 0;

    /* Header */
    buffer[length++] = C2G4A20S_PACKET_HEAD;

    /* Command */
    buffer[length++] = C2G4A20S_CMD_WRITE_DATA >> 8;
    buffer[length++] = C2G4A20S_CMD_WRITE_DATA;

    /* Length */
    buffer[length++] = (size + 6) >> 8;
    buffer[length++] = (size + 6);

    /* Crc */
    buffer[length++] = 0;

    /* Address */
    buffer[length++] = C2G4A20S_ADDRESS_SHORT;
    memcpy(&buffer[length], &address, sizeof(address));
    printf("%x %x\r\n", buffer[length], buffer[length + 1]);
    length += sizeof(address);
    buffer[length++] = 0x01;
    buffer[length++] = 0x01;

    /* Data */
    buffer[length++] = size;
    memcpy(&buffer[length], data, size);
    length += size;

    /* Tail */
    buffer[length++] = C2G4A20S_PACKET_TAIL;

    c2g4a20s->ops->write(c2g4a20s, buffer, length);

    c2g4a20s->parse.ack = C2G4A20S_ACK_NONE;
}

static void c2g4a20s_update(c2g4a20s_t c2g4a20s)
{
    /* Is idle */
    if (c2g4a20s->parse.ack == C2G4A20S_ACK_NONE)
    {
        /* Read type */
        if (c2g4a20s->type == C2G4A20S_TYPE_NONE)
        {
            c2g4a20s_write_cmd(c2g4a20s, C2G4A20S_CMD_READ_TYPE, C2G4A20S_ACK_READ_TYPE);
            return;
        }

        /* Read address */
        if (!c2g4a20s_check_address(c2g4a20s))
        {
            c2g4a20s_write_cmd(c2g4a20s, C2G4A20S_CMD_READ_ADDRESS, C2G4A20S_ACK_READ_ADDRESS);
            return;
        }

        /* Create network, if it is coordinator */
        if (c2g4a20s->state == C2G4A20S_STATE_INIT && c2g4a20s->type == C2G4A20S_TYPE_COORDINATOR)
        {
            c2g4a20s_write_cmd(c2g4a20s, C2G4A20S_CMD_CREATE_NETWORK, C2G4A20S_ACK_NONE);
            c2g4a20s->state = C2G4A20S_STATE_CREATE;
            return;
        }

        /* Close permit join */
        if (c2g4a20s->state == C2G4A20S_STATE_JOINED)
        {
            c2g4a20s_write_join(c2g4a20s, 0x00);
            c2g4a20s->state = C2G4A20S_STATE_CREATE;
            return;
        }
    }
}

void c2g4a20s_init(c2g4a20s_t c2g4a20s, const struct c2g4a20s_ops *ops, void *data)
{
    C2G4A20S_ASSERT(c2g4a20s != NULL);

    memset(c2g4a20s, 0, sizeof(struct c2g4a20s));
    c2g4a20s->ops = ops;
    c2g4a20s->data = data;
    c2g4a20s->type = C2G4A20S_TYPE_NONE;

    c2g4a20s_write_cmd(c2g4a20s, C2G4A20S_CMD_REBOOT, C2G4A20S_ACK_NONE);
}

static void c2g4a20s_parse(c2g4a20s_t c2g4a20s)
{
    switch (c2g4a20s->parse.ack)
    {
        case C2G4A20S_ACK_READ_ADDRESS:
        {
            memcpy(&c2g4a20s->address, c2g4a20s->parse.buffer + 6, sizeof(c2g4a20s->address));
            break;
        }

        case C2G4A20S_ACK_READ_TYPE:
        {
            memcpy(&c2g4a20s->type, c2g4a20s->parse.buffer + 7, sizeof(c2g4a20s->type));
            break;
        }

        case C2G4A20S_ACK_JOIN_NETWORK:
        {
            if (c2g4a20s->state == C2G4A20S_STATE_JOINING)
            {
                memcpy(&c2g4a20s->slave.address[c2g4a20s->slave.index],
                       c2g4a20s->parse.buffer + 6,
                       sizeof(struct c2g4a20s_address));
                c2g4a20s->state = C2G4A20S_STATE_JOINED;
            }

            break;
        }

        default:
            break;
    }

    c2g4a20s->parse.ack = C2G4A20S_ACK_NONE;
}

void c2g4a20s_isr(c2g4a20s_t c2g4a20s, uint8_t data)
{
    C2G4A20S_ASSERT(c2g4a20s != NULL);

    switch (c2g4a20s->parse.state)
    {
        case C2G4A20S_PARSE_STATE_IDLE:
        {
            if (data == C2G4A20S_PACKET_HEAD)
            {
                c2g4a20s_push_data(c2g4a20s, data);
                c2g4a20s->parse.state = C2G4A20S_PARSE_STATE_BUSY;
            }
            break;
        }

        case C2G4A20S_PARSE_STATE_BUSY:
        {
            c2g4a20s_push_data(c2g4a20s, data);
            if (data == C2G4A20S_PACKET_TAIL)
            {
                if (c2g4a20s_check_ack(c2g4a20s))
                {
                    c2g4a20s_parse(c2g4a20s);
                }

                c2g4a20s_clear_parse_buffer(c2g4a20s);
                c2g4a20s->parse.state = C2G4A20S_PARSE_STATE_IDLE;
            }
            break;
        }

        default:
            break;
    }
}

void c2g4a20s_handle(c2g4a20s_t c2g4a20s)
{
    C2G4A20S_ASSERT(c2g4a20s != NULL);

    /* Update state */
    c2g4a20s_update(c2g4a20s);
}

void c2g4a20s_add_slave(c2g4a20s_t c2g4a20s, size_t index)
{
    c2g4a20s->slave.index = index;

    c2g4a20s_write_join(c2g4a20s, 0xff);
    c2g4a20s->state = C2G4A20S_STATE_JOINING;
}

void c2g4a20s_write_slave(c2g4a20s_t c2g4a20s, size_t index, void *buffer, size_t size)
{
    uint16_t address = 0;

    C2G4A20S_ASSERT(c2g4a20s != NULL);

    address = c2g4a20s->slave.address[index].short_mac[1] << 8 | c2g4a20s->slave.address[index].short_mac[0];

    c2g4a20s_write_data(c2g4a20s, address, buffer, size);
}
