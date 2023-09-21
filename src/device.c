/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "mrapi.h"

#if (MR_CFG_DEVICE == MR_CFG_ENABLE)

#define DEBUG_TAG   "device"

static mr_ssize_t err_io_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    return -MR_ERR_IO;
}

static mr_ssize_t err_io_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    return -MR_ERR_IO;
}

/**
 * @brief This function finds a device.
 *
 * @param name The name of the device.
 *
 * @return A pointer to the found device, or MR_NULL if not found.
 */
mr_device_t mr_device_find(const char *name)
{
    MR_ASSERT(name != MR_NULL);

    /* Find the device object from the container */
    return (mr_device_t)mr_object_find(name, Mr_Object_Type_Device);
}

/**
 * @brief This function adds a device to the container.
 *
 * @param device The device to be added.
 * @param name The name of the device.
 * @param type The type of the device.
 * @param sflags The support open mode flags of the device.
 * @param ops The operations supported by the device.
 * @param data The private data of the device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_add(mr_device_t device,
                       const char *name,
                       mr_uint8_t type,
                       mr_uint8_t sflags,
                       struct mr_device_ops *ops,
                       void *data)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.magic != MR_OBJECT_MAGIC);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    device->type = type;
    device->sflags = sflags;
    device->oflags = MR_DEVICE_OFLAG_CLOSED;
    device->ref_count = 0;
    mr_rb_init(&device->rx_fifo, MR_NULL, 0);
    mr_rb_init(&device->tx_fifo, MR_NULL, 0);
    device->rx_cb = MR_NULL;
    device->tx_cb = MR_NULL;

    /* Protect every operation of the device */
    ops->read = ops->read ? ops->read : err_io_read;
    ops->write = ops->write ? ops->write : err_io_write;
    device->ops = ops;

    /* Set the private data */
    device->data = data;

    /* Add the object to the container */
    ret = mr_object_add(&device->object, name, Mr_Object_Type_Device);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] add failed: [%d]\r\n", name, ret);
    }

    return ret;
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
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);

    /* Remove the object from the container */
    ret = mr_object_remove(&device->object);
    if (ret != MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] remove failed: [%d]\r\n", device->object.name, ret);
        return ret;
    }

    /* Reset the device */
    mr_rb_allocate_buffer(&device->rx_fifo, 0);
    mr_rb_allocate_buffer(&device->tx_fifo, 0);

    return ret;
}

/**
 * @brief This function opens the device.
 *
 * @param device The device to be opened.
 * @param oflags The open mode flags of the device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_device_open(mr_device_t device, mr_uint8_t oflags)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);

    /* Check whether the device supports opening in this mode */
    if (oflags != (oflags & device->sflags))
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] open [%x] failed: [%d]\r\n", device->object.name, oflags, -MR_ERR_UNSUPPORTED);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Update the device open flag and reference count */
    device->oflags |= (oflags & MR_DEVICE_OFLAG_MASK);
    device->ref_count++;

    /* Device is already opened */
    if (device->ref_count > 1)
    {
        return MR_ERR_OK;
    }

    /* Call the open operation, if provided */
    if (device->ops->open != MR_NULL)
    {
        ret = device->ops->open(device);
        if (ret != MR_ERR_OK)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] open [%x] failed: [%d]\r\n", device->object.name, oflags, ret);
        }
        return ret;
    }

    return MR_ERR_OK;
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
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);

    /* Check if the device is already closed */
    if (device->oflags == MR_DEVICE_OFLAG_CLOSED)
    {
        return MR_ERR_OK;
    }

    /* Decrement the reference count */
    device->ref_count--;

    /* If the reference count is still non-zero, return without shutting down the device */
    if (device->ref_count != 0)
    {
        return MR_ERR_OK;
    }

    /* Set the device status to closed */
    device->oflags = MR_DEVICE_OFLAG_CLOSED;
    mr_rb_reset(&device->rx_fifo);
    mr_rb_reset(&device->tx_fifo);
    device->rx_cb = MR_NULL;
    device->tx_cb = MR_NULL;

    /* Call the close operation, if provided */
    if (device->ops->close != MR_NULL)
    {
        ret = device->ops->close(device);
        if (ret != MR_ERR_OK)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] close failed: [%d]\r\n", device->object.name, ret);
        }
        return ret;
    }

    return MR_ERR_OK;
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
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);

    /* Call the ioctl operation, if provided */
    if (device->ops->ioctl != MR_NULL)
    {
        ret = device->ops->ioctl(device, cmd, args);
        if (ret != MR_ERR_OK)
        {
            MR_DEBUG_D(DEBUG_TAG, "[%s] ioctl [%x] failed: [%d]\r\n", device->object.name, cmd, ret);
        }
        return ret;
    }

    return -MR_ERR_UNSUPPORTED;
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
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);
    MR_ASSERT(buffer != MR_NULL);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || ((device->oflags & MR_DEVICE_OFLAG_RDONLY) == 0))
    {
        return -MR_ERR_UNSUPPORTED;
    }

    /* Call the read operation */
    ret = device->ops->read(device, pos, buffer, size);
    if (ret < MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] read [%d] failed: [%d]\r\n", device->object.name, pos, ret);
    }

    return ret;
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
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_err_t ret = MR_ERR_OK;

    MR_ASSERT(device != MR_NULL);
    MR_ASSERT(device->object.type == Mr_Object_Type_Device);
    MR_ASSERT(buffer != MR_NULL);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || ((device->oflags & MR_DEVICE_OFLAG_WRONLY) == 0))
    {
        return -MR_ERR_UNSUPPORTED;
    }

    /* Call the write operation */
    ret = device->ops->write(device, pos, buffer, size);
    if (ret < MR_ERR_OK)
    {
        MR_DEBUG_D(DEBUG_TAG, "[%s] write [%d] failed: [%d]\r\n", device->object.name, pos, ret);
    }

    return ret;
}

#endif