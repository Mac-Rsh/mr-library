/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-10    MacRsh       first version
 */

#ifndef _LINK_H_
#define _LINK_H_

#include <mrlib.h>

#define MR_LINK_PACKET_MAX 4

enum mr_link_state
{
    MR_LINK_CHECK_HEAD,
    MR_LINK_CHECK_DATA,
    MR_LINK_OK,
};

struct mr_link_packet_head
{
    mr_uint8_t frame_head: 7;
    mr_uint8_t ack: 1;
    mr_uint8_t src_addr;
    mr_uint8_t dst_addr;
    mr_uint8_t sequence;
    mr_uint16_t length;
    mr_uint32_t crc;
};

typedef struct mr_link *mr_link_t;

struct mr_link_service
{
    struct mr_device device;

    struct mr_list list;
    mr_uint8_t src_addr;
};
typedef struct mr_link_service *mr_link_service_t;

struct mr_link
{
    struct mr_device device;

    struct mr_list list;
    mr_uint8_t buffer[MR_LINK_PACKET_MAX];
};

mr_size_t mr_link_write(mr_link_t link,
                        mr_uint8_t src_addr,
                        mr_uint8_t dst_addr,
                        mr_uint8_t *buffer,
                        mr_size_t count);

mr_err_t mr_link_service_add_to_container(mr_link_service_t link_service, const char *name);
mr_err_t mr_link_add_to_container(mr_link_t link, const char *name, char *port_name);

#endif
