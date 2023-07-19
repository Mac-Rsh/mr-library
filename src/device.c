/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrlib.h"

#if (MR_CONF_DEVICE == MR_CONF_ENABLE)

#define DEBUG_TAG   "device"

/**
 * @brief This function finds a device.
 *
 * @param name The name of the device.
 *
 * @return A handle to the found device, or MR_NULL if not found.
 */
mr_device_t mr_device_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the device object from the device container */
    return (mr_device_t)mr_object_find(name, MR_OBJECT_TYPE_DEVICE);
}

/**
 * @brief This function adds a device to the container.
 *
 * @param device The device to be added.
 * @param name The name of the device.
 * @param flags The open flags supported by the device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_add(mr_device_t device, const char *name, mr_uint16_t flags)
{
    mr_err_t ret = MR_ERR_OK;
    static const struct mr_device_ops null_ops = {MR_NULL};

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(flags != MR_OPEN_CLOSED);

    /* Add the object to the container */
    ret = mr_object_add(&device->object, name, MR_OBJECT_TYPE_DEVICE);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s add failed: %d.\r\n", device->object.name, ret);
        return ret;
    }

    /* Initialize the private fields */
    device->support_flag = flags;
    device->open_flag = MR_OPEN_CLOSED;
    device->ref_count = 0;
    device->rx_cb = MR_NULL;
    device->tx_cb = MR_NULL;

    /* Set operations as null-ops if ops is null */
    if (device->ops == MR_NULL)
    {
        device->ops = &null_ops;
    }

    return MR_ERR_OK;
}

/**
 * @brief This function removes a device from the container.
 *
 * @param device The device to be removed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_remove(mr_device_t device)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);

    /* Remove the object from the container */
    ret = mr_object_remove(&device->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s remove failed: %d.\r\n", device->object.name, ret);
        return ret;
    }

    /* Reset the private fields */
    device->open_flag = MR_OPEN_CLOSED;
    device->ref_count = 0;
    device->rx_cb = MR_NULL;
    device->tx_cb = MR_NULL;

    return MR_ERR_OK;
}

/**
 * @brief This function opens the device.
 *
 * @param device The device to be opened.
 * @param flags The open flags of the device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags)
{
    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);

    /* Check if the specified open flags are supported by the device */
    if (flags != (flags & device->support_flag))
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unsupported open flags: 0x%x.\r\n", device->object.name, flags);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Update the device open flag and reference count */
    device->open_flag |= (flags & _MR_OPEN_FLAG_MASK);
    device->ref_count++;

    /* Check if the device is already open */
    if ((device->open_flag & MR_OPEN_ACTIVE))
    {
        return MR_ERR_OK;
    }

    /* Set the device status to active */
    device->open_flag |= MR_OPEN_ACTIVE;

    /* Call the open function, if provided */
    if (device->ops->open == MR_NULL)
    {
        return MR_ERR_OK;
    }

    return device->ops->open(device);
}

/**
 * @brief This function closes the device.
 *
 * @param device The device to be closed.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_close(mr_device_t device)
{
    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);

    /* Check if the device is already closed */
    if (device->open_flag == MR_OPEN_CLOSED)
    {
        return MR_ERR_OK;
    }

    /* Decrement the reference count */
    device->ref_count--;

    /* If the reference count is still non-zero, return without closing the device */
    if (device->ref_count != 0)
    {
        return MR_ERR_OK;
    }

    /* Set the device status to closed */
    device->open_flag = MR_OPEN_CLOSED;
    device->rx_cb = MR_NULL;
    device->tx_cb = MR_NULL;

    /* Call the close function, if provided */
    if (device->ops->close == MR_NULL)
    {
        return MR_ERR_OK;
    }

    return device->ops->close(device);
}

/**
 * @brief This function controls the device.
 *
 * @param device The device to be controlled.
 * @param cmd The operation command of the device.
 * @param args The argument of command.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args)
{
    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);

    /* Call the ioctl function, if provided */
    if (device->ops->ioctl == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unsupported ioctl.\r\n", device->object.name);
        return -MR_ERR_UNSUPPORTED;
    }

    return device->ops->ioctl(device, cmd, args);
}

/**
 * @brief This function reads from the device.
 *
 * @param device The device to be read.
 * @param pos The read position.
 * @param buffer The data buffer to be read from the device.
 * @param size The size of the read.
 *
 * @return The size of the actual read on success, otherwise an error code.
 */
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);
    MR_ASSERT(buffer != MR_NULL);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || !(device->open_flag & MR_OPEN_RDONLY))
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unsupported read.\r\n", device->object.name);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Call the read function, if provided */
    if (device->ops->read == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unprovided read operation.\r\n", device->object.name);
        return -MR_ERR_IO;
    }

    return device->ops->read(device, pos, buffer, size);
}

/**
 * @brief This function write the device.
 *
 * @param device The device to be written.
 * @param pos The write position.
 * @param buffer The data buffer to be written to device.
 * @param size The size of write.
 *
 * @return The size of the actual write on success, otherwise return 0.
 */
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type & MR_OBJECT_TYPE_DEVICE);
    MR_ASSERT(buffer != MR_NULL);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || !(device->open_flag & MR_OPEN_WRONLY))
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unsupported write.\r\n", device->object.name);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Call the write function, if provided */
    if (device->ops->write == MR_NULL)
    {
        MR_DEBUG_D(DEBUG_TAG, "%s unprovided write operation.\r\n", device->object.name);
        return -MR_ERR_IO;
    }

    return device->ops->write(device, pos, buffer, size);
}

#endif /* MR_CONFIG_DEVICE */