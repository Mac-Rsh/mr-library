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

    /* Debug log */
    MR_LOG_D(name);

    /* Find the device object from the device container */
    device = (mr_device_t) mr_object_find(name, MR_CONTAINER_TYPE_DEVICE);
    if (device == MR_NULL)
    {
        /* Error log */
        MR_LOG_E(name, -MR_ERR_NOT_FOUND);
        return MR_NULL;
    }

    return device;
}

void mr_device_init(mr_device_t device, const char *name)
{
    /* Debug log */
    MR_LOG_D(name);

    /* Initialize the object */
    mr_object_init(&device->object, name);

    /* Initialize the private fields */
    device->rx_callback = MR_NULL;
    device->tx_callback = MR_NULL;
    device->type = MR_DEVICE_TYPE_NULL;
    device->support_flag = MR_OPEN_CLOSED;
    device->open_flag = MR_OPEN_CLOSED;
    device->ref_count = 0;
    device->data = MR_NULL;
    device->ops = MR_NULL;
}

mr_err_t mr_device_add_to_container(mr_device_t device,
                                    enum mr_device_type type,
                                    mr_uint16_t support_flag,
                                    const struct mr_device_ops *ops,
                                    void *data)
{
    mr_err_t error_code = MR_ERR_OK;
    static struct mr_device_ops null_ops = {MR_NULL};

    /* Debug log */
    MR_LOG_D(device->object.name);

    /* Initialize the private fields */
    device->type = type;
    device->support_flag = support_flag;
    device->data = data;

    /* Set operations as null_ops if ops is null */
    device->ops = (ops == MR_NULL) ? &null_ops : ops;

    /* Register the object to the container */
    error_code = mr_object_add_to_container(&device->object, MR_CONTAINER_TYPE_DEVICE);
    MR_LOG_E(device->object.name, error_code);

    return error_code;
}

mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags)
{
    mr_err_t error_code = MR_ERR_OK;

    /* Debug log */
    MR_LOG_D(device->object.name);

    /* Check if the specified open flags are supported by the device */
    if (flags != (flags & device->support_flag))
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_UNSUPPORTED);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Check if the device is already closed */
    if (!(device->open_flag & MR_OPEN_ACTIVE))
    {
        device->open_flag = (flags & _MR_OPEN_FLAG_MASK);

        /* Call the device open function, if provided */
        if (device->ops->open != MR_NULL)
        {
            error_code = device->ops->open(device);

            /* Error log */
            MR_LOG_E(device->object.name, error_code);
            if (error_code != MR_ERR_OK)
                return error_code;
        }

        /* Update the device type as active */
        device->open_flag |= MR_OPEN_ACTIVE;
        device->ref_count++;
    }

    return MR_ERR_OK;
}

mr_err_t mr_device_close(mr_device_t device)
{
    mr_err_t error_code = MR_ERR_OK;

    /* Debug log */
    MR_LOG_D(device->object.name);

    /* If the reference count is zero, the device has already been closed */
    if (device->ref_count == 0)
    {
        device->open_flag = MR_OPEN_CLOSED;
        return MR_ERR_OK;
    }

    /* Decrement the reference count */
    device->ref_count--;

    /* If the reference count is still non-zero, return without closing the device */
    if (device->ref_count != 0)
        return MR_ERR_OK;

    /* Call the device close function, if provided */
    if (device->ops->close != MR_NULL)
    {
        error_code = device->ops->close(device);

        /* Error log */
        MR_LOG_E(device->object.name, error_code);
        if (error_code != MR_ERR_OK)
            return error_code;
    }

    return MR_ERR_OK;
}

mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_err_t error_code = MR_ERR_OK;

    /* Debug log */
    MR_LOG_D(device->object.name);

    /* Check if the device is closed */
    if (device->ref_count == 0)
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_UNSUPPORTED);
        return -MR_ERR_UNSUPPORTED;
    }

    /* Call the device ioctl function, if provided */
    if (device->ops->ioctl != MR_NULL)
    {
        error_code = device->ops->ioctl(device, cmd, args);

        /* Error log */
        MR_LOG_E(device->object.name, error_code);
        return error_code;
    } else
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_IO);
        return -MR_ERR_UNSUPPORTED;
    }
}

mr_size_t mr_device_read(mr_device_t device, mr_off_t pos, void *buf, mr_size_t count)
{
    /* Debug log */
    MR_LOG_D(device->object.name);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || !(device->open_flag & MR_OPEN_RDONLY))
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_UNSUPPORTED);
        return 0;
    }

    /* Call the device read function, if provided */
    if (device->ops->read != MR_NULL)
    {
        return device->ops->read(device, pos, buf, count);
    } else
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_IO);
        return 0;
    }
}

mr_size_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buf, mr_size_t count)
{
    /* Debug log */
    MR_LOG_D(device->object.name);

    /* Check if the device is closed or unsupported */
    if ((device->ref_count == 0) || !(device->open_flag & MR_OPEN_WRONLY))
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_UNSUPPORTED);
        return 0;
    }

    /* Call the device write function, if provided */
    if (device->ops->write != MR_NULL)
    {
        return device->ops->write(device, pos, buf, count);
    } else
    {
        /* Error log */
        MR_LOG_E(device->object.name, -MR_ERR_IO);
        return 0;
    }
}