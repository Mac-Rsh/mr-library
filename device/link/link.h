/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#ifndef _LINK_H_
#define _LINK_H_

#include <mrlib.h>

#define MR_LINK_PACKET_HEAD 0x55

struct mr_link_packet_head
{
	mr_uint8_t start;
	mr_uint8_t length;
	mr_uint16_t sequence;
	mr_uint8_t crc;
};

struct mr_link_packet
{
	struct mr_link_packet_head head;
	mr_uint8_t *data;

	struct mr_link_packet *next;
};
typedef struct mr_link_packet *mr_link_packet_t;

struct mr_link
{
	struct mr_object object;

	struct mr_fifo rx_fifo;

	mr_uint16_t tx_sequence;
	mr_link_packet_t tx_packet;

	mr_uint32_t (*get_crc)(mr_uint8_t *buffer, mr_size_t length);
};
typedef struct mr_link *mr_link_t;

#endif
