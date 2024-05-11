/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 * @date 2024-04-07    MacRsh       Add async operation support
 * @date 2024-05-08    MacRsh       Add attach/detach operation
 */

#include "../mr-library/include/mr_api.h"
#include <string.h>

#ifdef MR_USE_DEVICE_LOG
#undef MR_LOG_TAG
#define MR_LOG_TAG                      ("device")
#endif /* MR_USE_DEVICE_LOG */

#define _MAGIC_NUMBER                   (0xdeadbeef)        /**< Magic number */
#define _FLAG_IN_ISR                    (0x80 << 24)        /**< In ISR */

/**
 * @brief Device event complete structure.
 */
struct _device_event
{
    struct mr_list list;                                    /**< Event list */
    uint32_t descriptor: 31;                                /**< Event descriptor */
    uint32_t free: 1;                                       /**< Free flag */
    uint32_t event: 31;                                     /**< Event */
    uint32_t private: 1;                                    /**< Private flag */
    void (*callback)(int descriptor, uint32_t event,
                     void *args);                           /**< Callback function */
};

static struct mr_device _root_device = {
    .magic = _MAGIC_NUMBER,
    .name = "/",
    .list = MR_LIST_INIT(&_root_device.list),
    .clist = MR_LIST_INIT(&_root_device.clist),
    .parent = NULL};

#ifndef MR_CFG_DESCRIPTOR_MAX
#define MR_CFG_DESCRIPTOR_MAX           (128)
#endif /* MR_CFG_DESCRIPTOR_MAX */
static struct mr_descriptor _descriptor_map[MR_CFG_DESCRIPTOR_MAX] = {0};

MR_INLINE struct mr_device *_device_find_from(const char *name,
                                              struct mr_device *parent)
{
    /* Deal with special names */
    if (strcmp(name, "..") == 0)
    {
        return parent->parent;
    }
    if (strcmp(name, ".") == 0)
    {
        return parent;
    }

    /* Find the device with the specified name */
    for (struct mr_list *list = parent->clist.next; list != &parent->clist;
         list = list->next)
    {
        struct mr_device *iter = MR_CONTAINER_OF(list, struct mr_device, list);
        if (strcmp(name, iter->name) == 0)
        {
            return iter;
        }
    }
    return NULL;
}

MR_INLINE struct mr_device *_device_next_find(const char **path,
                                              struct mr_device *parent)
{
    /* Skip the leading '/' */
    if ((*path)[0] == '/')
    {
        (*path)++;
    }

    /* Find the next path */
    const char *next_path = strchr((*path), '/');
    if (next_path != NULL)
    {
        char name[MR_CFG_DEVICE_NAME_MAX + 1];
        size_t len = MR_BOUND(next_path - (*path), 0, sizeof(name));

        strncpy(name, *path, len);
        name[len] = '\0';
        *path = next_path + 1;

        /* Find the next device */
        return _device_find_from(name, parent);
    }

    /* Without next */
    return NULL;
}

static int _device_register_iter(struct mr_device *device, const char *path,
                                 struct mr_device *parent)
{
    /* Find the next path */
    struct mr_device *next_parent = _device_next_find(&path, parent);
    if (next_parent != NULL)
    {
        /* Continue the registration */
        return _device_register_iter(device, path, next_parent);
    }

    /* Check if the device exists */
    if (_device_find_from(path, parent) != NULL)
    {
        return MR_EEXIST;
    }

    /* Notify the parent device that the child device will be attached */
    if (parent->ops->attach != NULL)
    {
        int ret = parent->ops->attach(parent, device);
        if (ret < 0)
        {
            return ret;
        }
    }

    /* Register the device */
    device->magic = _MAGIC_NUMBER;
    strncpy(device->name, path, sizeof(device->name));
    mr_list_insert_before(&parent->clist, &device->list);
    device->parent = parent;
    return MR_EOK;
}

static struct mr_device *_device_find_iter(const char *path,
                                           struct mr_device *parent)
{
    struct mr_device *next_parent = _device_next_find(&path, parent);
    if (next_parent != NULL)
    {
        /* Continue the search */
        return _device_find_iter(path, next_parent);
    }

    /* Find the device from the parent */
    return _device_find_from(path, parent);
}

MR_INLINE int _device_event_create(struct mr_device *device, int descriptor,
                                   const struct mr_device_event *event)
{
    int ret;

    /* Critical section enter */
    mr_critical_enter();

    /* Check if the event exists */
    for (struct mr_list *list = device->elist.next; list != &device->elist;
         list = list->next)
    {
        struct _device_event *_event =
            MR_CONTAINER_OF(list, struct _device_event, list);

        if ((_event->descriptor == descriptor) &&
            (_event->event == event->event))
        {
            ret = MR_EEXIST;
            goto _exit;
        }
    }

    /* Create the new event */
    struct _device_event *_event = mr_malloc(sizeof(struct _device_event));
    if (_event == NULL)
    {
        ret = MR_ENOMEM;
        goto _exit;
    }

    /* Add the event */
    mr_list_init(&_event->list);
    _event->descriptor = descriptor;
    _event->free = false;
    _event->event = event->event;
    _event->private = event->private;
    _event->callback = event->callback;
    mr_list_insert_before(&device->elist, &_event->list);
    ret = MR_EOK;

_exit:
    /* Critical section exit */
    mr_critical_exit();
    return ret;
}

MR_INLINE int _device_event_destroy(const struct mr_device *device,
                                    int descriptor,
                                    const struct mr_device_event *event)
{
    int ret;

    /* Critical section enter */
    mr_critical_enter();

    /* Find the event */
    for (struct mr_list *list = device->elist.next; list != &device->elist;
         list = list->next)
    {
        struct _device_event *_event =
            MR_CONTAINER_OF(list, struct _device_event, list);

        if ((_event->descriptor == descriptor) &&
            (_event->event == event->event))
        {
            /* Destroy the event */
            if (MR_BIT_IS_SET(device->flags, _FLAG_IN_ISR) == true)
            {
                /* The device is in ISR, make the event free */
                _event->free = true;
            } else
            {
                /* The device is not in ISR, destroy the event */
                mr_list_remove(&_event->list);
                mr_free(_event);
            }
            ret = MR_EOK;
            goto _exit;
        }
    }

    /* Not found the event */
    ret = MR_ENOENT;

_exit:
    /* Critical section exit */
    mr_critical_exit();
    return ret;
}

MR_INLINE void _device_event_destroy_all(const struct mr_device *device,
                                         int descriptor)
{
    /* Critical section enter */
    mr_critical_enter();

    /* Destroy all events for the specified descriptor */
    for (struct mr_list *list = device->elist.next; list != &device->elist;
         list = list->next)
    {
        struct _device_event *_event =
            MR_CONTAINER_OF(list, struct _device_event, list);

        if (_event->descriptor == descriptor)
        {
            /* Destroy the event */
            if (MR_BIT_IS_SET(device->flags, _FLAG_IN_ISR) == true)
            {
                /* The device is in ISR, make the event free */
                _event->free = true;
            } else
            {
                /* The device is not in ISR, destroy the event */
                list = list->prev;
                mr_list_remove(&_event->list);
                mr_free(_event);
            }
        }
    }

    /* Critical section exit */
    mr_critical_exit();
}

MR_INLINE void _device_event_handler(const struct mr_device *device,
                                     uint32_t event, void *args)
{
    /* Trigger events */
    for (struct mr_list *list = device->elist.next; list != &device->elist;
         list = list->next)
    {
        struct _device_event *_event =
            MR_CONTAINER_OF(list, struct _device_event, list);

        /* If the event is not private, take the mask */
        event = (_event->private == false) ? event & MR_EVENT_MASK : event;

        /* Check if the event is free */
        if (_event->free == true)
        {
            goto _free;
        }

        /* Call the callback if the event matches */
        if (_event->event == event)
        {
            _event->callback(_event->descriptor, _event->event, args);

            /* Make the event free if it is in ISR */
            if (_event->free == true)
            {
            _free:
                /* Destroy the event */
                list = list->prev;
                mr_list_remove(&_event->list);
                mr_free(_event);
            }
        }
    }

    /* Destroy free events */
    for (struct mr_list *list = device->elist.next; list != &device->elist;
         list = list->next)
    {
        struct _device_event *_event =
            MR_CONTAINER_OF(list, struct _device_event, list);

        /* Make the event free if it is in ISR */
        if (_event->free == true)
        {
            /* Destroy the event */
            list = list->prev;
            mr_list_remove(&_event->list);
            mr_free(_event);
        }
    }
}

MR_INLINE bool _device_flags_is_valid(const struct mr_device *device,
                                      uint32_t flags)
{
    return MR_BIT_IS_SET(device->flags, flags);
}

MR_INLINE bool _descriptor_is_valid(int descriptor)
{
    return (descriptor >= 0) && (descriptor < MR_ARRAY_NUM(_descriptor_map)) &&
           (_descriptor_map[descriptor].device != NULL);
}

MR_INLINE bool _descriptor_flags_is_valid(int descriptor, uint32_t flags)
{
    return MR_BIT_IS_SET(_descriptor_map[descriptor].flags, flags);
}

MR_INLINE struct mr_device *_device_find(const char *path)
{
    /* Critical section enter */
    mr_critical_enter();

    /* Find the device */
    struct mr_device *device = _device_find_iter(path, &_root_device);

    /* Critical section exit */
    mr_critical_exit();
    return device;
}

MR_INLINE int _device_take(struct mr_device *device, int descriptor,
                           uint32_t mask)
{
    int ret;

    /* If the device is not FDX, the read/writing must be locked */
    mask = (device->fdx == true) ? mask : _MR_OPERATE_MASK_ALL;

    /* Calculate the lock mask, since the descriptor can be 0, need to add 1 */
    uint32_t lock = (((descriptor + 1) << 16) | (descriptor + 1)) & mask;

    /* Critical section enter */
    mr_critical_enter();

    /* Check if the descriptor and lock are valid */
    if (_descriptor_is_valid(descriptor) == false)
    {
        ret = MR_EINVAL;
    } else if ((device->lock & mask) == 0)
    {
        MR_BIT_SET(device->lock, lock);
        ret = MR_EOK;
    } else
    {
        ret = MR_EBUSY;
    }

    /* Critical section exit */
    mr_critical_exit();
    return ret;
}

MR_INLINE void _device_release(struct mr_device *device, uint32_t mask)
{
    /* If the device is not FDX, the read/writing must be locked */
    mask = (device->fdx == true) ? mask : _MR_OPERATE_MASK_ALL;

    /* Release the device lock */
    MR_BIT_CLR(device->lock, mask);
}

MR_INLINE int _descriptor_allocate(struct mr_device *device, uint32_t flags)
{
    int descriptor = -1;

    /* Check if the flags are valid */
    if (_device_flags_is_valid(device, flags) == false)
    {
        return MR_EPERM;
    }

    /* Critical section enter */
    mr_critical_enter();

    /* Try to allocate the descriptor */
    for (int i = 0; i < MR_ARRAY_NUM(_descriptor_map); i++)
    {
        if (_descriptor_map[i].device == NULL)
        {
            _descriptor_map[i].device = device;
            _descriptor_map[i].flags = flags;
            _descriptor_map[i].pos = -1;
            descriptor = i;
            break;
        }
    }

    /* Critical section exit */
    mr_critical_exit();

    /* Return the descriptor or error */
    return (descriptor >= 0) ? descriptor : MR_ENOMEM;
}

MR_INLINE void _descriptor_free(int descriptor)
{
    /* Critical section enter */
    mr_critical_enter();

    /* Free the descriptor */
    _descriptor_map[descriptor].device = NULL;

    /* Critical section exit */
    mr_critical_exit();
}

static int _device_register(struct mr_device *device, const char *path,
                            uint32_t type, struct mr_device_ops *ops,
                            const void *driver, const char *to_path)
{
    static struct mr_device_ops null_ops = {NULL};
    struct mr_device *to_device = &_root_device;

    /* If specify a registration target, find it */
    if (to_path != NULL)
    {
        to_device = _device_find(to_path);
        if (to_device == NULL)
        {
            return MR_ENOENT;
        }
    }

    /* Set default ops if not specified */
    ops = (ops == NULL) ? &null_ops : ops;
    to_device->ops = (to_device->ops == NULL) ? &null_ops : to_device->ops;

    /* Initialize the device */
    mr_list_init(&device->list);
    mr_list_init(&device->clist);
    device->parent = NULL;
    device->type = type & (~MR_DEVICE_TYPE_FDX);
    device->fdx = MR_BIT_IS_SET(type, MR_DEVICE_TYPE_FDX);
    device->flags = (ops->read != NULL ? MR_FLAG_RDONLY : 0) |
                    (ops->write != NULL ? MR_FLAG_WRONLY : 0) |
                    (ops->aread != NULL ? MR_FLAG_ARDONLY : 0) |
                    (ops->awrite != NULL ? MR_FLAG_AWRONLY : 0);
    device->ref_count = 0;
    device->lock = 0;
    device->ops = ops;
    device->driver = driver;
    mr_list_init(&device->elist);

    /* Critical section enter */
    mr_critical_enter();

    /* Register the device to the target */
    int ret = _device_register_iter(device, path, to_device);

    /* Critical section exit */
    mr_critical_exit();

    return ret;
}

static int _device_unregister(struct mr_device *device)
{
    struct mr_device *parent = device->parent;
    int ret;

    /* Critical section enter */
    mr_critical_enter();

    /* Unregister the device only when there are no more references to it */
    if (device->ref_count > 0)
    {
        ret = MR_EBUSY;
        goto _exit;
    }

    /* Try to detach the device from its parent */
    mr_list_remove(&device->list);

    /* Notify the parent device that the child device will be detached */
    if (parent->ops->detach != NULL)
    {
        ret = parent->ops->detach(parent, device);
        if (ret < 0)
        {
            /* Detach failed, revert the registration */
            mr_list_insert_after(&parent->clist, &device->list);
            goto _exit;
        }
    }

    /* Unregister the device */
    device->magic = 0;
    device->parent = NULL;
    ret = MR_EOK;

_exit:
    /* Critical section exit */
    mr_critical_exit();
    return ret;
}

static int _device_isr(struct mr_device *device, uint32_t event, void *args)
{
    int ret;

    /* Critical section enter */
    mr_critical_enter();

    /* If the device has no references, cannot handle the ISR */
    if (device->ref_count == 0)
    {
        ret = MR_EPERM;
        goto _exit;
    }

    /* Set the device ISR flag */
    MR_BIT_SET(device->flags, _FLAG_IN_ISR);

    /* Call the device ISR */
    if (device->ops->isr != NULL)
    {
        ret = device->ops->isr(device, event, args);
        if (ret < 0)
        {
            goto _exit;
        }
    }

    /* Release the device based on event */
    uint32_t mask = (event & MR_EVENT_RD_COMPLETE) ? _MR_OPERATE_MASK_RD : 0;
    mask |= (event & MR_EVENT_WR_COMPLETE) ? _MR_OPERATE_MASK_WR : 0;
    _device_release(device, mask);

    /* Call the event handler */
    _device_event_handler(device, event, &ret);
    ret = MR_EOK;

_exit:
    /* Clear the device ISR flag */
    MR_BIT_CLR(device->flags, _FLAG_IN_ISR);

    /* Critical section exit */
    mr_critical_exit();
    return ret;
}

static int _device_open(const char *path, uint32_t flags)
{
    /* Find the device */
    struct mr_device *device = _device_find(path);
    if (device == NULL)
    {
        return MR_ENOENT;
    }

    /* Allocate a descriptor */
    int descriptor = _descriptor_allocate(device, flags);
    if (descriptor < 0)
    {
        return descriptor;
    }

    /* Take the device */
    int ret = _device_take(device, descriptor, _MR_OPERATE_MASK_ALL);
    if (ret < 0)
    {
        _descriptor_free(descriptor);
        return ret;
    }

    /* The Device will only be opened the first time it is accessed */
    if ((device->ref_count == 0) && (device->ops->open != NULL))
    {
        ret = device->ops->open(device);
        if (ret < 0)
        {
            _descriptor_free(descriptor);
            goto _exit;
        }
    }

    /* Increment the reference count */
    device->ref_count++;

    /* Return the descriptor */
    ret = descriptor;

_exit:
    /* Release the device */
    _device_release(device, _MR_OPERATE_MASK_ALL);
    return ret;
}

static int _device_close(int descriptor)
{
    /* Get the device */
    struct mr_device *device = _descriptor_map[descriptor].device;
    if (device == NULL)
    {
        return MR_EINVAL;
    }

    /* Take the device */
    int ret = _device_take(device, descriptor, _MR_OPERATE_MASK_ALL);
    if (ret < 0)
    {
        return ret;
    }

    /* The Device will only be closed the last time it is accessed */
    if ((device->ref_count == 1) && (device->ops->close != NULL))
    {
        ret = device->ops->close(device);
        if (ret < 0)
        {
            goto _exit;
        }
    }

    /* Decrement the reference count */
    device->ref_count--;

    /* Destroy events associated with the descriptor */
    _device_event_destroy_all(device, descriptor);

    /* Free the descriptor */
    _descriptor_free(descriptor);

_exit:
    /* Release the device */
    _device_release(device, _MR_OPERATE_MASK_ALL);
    return ret;
}

static ssize_t _device_read(int descriptor, void *buf, size_t count)
{
    /* Find the device */
    struct mr_device *device = _descriptor_map[descriptor].device;
    if (device == NULL)
    {
        return MR_EINVAL;
    }

    /* Take the device */
    ssize_t ret = _device_take(device, descriptor, _MR_OPERATE_MASK_RD);
    if (ret < 0)
    {
        return ret;
    }

    /* Get the position */
    int pos = _descriptor_map[descriptor].pos;

    /* Async or sync read */
    if (_descriptor_flags_is_valid(descriptor, MR_FLAG_ARDONLY) == true)
    {
        /* Async read */
        ret = device->ops->aread(device, pos, buf, count);
        if ((ret == 0) && (count > 0))
        {
            /* If the operation is successful, the device will not be released
             * until the operation is finished */
            return ret;
        }
    } else if (_descriptor_flags_is_valid(descriptor, MR_FLAG_RDONLY) == true)
    {
        /* Sync read */
        ret = device->ops->read(device, pos, buf, count);
    } else
    {
        ret = MR_EACCES;
    }

    /* Release the device */
    _device_release(device, _MR_OPERATE_MASK_RD);
    return ret;
}

static ssize_t _device_write(int descriptor, const void *buf, size_t count)
{
    /* Find the device */
    struct mr_device *device = _descriptor_map[descriptor].device;
    if (device == NULL)
    {
        return MR_EINVAL;
    }

    /* Take the device */
    ssize_t ret = _device_take(device, descriptor, _MR_OPERATE_MASK_WR);
    if (ret < 0)
    {
        return ret;
    }

    /* Get the position */
    int pos = _descriptor_map[descriptor].pos;

    /* Async or sync writes */
    if (_descriptor_flags_is_valid(descriptor, MR_FLAG_AWRONLY) == true)
    {
        /* Async write */
        ret = device->ops->awrite(device, pos, buf, count);
        if ((ret == 0) && (count > 0))
        {
            /* If the operation is successful, the device will not be released
             * until the operation is finished */
            return ret;
        }
    } else if (_descriptor_flags_is_valid(descriptor, MR_FLAG_WRONLY) == true)
    {
        /* Sync write */
        ret = device->ops->write(device, pos, buf, count);
    } else
    {
        ret = MR_EACCES;
    }

    /* Release the device */
    _device_release(device, _MR_OPERATE_MASK_WR);
    return ret;
}

static int _device_ioctl(int descriptor, int cmd, void *args)
{
    /* Find the device */
    struct mr_device *device = _descriptor_map[descriptor].device;
    if (device == NULL)
    {
        return MR_EINVAL;
    }

    /* Take the device */
    int ret = _device_take(device, descriptor, _MR_OPERATE_MASK_ALL);
    if (ret < 0)
    {
        return ret;
    }

    /* Get the position */
    int pos = _descriptor_map[descriptor].pos;

    /* Control the device */
    switch (cmd)
    {
        case MR_CTRL_SET(MR_CMD_POS):
        {
            /* Check the argument */
            if (args == NULL)
            {
                ret = MR_EINVAL;
                goto _exit;
            }

            /* Set the position */
            _descriptor_map[descriptor].pos = *(int *)args;
            ret = MR_EOK;
            break;
        }
        case MR_CTRL_GET(MR_CMD_POS):
        {
            /* Check the argument */
            if (args == NULL)
            {
                ret = MR_EINVAL;
                goto _exit;
            }

            /* Get the position */
            *(int *)args = pos;
            ret = MR_EOK;
            break;
        }
        case MR_CTRL_NEW(MR_CMD_EVENT):
        {
            /* Check the argument */
            if (args == NULL)
            {
                ret = MR_EINVAL;
                goto _exit;
            }

            /* Create the event */
            ret = _device_event_create(device, descriptor, args);
            break;
        }
        case MR_CTRL_DEL(MR_CMD_EVENT):
        {
            /* Check the argument */
            if (args == NULL)
            {
                ret = MR_EINVAL;
                goto _exit;
            }

            /* Destroy the event */
            ret = _device_event_destroy(device, descriptor, args);
            break;
        }
        default:
        {
            /* CMD is defined by the device */
            ret = device->ops->ioctl(device, pos, cmd, args);
            break;
        }
    }

_exit:
    /* Release the device */
    _device_release(device, _MR_OPERATE_MASK_ALL);
    return ret;
}

/**
 * @brief This function gets the descriptor map.
 *
 * @param descriptor_map The descriptor map.
 *
 * @return The count of the descriptor map.
 */
size_t _mr_descriptor_map_get(struct mr_descriptor **descriptor_map)
{
    MR_ASSERT(descriptor_map != NULL);

    *descriptor_map = _descriptor_map;
    return sizeof(_descriptor_map) / sizeof(struct mr_descriptor);
}

/**
 * @brief This function register a device to a path.
 *
 * @param device The device.
 * @param path The path of the device.
 * @param type The type of the device.
 * @param ops The operations of the device.
 * @param driver The driver of the device.
 * @param to_path The path of the registration target.
 *
 * @return The error code.
 */
int mr_device_register_to(struct mr_device *device, const char *path,
                          uint32_t type, struct mr_device_ops *ops,
                          const void *driver, const char *to_path)
{
    MR_ASSERT((device != NULL) && (device->magic != _MAGIC_NUMBER));
    MR_ASSERT(path != NULL);

    /* Register the device to the target path */
    int ret = _device_register(device, path, type, ops, driver, to_path);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return error code */
    return ret;

_exit:
    MR_LOG_E("Register '%s' to '%s' failed: %s.\r\n", path, to_path,
             mr_strerror(ret));
    return ret;
}

/**
 * @brief This function register a device.
 *
 * @param device The device.
 * @param path The path of the device.
 * @param type The type of the device.
 * @param ops The operations of the device.
 * @param driver The driver of the device.
 *
 * @return The error code.
 */
int mr_device_register(struct mr_device *device, const char *path,
                       uint32_t type, struct mr_device_ops *ops,
                       const void *driver)
{
    MR_ASSERT((device != NULL) && (device->magic != _MAGIC_NUMBER));
    MR_ASSERT(path != NULL);

    /* Register the device */
    int ret = _device_register(device, path, type, ops, driver, NULL);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return error code */
    return ret;

_exit:
    MR_LOG_E("Register '%s' failed: %s.\r\n", path, mr_strerror(ret));
    return ret;
}

/**
 * @brief This function unregister a device.
 *
 * @param device The device.
 *
 * @return The error code.
 */
int mr_device_unregister(struct mr_device *device)
{
    MR_ASSERT((device != NULL) && (device->magic == _MAGIC_NUMBER));

    /* Unregister the device */
    int ret = _device_unregister(device);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return error code */
    return ret;

_exit:
    MR_LOG_E("Unregister '%s' failed: %s.\r\n", device->name, mr_strerror(ret));
    return ret;
}

/**
 * @brief This function ISR of a device.
 *
 * @param device The device.
 * @param event The event of the ISR.
 * @param args The arguments.
 *
 * @return The error code.
 */
int mr_device_isr(struct mr_device *device, uint32_t event, void *args)
{
    MR_ASSERT((device != NULL) && (device->magic == _MAGIC_NUMBER));

    /* Call the device ISR */
    return _device_isr(device, event, args);
}

/**
 * @brief This function open a device.
 *
 * @param path The path of the device.
 * @param flags The flags of the device.
 *
 * @return The descriptor of the device on success, otherwise an error code.
 */
int mr_device_open(const char *path, uint32_t flags)
{
    MR_ASSERT(path != NULL);
    MR_ASSERT(flags != 0);

    /* Open the device */
    int ret = _device_open(path, flags);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return the descriptor */
    return ret;

_exit:
    MR_LOG_E("Open '%s' failed: %s.\r\n", path, mr_strerror(ret));
    return ret;
}

/**
 * @brief This function close a device.
 *
 * @param descriptor The descriptor of the device.
 *
 * @return The error code.
 */
int mr_device_close(int descriptor)
{
    /* Check if the descriptor is valid */
    if (_descriptor_is_valid(descriptor) == false)
    {
        return MR_EINVAL;
    }

    /* Close the device */
    int ret = _device_close(descriptor);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return ok */
    return ret;

_exit:
    MR_LOG_E("Close '%d' failed: %s.\r\n", descriptor, mr_strerror(ret));
    return ret;
}

/**
 * @brief This function read data from a device.
 *
 * @param descriptor The descriptor of the device.
 * @param buf The buffer to be read.
 * @param count The count of read.
 *
 * @return The size of the actual read, otherwise an error code.
 *
 * @note If the async operation is successful, the device will not be released
 *       until the operation is complete.
 */
ssize_t mr_device_read(int descriptor, void *buf, size_t count)
{
    /* Check if the descriptor is valid */
    if (_descriptor_is_valid(descriptor) == false)
    {
        return MR_EINVAL;
    }

    /* Read the device */
    ssize_t ret = _device_read(descriptor, buf, count);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return actual read size */
    return ret;

_exit:
    MR_LOG_E("Read '%d' failed: %s.\r\n", descriptor, mr_strerror((int)ret));
    return ret;
}

/**
 * @brief This function write data to a device.
 *
 * @param descriptor The descriptor of the device.
 * @param buf The buffer to be written.
 * @param count The count of write.
 *
 * @return The size of the actual write, otherwise an error code.
 *
 * @note If the async operation is successful, the device will not be released
 *       until the operation is complete.
 */
ssize_t mr_device_write(int descriptor, const void *buf, size_t count)
{
    /* Check if the descriptor is valid */
    if (_descriptor_is_valid(descriptor) == false)
    {
        return MR_EINVAL;
    }

    /* Write the device */
    ssize_t ret = _device_write(descriptor, buf, count);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return actual write size */
    return ret;

_exit:
    MR_LOG_E("Write '%d' failed: %s.\r\n", descriptor, mr_strerror((int)ret));
    return ret;
}

/**
 * @brief This function ioctl a device.
 *
 * @param descriptor The descriptor of the device.
 * @param cmd The command of the ioctl.
 * @param args The arguments of the ioctl.
 *
 * @return The size of the actual ioctl, otherwise an error code.
 */
int mr_device_ioctl(int descriptor, int cmd, void *args)
{
    /* Check if the descriptor is valid */
    if (_descriptor_is_valid(descriptor) == false)
    {
        return MR_EINVAL;
    }

    /* Ioctl the device */
    int ret = _device_ioctl(descriptor, cmd, args);
    if (ret < 0)
    {
        goto _exit;
    }

    /* Return actual ioctl size */
    return ret;

_exit:
    MR_LOG_E("Ioctl '%d' failed: %s.\r\n", descriptor, mr_strerror(ret));
    return ret;
}
