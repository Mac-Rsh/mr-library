/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "include/mr_api.h"

#define MR_ROOT_DEV_NAME "dev"

static struct mr_dev root_dev =
    {
        MR_MAGIC_NUMBER,
        MR_ROOT_DEV_NAME,
        Mr_Dev_Type_Root,
        MR_NULL,
        {&root_dev.list, &root_dev.list},
        {&root_dev.clist, &root_dev.clist}
    };

static int dev_is_root(struct mr_dev *dev)
{
    return (int)dev->type == Mr_Dev_Type_Root;
}

static struct mr_dev *dev_find_child(struct mr_dev *parent, const char *name)
{
    /* Disable interrupt */
    mr_interrupt_disable();

    /* Find the child device */
    struct mr_list *list = MR_NULL;
    for (list = parent->clist.next; list != &parent->clist; list = list->next)
    {
        struct mr_dev *dev = (struct mr_dev *)mr_container_of(list, struct mr_dev, list);
        if (strncmp(name, dev->name, MR_CFG_NAME_MAX) == 0)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return dev;
        }
    }

    /* Enable interrupt */
    mr_interrupt_enable();
    return MR_NULL;
}

static int dev_register_child(struct mr_dev *parent, struct mr_dev *child, const char *name)
{
    /* Check whether the device with the same name exists */
    if (dev_find_child(parent, name) != MR_NULL)
    {
        return MR_EEXIST;
    }

    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the device into the child list */
    child->magic = MR_MAGIC_NUMBER;
    strncpy(child->name, name, MR_CFG_NAME_MAX);
    child->parent = parent;
    mr_list_insert_before(&parent->clist, &child->list);

    /* Enable interrupt */
    mr_interrupt_enable();
    return MR_EOK;
}

static const char *dev_clear_path(const char *path)
{
    /* Skip the leading '/' */
    if (*path == '/')
    {
        path++;
    }

    /* Skip the leading 'dev/'(root path) */
    if (strncmp(path, MR_ROOT_DEV_NAME"/", sizeof(MR_ROOT_DEV_NAME"/") - 1) == 0)
    {
        path += sizeof(MR_ROOT_DEV_NAME"/") - 1;
    }
    return path;
}

static int dev_register_by_path(struct mr_dev *parent, struct mr_dev *dev, const char *path)
{
    char parent_name[MR_CFG_NAME_MAX + 1] = {0};
    char *parent_path = MR_NULL;

    /* Clear the path */
    path = dev_clear_path(path);

    /* Check whether the parent exists */
    parent_path = strchr(path, '/');
    if (parent_path != MR_NULL)
    {
        /* Get the parent name */
        size_t len = (parent_path - path) > MR_CFG_NAME_MAX ? MR_CFG_NAME_MAX : (parent_path - path);
        strncpy(parent_name, path, len);
        parent_name[len] = '\0';

        /* Find the parent */
        parent = dev_find_child(parent, parent_name);
        if (parent == MR_NULL)
        {
            return MR_EINVAL;
        }

        /* Continue iterating */
        return dev_register_by_path(parent, dev, parent_path);
    } else
    {
        /* Register the child device to the parent */
        return dev_register_child(parent, dev, path);
    }
}

static struct mr_dev *dev_find_by_path(struct mr_dev *parent, const char *path)
{
    char parent_name[MR_CFG_NAME_MAX + 1] = {0};
    char *parent_path = MR_NULL;

    /* Clear the path */
    path = dev_clear_path(path);

    /* Check whether the parent exists */
    parent_path = strchr(path, '/');
    if (parent_path != MR_NULL)
    {
        /* Get the parent name */
        size_t len = (parent_path - path) > MR_CFG_NAME_MAX ? MR_CFG_NAME_MAX : (parent_path - path);
        strncpy(parent_name, path, len);
        parent_name[len] = '\0';

        parent = dev_find_child(parent, parent_name);
        if (parent == MR_NULL)
        {
            return MR_NULL;
        }
        return dev_find_by_path(parent, parent_path);
    } else
    {
        return dev_find_child(parent, path);
    }
}

#ifdef MR_USING_RDWR_CTL
static int dev_lock_take(struct mr_dev *dev, int take, int set)
{
    /* Continue iterating until reach the root device */
    if (dev_is_root(dev->parent) != MR_TRUE)
    {
        int ret = dev_lock_take(dev->parent, take, set);
        if (ret != MR_EOK)
        {
            return ret;
        }
    }

    /* Check whether the device is taken */
    if (dev->lflags & take)
    {
        return MR_EBUSY;
    }

    /* Take the device */
    mr_bits_set(dev->lflags, set);
    return MR_EOK;
}

static void dev_lock_release(struct mr_dev *dev, int release)
{
    /* Continue iterating until reach the root device */
    if (dev_is_root(dev->parent) != MR_TRUE)
    {
        dev_lock_release(dev->parent, release);
    }

    /* Release the device */
    mr_bits_clr(dev->lflags, release);
}
#endif /* MR_USING_RDWR_CTL */

static int dev_register(struct mr_dev *dev, const char *path)
{
    /* Register the device to the root device */
    return dev_register_by_path(&root_dev, dev, path);
}

static struct mr_dev *dev_find(const char *path)
{
    /* Find the device from the root device */
    return dev_find_by_path(&root_dev, path);
}

static int dev_open(struct mr_dev *dev, int oflags)
{
#ifdef MR_USING_RDWR_CTL
    if (mr_bits_is_set(dev->sflags, oflags) != MR_ENABLE)
    {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Check whether the device is opened */
    if (dev->ref_count == 0)
    {
        /* Continue iterating until reach the root device */
        if (dev_is_root(dev->parent) != MR_TRUE)
        {
            int ret = dev_open(dev->parent, oflags);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }

        /* Open the device */
        if (dev->ops->open != MR_NULL)
        {
            int ret = dev->ops->open(dev);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }
    }
#ifdef MR_USING_RDWR_CTL
    else if (mr_bits_is_set(dev->sflags, MR_SFLAG_ONLY) == MR_ENABLE)
    {
        return MR_EBUSY;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Increase the reference count */
    dev->ref_count++;
    return MR_EOK;
}

static int dev_close(struct mr_dev *dev)
{
    /* Decrease the reference count */
    dev->ref_count--;

    /* Check whether the device needs to be closed */
    if (dev->ref_count == 0)
    {
        /* Continue iterating until reach the root device */
        if (dev_is_root(dev->parent) != MR_TRUE)
        {
            int ret = dev_close(dev->parent);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }

        /* Close the device */
        if (dev->ops->close != MR_NULL)
        {
            return dev->ops->close(dev);
        }
    }
    return MR_EOK;
}

static ssize_t dev_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
#ifdef MR_USING_RDWR_CTL
    do
    {
        /* Disable interrupt */
        mr_interrupt_disable();
        int ret = dev_lock_take(dev, (MR_LFLAG_RD | MR_LFLAG_SLEEP), MR_LFLAG_RD);
        if (ret != MR_EOK)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return ret;
        }
        /* Enable interrupt */
        mr_interrupt_enable();
    } while (0);
#endif /* MR_USING_RDWR_CTL */

    /* Read buffer from the device */
    ssize_t ret = dev->ops->read(dev, off, buf, size, async);

#ifdef MR_USING_RDWR_CTL
    dev_lock_release(dev, MR_LFLAG_RD);
#endif /* MR_USING_RDWR_CTL */
    return ret;
}

static ssize_t dev_write(struct mr_dev *dev, int offset, const void *buf, size_t size, int async)
{
#ifdef MR_USING_RDWR_CTL
    do
    {
        /* Disable interrupt */
        mr_interrupt_disable();
        int ret = dev_lock_take(dev,
                                (MR_LFLAG_WR | MR_LFLAG_SLEEP | (async == MR_SYNC ? MR_LFLAG_NONBLOCK : 0)),
                                MR_LFLAG_WR);
        if (ret != MR_EOK)
        {
            /* Enable interrupt */
            mr_interrupt_enable();
            return ret;
        }
        /* Enable interrupt */
        mr_interrupt_enable();
    } while (0);
#endif /* MR_USING_RDWR_CTL */

    /* Write buffer to the device */
    ssize_t ret = dev->ops->write(dev, offset, buf, size, async);

#ifdef MR_USING_RDWR_CTL
    dev_lock_release(dev, MR_LFLAG_WR);
    if ((async == MR_ASYNC) && (ret != 0))
    {
        /* Disable interrupt */
        mr_interrupt_disable();
        dev_lock_take(dev, 0, MR_LFLAG_NONBLOCK);
        /* Enable interrupt */
        mr_interrupt_enable();
    }
#endif /* MR_USING_RDWR_CTL */
    return ret;
}

static int dev_ioctl(struct mr_dev *dev, int desc, int off, int cmd, void *args)
{
    /* Check whether the device has an ioctl function */
    if (dev->ops->ioctl == MR_NULL)
    {
        return MR_ENOTSUP;
    }

    switch (cmd)
    {
        case MR_CTL_SET_RD_CALL:
        {
            dev->rd_call.desc = desc;
            dev->rd_call.call = (int (*)(int desc, void *args))args;
            return MR_EOK;
        }
        case MR_CTL_SET_WR_CALL:
        {
            dev->wr_call.desc = desc;
            dev->wr_call.call = (int (*)(int desc, void *args))args;
            return MR_EOK;
        }

        case MR_CTL_GET_RD_CALL:
        {
            if (args != MR_NULL)
            {
                *(int (**)(int desc, void *args))args = dev->rd_call.call;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_GET_WR_CALL:
        {
            if (args != MR_NULL)
            {
                *(int (**)(int desc, void *args))args = dev->wr_call.call;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
#ifdef MR_USING_RDWR_CTL
            do
            {
                /* Disable interrupt */
                mr_interrupt_disable();
                int ret = dev_lock_take(dev, (MR_LFLAG_RDWR | MR_LFLAG_SLEEP | MR_LFLAG_NONBLOCK), MR_LFLAG_RDWR);
                if (ret != MR_EOK)
                {
                    /* Enable interrupt */
                    mr_interrupt_enable();
                    return ret;
                }
                /* Enable interrupt */
                mr_interrupt_enable();
            } while (0);
#endif /* MR_USING_RDWR_CTL */

            /* I/O control to the device */
            int ret = dev->ops->ioctl(dev, off, cmd, args);

#ifdef MR_USING_RDWR_CTL
            dev_lock_release(dev, MR_LFLAG_RDWR);
#endif /* MR_USING_RDWR_CTL */
            return ret;
        }
    }
}

/**
 * @brief This function register a device.
 *
 * @param dev The device.
 * @param path The path of the device.
 * @param type The type of the device.
 * @param sflags The support flags of the device.
 * @param ops The operations of the device.
 * @param drv The driver of the device.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_dev_register(struct mr_dev *dev,
                    const char *path,
                    int type,
                    int sflags,
                    struct mr_dev_ops *ops,
                    struct mr_drv *drv)
{
    static struct mr_dev_ops null_ops = {0};

    mr_assert(dev != MR_NULL);
    mr_assert(dev->magic != MR_MAGIC_NUMBER);
    mr_assert(path != MR_NULL);
    mr_assert(type != Mr_Dev_Type_Root);
    mr_assert((ops != MR_NULL) || (sflags == MR_SFLAG_NONRDWR));
    mr_assert((ops->read != MR_NULL) || (mr_bits_is_set(sflags, MR_SFLAG_RDONLY) == MR_DISABLE));
    mr_assert((ops->write != MR_NULL) || (mr_bits_is_set(sflags, MR_SFLAG_WRONLY) == MR_DISABLE));
    mr_assert((drv != MR_NULL) || (sflags & MR_SFLAG_NONDRV));
    mr_assert((drv == MR_NULL) || (drv->type == type));

    /* Initialize the fields */
    dev->magic = 0;
    memset(dev->name, '\0', MR_CFG_NAME_MAX);
    dev->parent = MR_NULL;
    mr_list_init(&dev->list);
    mr_list_init(&dev->clist);
    dev->type = type;
    dev->ref_count = 0;
#ifdef MR_USING_RDWR_CTL
    dev->sflags = sflags;
    dev->lflags = 0;
#endif /* MR_USING_RDWR_CTL */
    dev->rd_call.desc = -1;
    dev->rd_call.call = MR_NULL;
    dev->wr_call.desc = -1;
    dev->wr_call.call = MR_NULL;
    dev->ops = (ops != MR_NULL) ? ops : &null_ops;
    dev->drv = drv;

    /* Register the device */
    return dev_register(dev, path);
}

/**
 * @brief This function handle device interrupt.
 *
 * @param dev The device to be handle.
 * @param event The event to be handle.
 * @param args The arguments of the event.
 */
int mr_dev_isr(struct mr_dev *dev, int event, void *args)
{
    mr_assert(dev != MR_NULL);

    /* Check whether the device is opened */
    if (dev->ref_count == 0)
    {
        return MR_EINVAL;
    }

    /* Check whether the device has ISR */
    if (dev->ops->isr == MR_NULL)
    {
        return MR_ENOTSUP;
    }

    /* Call the device ISR */
    ssize_t ret = dev->ops->isr(dev, event, args);
    if (ret < 0)
    {
        return (int)ret;
    }

    /* Handle the event */
    switch (event & MR_ISR_MASK)
    {
        case MR_ISR_RD:
        {
            if (dev->rd_call.call != MR_NULL)
            {
                return dev->rd_call.call(dev->rd_call.desc, &ret);
            }
            return MR_EOK;
        }

        case MR_ISR_WR:
        {
            if (ret != MR_EOK)
            {
                return MR_EBUSY;
            }
#ifdef MR_USING_RDWR_CTL
            dev_lock_release(dev, MR_LFLAG_NONBLOCK);
#endif /* MR_USING_RDWR_CTL */
            if (dev->wr_call.call != MR_NULL)
            {
                return dev->wr_call.call(dev->wr_call.desc, &ret);
            }
            return MR_EOK;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function get the path of the device.
 *
 * @param dev The device.
 * @param buf The buffer to store the path.
 * @param bufsz The size of the buffer.
 *
 * @return The length of the path on success, otherwise an error code.
 */
int mr_dev_get_path(struct mr_dev *dev, char *buf, size_t bufsz)
{
    int ret = 0;

    mr_assert(dev != MR_NULL);
    mr_assert((buf != MR_NULL) || (bufsz == 0));

    /* Continue to get the path of the parent device */
    if (dev->parent != MR_NULL)
    {
        ret = mr_dev_get_path(dev->parent, buf, bufsz);
        if (ret < 0)
        {
            return ret;
        }
    }

    /* Check whether the buffer is enough */
    if ((bufsz - ret) <= (strlen(dev->name) + 1))
    {
        return MR_ENOMEM;
    }
    ret += snprintf(buf + ret, bufsz - ret, "/%s", dev->name);
    return ret;
}

/**
 * @brief Device descriptor structure.
 */
static struct mr_desc
{
    struct mr_dev *dev;                                             /* Device */
    int oflags;                                                     /* Open flags */
    int offset;                                                     /* Offset */
#ifndef MR_CFG_DESC_MAX
#define MR_CFG_DESC_MAX                 (32)
#endif /* MR_CFG_DESC_MAX */
} desc_map[MR_CFG_DESC_MAX] = {0};

#define desc_of(desc)                   (desc_map[(desc)])
#define desc_is_valid(desc)             (((desc) >= 0 && (desc) < MR_CFG_DESC_MAX) && ((desc_of(desc).dev) != MR_NULL))

static int desc_allocate(const char *path)
{
    int desc = -1;
    int i = 0;

    /* Find a free descriptor */
    for (i = 0; i < MR_CFG_DESC_MAX; i++)
    {
        if (desc_of(i).dev == MR_NULL)
        {
            desc = i;
            break;
        }
    }
    if (desc < 0)
    {
        return MR_ENOMEM;
    }

    /* Find the device */
    struct mr_dev *dev = dev_find(path);
    if (dev == MR_NULL)
    {
        return MR_ENOTFOUND;
    }

    /* Initialize the fields */
    desc_of(desc).dev = dev;
    desc_of(desc).offset = -1;
    desc_of(desc).oflags = MR_OFLAG_CLOSED;
    return desc;
}

static void desc_free(int desc)
{
    if (desc >= 0 && desc < MR_CFG_DESC_MAX)
    {
        desc_of(desc).dev = MR_NULL;
        desc_of(desc).oflags = MR_OFLAG_CLOSED;
        desc_of(desc).offset = -1;
    }
}

/**
 * @brief This function open a device.
 *
 * @param path The path of the device.
 * @param oflags The open flags of the device.
 *
 * @return The descriptor of the device, otherwise an error code.
 */
int mr_dev_open(const char *path, int oflags)
{
    mr_assert(path != MR_NULL);
    mr_assert(oflags != MR_OFLAG_CLOSED);

    /* Allocate a descriptor */
    int desc = desc_allocate(path);
    if (desc < 0)
    {
        return desc;
    }

    /* Open the device */
    int ret = dev_open(desc_of(desc).dev, oflags);
    if (ret != MR_EOK)
    {
        desc_free(desc);
        return ret;
    }

    /* Initialize the open flags */
    desc_of(desc).oflags = oflags;
    return desc;
}

/**
 * @brief This function close a device.
 *
 * @param desc The descriptor of the device.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_dev_close(int desc)
{
    mr_assert(desc_is_valid(desc));

    /* Close the device */
    int ret = dev_close(desc_of(desc).dev);
    if (ret != MR_EOK)
    {
        return ret;
    }

    /* Free the descriptor */
    desc_free(desc);
    return MR_EOK;
}

/**
 * @brief This function read a device.
 *
 * @param desc The descriptor of the device.
 * @param buf The buf buffer to be read.
 * @param size The size of read.
 *
 * @return The size of the actual read, otherwise an error code.
 */
ssize_t mr_dev_read(int desc, void *buf, size_t size)
{
    mr_assert(desc_is_valid(desc));
    mr_assert(buf != MR_NULL || size == 0);

#ifdef MR_USING_RDWR_CTL
    if (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_RDONLY) == MR_DISABLE)
    {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Read buffer from the device */
    return dev_read(desc_of(desc).dev,
                    desc_of(desc).offset,
                    buf,
                    size,
                    (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_NONBLOCK)));
}

/**
 * @brief This function write a device.
 *
 * @param desc The descriptor of the device.
 * @param buf The buf buffer to be written.
 * @param size The size of write.
 *
 * @return The size of the actual write, otherwise an error code.
 */
ssize_t mr_dev_write(int desc, const void *buf, size_t size)
{
    mr_assert(desc_is_valid(desc));
    mr_assert(buf != MR_NULL || size == 0);

#ifdef MR_USING_RDWR_CTL
    if (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_WRONLY) == MR_DISABLE)
    {

        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Write buffer to the device */
    return dev_write(desc_of(desc).dev,
                     desc_of(desc).offset,
                     buf,
                     size,
                     (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_NONBLOCK)));
}

/**
 * @brief This function ioctl a device.
 *
 * @param desc The descriptor of the device.
 * @param cmd The command of the device.
 * @param args The arguments of the device.
 *
 * @return The arguments of the device, otherwise an error code.
 */
int mr_dev_ioctl(int desc, int cmd, void *args)
{
    mr_assert(desc_is_valid(desc));

    switch (cmd)
    {
        case MR_CTL_SET_OFFSET:
        {
            if (args != MR_NULL)
            {
                desc_of(desc).offset = *(int *)args;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_CTL_GET_OFFSET:
        {
            if (args != MR_NULL)
            {
                *(int *)args = desc_of(desc).offset;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
            return dev_ioctl(desc_of(desc).dev, desc, desc_of(desc).offset, cmd, args);
        }
    }
}

/**
 * @brief Get the name of the device.
 *
 * @param desc The descriptor of the device.
 *
 * @return The name of the device.
 */
const char *mr_dev_get_name(int desc)
{
    mr_assert(desc_is_valid(desc));

    return desc_of(desc).dev->name;
}
