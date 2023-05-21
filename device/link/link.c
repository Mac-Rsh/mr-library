/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include <device/link/link.h>

#undef LOG_TAG
#define LOG_TAG "link"

mr_link_t mr_link_find(const char *name)
{
	MR_ASSERT(name != MR_NULL);

	/* Find the link object from the misc-container */
	return (mr_link_t)mr_object_find(name, MR_CONTAINER_TYPE_MISC);
}

mr_err_t mr_link_add(mr_link_t link,
					 const char *name,
					 mr_size_t rx_bufsz,
					 mr_uint32_t (*get_crc)(mr_uint8_t *buffer, mr_size_t length))
{
	mr_err_t ret = MR_ERR_OK;
	mr_uint8_t *pool = MR_NULL;

	MR_ASSERT(link != MR_NULL);

	/* Add the object to the container */
	ret = mr_object_add(&link->object, name, MR_CONTAINER_TYPE_MISC);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	pool = (mr_uint8_t *)malloc(rx_bufsz);
	if (pool == NULL)
	{
		MR_LOG_D(LOG_TAG, "Failed to allocate memory for rx-pool\r\n");
		return - MR_ERR_NO_MEMORY;
	}
	mr_fifo_init(&link->rx_fifo, pool, rx_bufsz);
	link->tx_sequence = 0;
	link->tx_packet = MR_NULL;
	link->get_crc = get_crc;

	return MR_ERR_OK;
}

mr_err_t mr_link_remove(mr_link_t link)
{
	mr_err_t ret = MR_ERR_OK;

	MR_ASSERT(link != MR_NULL);

}

mr_uint16_t mr_link_get_crc(mr_uint8_t *buffer, mr_uint16_t length)
{
	mr_uint16_t crc = 0;

	while (length --)
	{
		crc += *buffer ++;
	}

	return crc;
}

mr_err_t mr_link_packet_create(mr_uint8_t *buffer, mr_uint16_t length, mr_link_t link)
{
	mr_link_packet_t packet = MR_NULL;

	MR_ASSERT(buffer != NULL);

	packet = (mr_link_packet_t)malloc(sizeof(struct mr_link_packet));
	if (packet == NULL)
	{
		MR_LOG_D(LOG_TAG, "Failed to allocate memory for packet\r\n");
		return - MR_ERR_NO_MEMORY;
	}

	packet->head.start = MR_LINK_PACKET_HEAD;
	packet->head.length = length;
	packet->head.sequence = link->tx_sequence;

	packet->data = (mr_uint8_t *)malloc(length);
	if (packet->data == NULL)
	{
		MR_LOG_D(LOG_TAG, "Failed to allocate memory for packet\r\n");
		return - MR_ERR_NO_MEMORY;
	}
	memcpy(packet->data, buffer, length);

	packet->head.crc = link->get_crc(packet->data, length);

	link->tx_sequence ++;
	if (link->tx_packet == MR_NULL)
		link->tx_packet = packet;
	else
		link->tx_packet->next = packet;

	return MR_ERR_OK;
}

mr_err_t mr_link_packet_parse(mr_link_packet_t packet, mr_link_t link)
{
	MR_ASSERT(packet != NULL);
	MR_ASSERT(link != NULL);

//	if (packet->head.start == m)

}

mr_uint16_t mr_link_parse_packet(mr_link_packet_t packet, mr_uint8_t *buffer)
{
	MR_ASSERT(packet != NULL);
	MR_ASSERT(buffer != NULL);

}
