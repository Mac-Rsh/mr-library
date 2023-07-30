/*
 * Copyright (c) 2023, mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-23     MacRsh       first version
 */

#include "device/pin/pin.h"

#if (MR_CONF_PIN == MR_CONF_ENABLE)

static mr_err_t mr_pin_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_pin_t pin = (mr_pin_t)device;

    switch (cmd & _MR_CTRL_FLAG_MASK)
    {
        case MR_CTRL_SET_CONFIG:
        {
            if (args)
            {
                struct mr_pin_config *config = (struct mr_pin_config *)args;
                return pin->ops->configure(pin, config);
            }
            return -MR_ERR_INVALID;
        }

        case MR_CTRL_SET_RX_CB:
        {
            device->rx_cb = args;
            return MR_ERR_OK;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_pin_read(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size)
{
    mr_pin_t pin = (mr_pin_t)device;
    mr_level_t *read_buffer = (mr_level_t *)buffer;
    mr_size_t read_size = 0;

    if (size < sizeof(*read_buffer))
    {
        return -MR_ERR_INVALID;
    }

    for (read_size = 0; read_size < size; read_size += sizeof(*read_buffer))
    {
        *read_buffer = pin->ops->read(pin, pos);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_pin_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size)
{
    mr_pin_t pin = (mr_pin_t)device;
    mr_level_t *write_buffer = (mr_level_t *)buffer;
    mr_size_t write_size = 0;

    if (size < sizeof(*write_buffer))
    {
        return -MR_ERR_INVALID;
    }

    for (write_size = 0; write_size < size; write_size += sizeof(*write_buffer))
    {
        pin->ops->write(pin, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

static mr_err_t _err_io_pin_configure(mr_pin_t pin, struct mr_pin_config *config)
{
    MR_ASSERT(0);
    return -MR_ERR_IO;
}

static void _err_io_pin_write(mr_pin_t pin, mr_pos_t number, mr_level_t level)
{
    MR_ASSERT(0);
}

static mr_level_t _err_io_pin_read(mr_pin_t pin, mr_pos_t number)
{
    MR_ASSERT(0);
    return 0;
}

mr_err_t mr_pin_device_add(mr_pin_t pin, const char *name, void *data, struct mr_pin_ops *ops)
{
    const static struct mr_device_ops device_ops =
            {
                    MR_NULL,
                    MR_NULL,
                    mr_pin_ioctl,
                    mr_pin_read,
                    mr_pin_write,
            };

    MR_ASSERT(pin != MR_NULL);
    MR_ASSERT(name != MR_NULL);
    MR_ASSERT(ops != MR_NULL);

    /* Initialize the private fields */
    pin->device.type = MR_DEVICE_TYPE_PIN;
    pin->device.data = data;
    pin->device.ops = &device_ops;

    /* Set operations as protection-ops if ops is null */
    ops->configure = ops->configure ? ops->configure : _err_io_pin_configure;
    ops->write = ops->write ? ops->write : _err_io_pin_write;
    ops->read = ops->read ? ops->read : _err_io_pin_read;
    pin->ops = ops;

    /* Add to the container */
    return mr_device_add(&pin->device, name, MR_OPEN_RDWR);
}

void mr_pin_device_isr(mr_pin_t pin, mr_pos_t number)
{
    MR_ASSERT(pin != MR_NULL);

    /* Invoke the rx-cb function */
    if (pin->device.rx_cb != MR_NULL)
    {
        pin->device.rx_cb(&pin->device, &number);
    }
}

#endif  /* MR_CONF_PIN */