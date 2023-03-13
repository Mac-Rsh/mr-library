/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-10     MacRsh       first version
 */

#include "link.h"

static mr_uint32_t mr_link_get_crc(const mr_uint8_t *data, mr_size_t count)
{
    mr_uint32_t crc = 0;

    while (count--)
    {
        crc += *data;
        data++;
    }

    return crc;
}

static mr_err_t mr_link_callback(mr_device_t device, void *args)
{
    static struct mr_link_packet_head packet_head = {MR_NULL};
    static enum mr_link_state state = MR_LINK_CHECK_HEAD;
    mr_uint8_t buffer[MR_LINK_PACKET_MAX] = {0};

    switch (state)
    {
        case MR_LINK_CHECK_HEAD:
        {
            if (*(mr_size_t *) args >= sizeof(packet_head))
            {
                /* Check frame-head */
                mr_device_read(device, -1, &packet_head, 1);
                if (packet_head.frame_head != 0x3A)
                    return -MR_ERR_BUSY;

                mr_device_read(device, -1, (((mr_uint8_t *) &packet_head) + 1), sizeof(packet_head) - 1);
                state = MR_LINK_CHECK_DATA;
                return MR_ERR_OK;
            } else
                return -MR_ERR_BUSY;
        }

        case MR_LINK_CHECK_DATA:
        {
            if (*(mr_size_t *) args >= packet_head.length)
            {
                mr_device_read(device, -1, buffer, packet_head.length);

                if (packet_head.crc == mr_link_get_crc(buffer, packet_head.length))
                {
                    for (int i = 0; i < packet_head.length; ++i)
                    {
                        printf("%x\r\n", buffer[i]);
                    }
                }
            } else
                return -MR_ERR_BUSY;
        }

        default:return -MR_ERR_BUSY;
    }
}

static mr_device_t mr_link_open_port(const char *port_name)
{
    mr_err_t error_code = MR_ERR_OK;
    mr_device_t port_device = MR_NULL;

    port_device = mr_device_find(port_name);
    if (port_device == MR_NULL)
        return MR_NULL;

    error_code = mr_device_open(port_device, MR_OPEN_RDWR);
    if (error_code != MR_ERR_OK)
        return MR_NULL;

    mr_device_ioctl(port_device, MR_CMD_SET_RX_CALLBACK, mr_link_callback);

    return port_device;
}

static void mr_link_packet_head_init(struct mr_link_packet_head *packet_head, mr_uint8_t src_addr, mr_uint8_t dst_addr)
{
    packet_head->frame_head = 0x3A;
    packet_head->crc = 0;
    packet_head->ack = 0;
    packet_head->sequence = 0;
    packet_head->sequence = 0;
    packet_head->src_addr = src_addr;
    packet_head->dst_addr = dst_addr;
    packet_head->length = 0;
}

mr_size_t mr_link_pack_head(struct mr_link_packet_head *head,
                            mr_uint8_t *buffer,
                            mr_size_t count)
{
    if (count > MR_LINK_PACKET_MAX)
        count = MR_LINK_PACKET_MAX;

    head->sequence++;
    head->length = count;
    head->crc = mr_link_get_crc(buffer, head->length);

    return count;
}

mr_size_t mr_link_write(mr_link_t link,
                        mr_uint8_t src_addr,
                        mr_uint8_t dst_addr,
                        mr_uint8_t *buffer,
                        mr_size_t count)
{
    mr_device_t port_device = (mr_device_t) link->device.data;
    struct mr_link_packet_head packet_head = {MR_NULL};
    mr_size_t send_count = 0;

    mr_link_packet_head_init(&packet_head, src_addr, dst_addr);

    while (send_count < count)
    {
        mr_link_pack_head(&packet_head, &buffer[send_count], (count - send_count));

        mr_device_write(port_device, -1, (mr_uint8_t *) &packet_head, sizeof(struct mr_link_packet_head));
        mr_device_write(port_device, -1, &buffer[send_count], packet_head.length);

        send_count += packet_head.length;
    }

    return count;
}

mr_err_t mr_link_service_add_to_container(mr_link_service_t link_service, const char *name)
{
    mr_err_t error_code = MR_ERR_OK;
    const static struct mr_device_ops device_ops = {MR_NULL};

    /* Initialize the device and add the device to the container */
    mr_device_init(&link_service->device, name);
    error_code = mr_device_add_to_container(&link_service->device,
                                            MR_DEVICE_TYPE_LINK_SERVICE,
                                            MR_OPEN_RDWR,
                                            &device_ops,
                                            MR_NULL);
    if (error_code != MR_ERR_OK)
        return error_code;

    /* Initialize the test_link-service fields */


    return MR_ERR_OK;
}

mr_err_t mr_link_add_to_container(mr_link_t link, const char *name, char *port_name)
{
    mr_err_t error_code = MR_ERR_OK;
    const static struct mr_device_ops device_ops = {MR_NULL};
    mr_device_t port_device = MR_NULL;

    /* Open service port */
    port_device = mr_link_open_port(port_name);
    if (port_device == MR_NULL)
        return -MR_ERR_NOT_FOUND;

    /* Initialize the device and add the device to the container */
    mr_device_init(&link->device, name);
    error_code = mr_device_add_to_container(&link->device, MR_DEVICE_TYPE_LINK, MR_OPEN_RDWR, &device_ops, port_device);
    if (error_code != MR_ERR_OK)
        return error_code;

    /* Initialize the link fields */
    mr_list_init(&link->list);

    return MR_ERR_OK;
}


