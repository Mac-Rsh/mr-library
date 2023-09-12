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

#if (MR_CFG_PIN == MR_CFG_ENABLE)

static mr_err_t err_io_pin_configure(mr_pin_t pin, struct mr_pin_config *config)
{
    return -MR_ERR_IO;
}

static mr_level_t err_io_pin_read(mr_pin_t pin, mr_off_t number)
{
    return 0;
}

static void err_io_pin_write(mr_pin_t pin, mr_off_t number, mr_level_t level)
{

}

static mr_err_t mr_pin_ioctl(mr_device_t device, int cmd, void *args)
{
    mr_pin_t pin = (mr_pin_t)device;

    switch (cmd)
    {
        case MR_DEVICE_CTRL_SET_CONFIG:
        {
            if (args)
            {
                mr_pin_config_t config = (mr_pin_config_t)args;
                return pin->ops->configure(pin, config);
            }
            return -MR_ERR_INVALID;
        }

        case MR_DEVICE_CTRL_SET_RX_CB:
        {
            device->rx_cb = (mr_device_cb_t)args;
            return MR_ERR_OK;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

static mr_ssize_t mr_pin_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size)
{
    mr_pin_t pin = (mr_pin_t)device;
    mr_level_t *read_buffer = (mr_level_t *)buffer;
    mr_size_t read_size = 0;

    if (pos < 0)
    {
        return -MR_ERR_INVALID;
    }

    while ((read_size += sizeof(*read_buffer)) <= size)
    {
        *read_buffer = pin->ops->read(pin, pos);
        read_buffer++;
    }

    return (mr_ssize_t)read_size;
}

static mr_ssize_t mr_pin_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size)
{
    mr_pin_t pin = (mr_pin_t)device;
    mr_level_t *write_buffer = (mr_level_t *)buffer;
    mr_size_t write_size = 0;

    if (pos < 0)
    {
        return -MR_ERR_INVALID;
    }

    while ((write_size += sizeof(*write_buffer)) <= size)
    {
        pin->ops->write(pin, pos, *write_buffer);
        write_buffer++;
    }

    return (mr_ssize_t)write_size;
}

/**
 * @brief This function adds the pin device.
 *
 * @param pin The pin device to be added.
 * @param name The name of the pin device.
 * @param ops The operations of the pin device.
 * @param data The private data of the pin device.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
mr_err_t mr_pin_device_add(mr_pin_t pin, const char *name, struct mr_pin_ops *ops, void *data)
{
    static struct mr_device_ops device_ops =
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

    /* Protect every operation of the pin device */
    ops->configure = ops->configure ? ops->configure : err_io_pin_configure;
    ops->write = ops->write ? ops->write : err_io_pin_write;
    ops->read = ops->read ? ops->read : err_io_pin_read;
    pin->ops = ops;

    /* Add the device */
    return mr_device_add(&pin->device, name, Mr_Device_Type_Pin, MR_DEVICE_OFLAG_RDWR, &device_ops, data);
}

/**
 * @brief This function service interrupt routine of the pin device.
 *
 * @param pin The pin device.
 * @param number The number of the interrupt.
 */
void mr_pin_device_isr(mr_pin_t pin, mr_off_t number)
{
    MR_ASSERT(pin != MR_NULL);

    /* Call the receiving completion function */
    if (pin->device.rx_cb != MR_NULL)
    {
        pin->device.rx_cb(&pin->device, &number);
    }
}

#endif