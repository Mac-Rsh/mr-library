/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-09     MacRsh       first version
 */

#include <mrlib.h>

mr_device_t mr_device_find(const char *name)
{
	mr_device_t device = MR_NULL;

	/* Find the device object from the device container */
	device = (mr_device_t)mr_object_find(name, MR_CONTAINER_TYPE_DEVICE);

	return device;
}

mr_err_t mr_device_add_to_container(mr_device_t device,
									const char *name,
									enum mr_device_type type,
									mr_uint16_t support_flag,
									const struct mr_device_ops *ops,
									void *data)
{
	mr_err_t ret = MR_ERR_OK;
	static struct mr_device_ops null_ops = {MR_NULL};

	MR_ASSERT(device != MR_NULL);
	MR_ASSERT(support_flag != MR_NULL);

	/* Add the object to the container */
	ret = mr_object_add_to_container(&device->object, name, MR_CONTAINER_TYPE_DEVICE);
	if (ret != MR_ERR_OK)
		return ret;

	/* Initialize the private fields */
	device->rx_callback = MR_NULL;
	device->tx_callback = MR_NULL;
	device->type = type;
	device->support_flag = support_flag;
	device->open_flag = MR_NULL;
	device->data = data;

	/* Set operations as null-ops if ops is null */
	device->ops = (ops == MR_NULL) ? &null_ops : ops;

	return ret;
}

mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags)
{
	MR_ASSERT(device != MR_NULL);
	MR_ASSERT(flags != MR_NULL);

	/* Check if the specified open flags are supported by the device */
	if (flags != (flags & device->support_flag))
		return - MR_ERR_UNSUPPORTED;

	/* Check if the device is already closed */
	if ((device->open_flag & MR_OPEN_ACTIVE))
		return MR_ERR_OK;

	/* Update the device open-flag and refer-count */
	device->open_flag = (flags & _MR_OPEN_FLAG_MASK);
	device->open_flag |= MR_OPEN_ACTIVE;
	device->ref_count ++;

	/* Call the device-open function, if provided */
	if (device->ops->open == MR_NULL)
		return MR_ERR_OK;

	return device->ops->open(device);
}

mr_err_t mr_device_close(mr_device_t device)
{
	MR_ASSERT(device != MR_NULL);

	/* If the reference count is zero, the device has already been closed */
	if (device->ref_count == 0)
	{
		device->open_flag = MR_OPEN_CLOSED;
		return MR_ERR_OK;
	}

	/* Decrement the reference count */
	device->ref_count --;

	/* If the reference count is still non-zero, return without closing the device */
	if (device->ref_count != 0)
		return MR_ERR_OK;

	/* Call the device-close function, if provided */
	if (device->ops->close == MR_NULL)
		return MR_ERR_OK;

	return device->ops->close(device);
}

mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args)
{
	MR_ASSERT(device != MR_NULL);

	/* Call the device-ioctl function, if provided */
	if (device->ops->ioctl == MR_NULL)
		return - MR_ERR_UNSUPPORTED;

	return device->ops->ioctl(device, cmd, args);
}

mr_size_t mr_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t count)
{
	MR_ASSERT(device != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	/* Check if the device is closed or unsupported */
	if ((device->ref_count == 0) || ! (device->open_flag & MR_OPEN_RDONLY))
		return 0;

	/* Call the device-read function, if provided */
	if (device->ops->read == MR_NULL)
		return 0;

	return device->ops->read(device, pos, buffer, count);
}

mr_size_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t count)
{
	MR_ASSERT(device != MR_NULL);
	MR_ASSERT(buffer != MR_NULL);

	/* Check if the device is closed or unsupported */
	if ((device->ref_count == 0) || ! (device->open_flag & MR_OPEN_WRONLY))
		return 0;

	/* Call the device-write function, if provided */
	if (device->ops->write == MR_NULL)
		return 0;

	return device->ops->write(device, pos, buffer, count);
}