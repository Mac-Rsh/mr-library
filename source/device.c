/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "include/mr_api.h"

static struct mr_dev root_dev = {.name = "dev", .type = MR_DEV_TYPE_ROOT, .parent = MR_NULL, .list = MR_LIST_INIT(
    root_dev.list), .clist = MR_LIST_INIT(root_dev.clist)};         /**< Root device */

#ifndef MR_CFG_DESC_NUM
#define MR_CFG_DESC_NUM                 (32)
#endif /* MR_CFG_DESC_NUM */
static struct mr_dev_desc desc_map[MR_CFG_DESC_NUM] = {0};          /**< Device descriptor map */

#define DESC_OF(desc)                   (desc_map[(desc)])          /**< Descriptor of the device */
/* Check if the descriptor is valid */
#define DESC_IS_VALID(desc)             (((desc) >= 0 && (desc) < MR_CFG_DESC_NUM) && ((DESC_OF(desc).dev) != MR_NULL))
#ifdef MR_USING_DESC_CHECK
#define MR_DESC_CHECK(desc)             if (DESC_IS_VALID(desc) == MR_FALSE) { return MR_EINVAL; }
#else
#define MR_DESC_CHECK(desc)
#endif /* MR_USING_DESC_CHECK */

MR_INLINE int dev_is_root(struct mr_dev *dev)
{
    return dev->type == MR_DEV_TYPE_ROOT;
}

MR_INLINE struct mr_dev *dev_find_child(struct mr_dev *parent, const char *name)
{
    for (struct mr_list *list = parent->clist.next; list != &parent->clist; list = list->next) {
        struct mr_dev *dev = (struct mr_dev *)MR_CONTAINER_OF(list, struct mr_dev, list);
        if (strncmp(name, dev->name, MR_CFG_DEV_NAME_LEN) == 0) {
            return dev;
        }
    }
    return MR_NULL;
}

MR_INLINE int dev_register_child(struct mr_dev *parent, struct mr_dev *child, const char *name)
{
    if (dev_find_child(parent, name) != MR_NULL) {
        return MR_EEXIST;
    }

    child->magic = MR_MAGIC_NUMBER;
    strncpy(child->name, name, MR_CFG_DEV_NAME_LEN);
    child->parent = parent;
    mr_list_insert_before(&parent->clist, &child->list);
    return MR_EOK;
}

static int dev_register_by_path(struct mr_dev *parent, struct mr_dev *dev, const char *path)
{
    if (path[0] == '/') {
        path++;
    }

    /* Check for child path separator */
    const char *child_path = strchr(path, '/');
    if (child_path != MR_NULL) {
        char child_name[MR_CFG_DEV_NAME_LEN + 1] = {0};
        size_t len = MR_BOUND(child_path - path, 0, MR_CFG_DEV_NAME_LEN);

        /* Find the child device */
        strncpy(child_name, path, len);
        child_name[len] = '\0';
        struct mr_dev *child = dev_find_child(parent, child_name);
        if (child == MR_NULL) {
            return MR_ENOTFOUND;
        }

        /* Register recursively */
        return dev_register_by_path(child, dev, child_path);
    } else {
        /* Register with parent */
        return dev_register_child(parent, dev, path);
    }
}

static struct mr_dev *dev_find_by_path(struct mr_dev *parent, const char *path)
{
    if (path[0] == '/') {
        path++;
    }

    /* Check for child path separator */
    const char *child_path = strchr(path, '/');
    if (child_path != MR_NULL) {
        char child_name[MR_CFG_DEV_NAME_LEN + 1] = {0};
        size_t len = MR_BOUND(child_path - path, 0, MR_CFG_DEV_NAME_LEN);

        /* Find the child device */
        strncpy(child_name, path, len);
        child_name[len] = '\0';
        struct mr_dev *child = dev_find_child(parent, child_name);
        if (child == MR_NULL) {
            return MR_NULL;
        }

        /* Find recursively */
        return dev_find_by_path(child, child_path);
    } else {
        /* Find with parent */
        return dev_find_child(parent, path);
    }
}

#ifdef MR_USING_RDWR_CTL
static int dev_lock_take(struct mr_dev *dev, uint32_t take, uint32_t set)
{
    /* Continue iterating until reach the root device */
    if (dev_is_root(dev->parent) != MR_TRUE) {
        int ret = dev_lock_take(dev->parent, take, set);
        if (ret < 0) {
            return ret;
        }
    }

    if (dev->lock & take) {
        return MR_EBUSY;
    }
    MR_BIT_SET(dev->lock, set);
    return MR_EOK;
}

static void dev_lock_release(struct mr_dev *dev, uint32_t release)
{
    /* Continue iterating until reach the root device */
    if (dev_is_root(dev->parent) != MR_TRUE) {
        dev_lock_release(dev->parent, release);
    }

    MR_BIT_CLR(dev->lock, release);
}
#endif /* MR_USING_RDWR_CTL */

MR_INLINE struct mr_dev *dev_find(const char *path)
{
    /* Check whether the path is absolute */
    if (*path == '/') {
        path++;
        const char *next_slash = strchr(path, '/');
        if ((next_slash == MR_NULL) ||
            (strncmp(path, root_dev.name, MR_BOUND(next_slash - path, 0, MR_CFG_DEV_NAME_LEN)) !=
             0)) {
            return MR_NULL;
        }
        path += MR_BOUND(next_slash - path, 0, MR_CFG_DEV_NAME_LEN);
    }

    /* Find the device from the root device */
    return dev_find_by_path(&root_dev, path);
}

MR_INLINE int dev_register(struct mr_dev *dev, const char *path)
{
    /* Check whether the path is absolute */
    if (*path == '/') {
        path++;
        const char *next_slash = strchr(path, '/');
        if ((next_slash == MR_NULL) ||
            (strncmp(path, root_dev.name, MR_BOUND(next_slash - path, 0, MR_CFG_DEV_NAME_LEN)) !=
             0)) {
            return MR_EINVAL;
        }
        path += MR_BOUND(next_slash - path, 0, MR_CFG_DEV_NAME_LEN);
    }

    /* Register the device with the root device */
    mr_interrupt_disable();
    int ret = dev_register_by_path(&root_dev, dev, path);
    mr_interrupt_enable();
    return ret;
}

static int dev_open(struct mr_dev *dev, int flags)
{
#ifdef MR_USING_RDWR_CTL
    if (MR_BIT_IS_SET(dev->flags, flags) != MR_ENABLE) {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Check whether the device is opened */
    if (dev->ref_count == 0) {
        /* Continue iterating until reach the root device */
        if (dev_is_root(dev->parent) != MR_TRUE) {
            int ret = dev_open(dev->parent, flags);
            if (ret < 0) {
                return ret;
            }
        }

        /* Open the device */
        if (dev->ops->open != MR_NULL) {
            int ret = dev->ops->open(dev);
            if (ret < 0) {
                return ret;
            }
        }
    }

    /* Increase the reference count */
    dev->ref_count++;
    return MR_EOK;
}

static int dev_close(struct mr_dev *dev)
{
    /* Decrease the reference count */
    dev->ref_count--;

    /* Check whether the device needs to be closed */
    if (dev->ref_count == 0) {
        /* Continue iterating until reach the root device */
        if (dev_is_root(dev->parent) != MR_TRUE) {
            int ret = dev_close(dev->parent);
            if (ret < 0) {
                return ret;
            }
        }

        /* Close the device */
        if (dev->ops->close != MR_NULL) {
            return dev->ops->close(dev);
        }
    }
    return MR_EOK;
}

MR_INLINE ssize_t dev_read(struct mr_dev *dev, int position, int sync, void *buf, size_t count)
{
#ifdef MR_USING_RDWR_CTL
    do {
        mr_interrupt_disable();
        int ret = dev_lock_take(dev, (MR_LOCK_RD | MR_LOCK_SLEEP), MR_LOCK_RD);
        if (ret < 0) {
            mr_interrupt_enable();
            return ret;
        }
        mr_interrupt_enable();
    } while (0);
#endif /* MR_USING_RDWR_CTL */

    /* Update information */
    dev->sync = sync;
    dev->position = position;

    /* Read buffer from the device */
    ssize_t ret = dev->ops->read(dev, buf, count);

#ifdef MR_USING_RDWR_CTL
    dev_lock_release(dev, MR_LOCK_RD);
#endif /* MR_USING_RDWR_CTL */
    return ret;
}

MR_INLINE ssize_t dev_write(struct mr_dev *dev,
                            int position,
                            int sync,
                            const void *buf,
                            size_t count)
{
#ifdef MR_USING_RDWR_CTL
    do {
        mr_interrupt_disable();
        int ret = dev_lock_take(dev,
                                (MR_LOCK_WR |
                                 MR_LOCK_SLEEP |
                                 (sync == MR_SYNC ? MR_LOCK_NONBLOCK : 0)),
                                MR_LOCK_WR);
        if (ret < 0) {
            mr_interrupt_enable();
            return ret;
        }
        mr_interrupt_enable();
    } while (0);
#endif /* MR_USING_RDWR_CTL */

    /* Update information */
    dev->sync = sync;
    dev->position = position;

    /* Write buffer to the device */
    ssize_t ret = dev->ops->write(dev, buf, count);

#ifdef MR_USING_RDWR_CTL
    dev_lock_release(dev, MR_LOCK_WR);
    if ((sync == MR_ASYNC) && (ret > 0)) {
        mr_interrupt_disable();
        dev_lock_take(dev, 0, MR_LOCK_NONBLOCK);
        mr_interrupt_enable();
    }
#endif /* MR_USING_RDWR_CTL */
    return ret;
}

MR_INLINE int dev_ioctl(struct mr_dev *dev, int position, int sync, int cmd, void *args)
{
    if (dev->ops->ioctl == MR_NULL) {
        return MR_ENOTSUP;
    }

#ifdef MR_USING_RDWR_CTL
    do {
        /* Lock only when user -> device command */
        if (cmd) {
            mr_interrupt_disable();
            int ret = dev_lock_take(dev,
                                    (MR_LOCK_RDWR | MR_LOCK_SLEEP | MR_LOCK_NONBLOCK),
                                    MR_LOCK_RDWR);
            if (ret < 0) {
                mr_interrupt_enable();
                return ret;
            }
            mr_interrupt_enable();
        }
    } while (0);
#endif /* MR_USING_RDWR_CTL */

    /* Update information */
    dev->sync = sync;
    dev->position = position;

    /* I/O control to the device */
    int ret = dev->ops->ioctl(dev, cmd, args);

#ifdef MR_USING_RDWR_CTL
    dev_lock_release(dev, MR_LOCK_RDWR);
#endif /* MR_USING_RDWR_CTL */
    return ret;
}

MR_INLINE int desc_allocate(const char *path)
{
    int desc = -1;

    struct mr_dev *dev = dev_find(path);
    if (dev == MR_NULL) {
        return MR_ENOTFOUND;
    }

    /* Find a free descriptor */
    for (size_t i = 0; i < MR_CFG_DESC_NUM; i++) {
        if (DESC_OF(i).dev == MR_NULL) {
            desc = (int)i;
            break;
        }
    }
    if (desc < 0) {
        return MR_ENOMEM;
    }

    DESC_OF(desc).dev = dev;
    DESC_OF(desc).flags = MR_O_CLOSED;
    DESC_OF(desc).position = -1;
    DESC_OF(desc).rd_call.fn = MR_NULL;
    DESC_OF(desc).wr_call.fn = MR_NULL;
    mr_list_init(&DESC_OF(desc).rd_call.list);
    mr_list_init(&DESC_OF(desc).wr_call.list);
    return desc;
}

MR_INLINE void desc_free(int desc)
{
    if (DESC_IS_VALID(desc) == MR_TRUE) {
        DESC_OF(desc).dev = MR_NULL;
        DESC_OF(desc).flags = MR_O_CLOSED;
        DESC_OF(desc).position = -1;
        DESC_OF(desc).rd_call.fn = MR_NULL;
        DESC_OF(desc).wr_call.fn = MR_NULL;
        mr_list_remove(&DESC_OF(desc).rd_call.list);
        mr_list_remove(&DESC_OF(desc).wr_call.list);
    }
}

/**
 * @brief This function register a device.
 *
 * @param dev The device.
 * @param path The path of the device.
 * @param type The type of the device.
 * @param flags The support flags of the device.
 * @param ops The operations of the device.
 * @param drv The driver of the device.
 *
 * @return 0 on success, otherwise an error code.
 *
 * @retval -3 a node in the path is not found.
 * @retval -5 name is already exists.
 * @retval -7 path is invalid.
 */
int mr_dev_register(struct mr_dev *dev,
                    const char *path,
                    int type,
                    int flags,
                    struct mr_dev_ops *ops,
                    struct mr_drv *drv)
{
    MR_ASSERT(dev != MR_NULL);
    MR_ASSERT(dev->magic != MR_MAGIC_NUMBER);
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(type != MR_DEV_TYPE_ROOT);
    MR_ASSERT(flags >= MR_O_CLOSED);
    MR_ASSERT(ops != MR_NULL);
    MR_ASSERT((ops->read != MR_NULL) || (MR_BIT_IS_SET(flags, MR_O_RDONLY) == MR_DISABLE));
    MR_ASSERT((ops->write != MR_NULL) || (MR_BIT_IS_SET(flags, MR_O_WRONLY) == MR_DISABLE));

    /* Initialize the fields */
    dev->magic = 0;
    memset(dev->name, '\0', MR_CFG_DEV_NAME_LEN);
    dev->type = type;
    dev->flags = flags;
    dev->parent = MR_NULL;
    mr_list_init(&dev->list);
    mr_list_init(&dev->clist);
    dev->ref_count = 0;
#ifdef MR_USING_RDWR_CTL
    dev->lock = 0;
#endif /* MR_USING_RDWR_CTL */
    dev->sync = MR_SYNC;
    dev->position = -1;
    mr_list_init(&dev->rd_call_list);
    mr_list_init(&dev->wr_call_list);
    dev->ops = ops;
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
 *
 * @return 0 on success, otherwise an error code.
 *
 * @retval -6 device is closed or not supported.
 * @retval other error code.
 */
int mr_dev_isr(struct mr_dev *dev, int event, void *args)
{
    MR_ASSERT(dev != MR_NULL);
    MR_ASSERT(event >= 0);

    if (dev->ref_count == 0) {
        return MR_ENOTSUP;
    }

    if (dev->ops->isr != MR_NULL) {
        ssize_t ret = dev->ops->isr(dev, event, args);
        if (ret < 0) {
            return (int)ret;
        }
    }

    /* Call the all set callbacks */
    switch (event & MR_ISR_MASK) {
        case MR_ISR_RD: {
            for (struct mr_list *list = dev->rd_call_list.next;
                 list != &dev->rd_call_list;
                 list = list->next) {
                struct mr_dev_desc *desc = (struct mr_dev_desc *)MR_CONTAINER_OF(list,
                                                                                 struct mr_dev_desc,
                                                                                 rd_call.list);
                if (desc->rd_call.fn != MR_NULL) {
                    desc->rd_call.fn((int)(desc - &desc_map[0]), args);
                }
            }
            return MR_EOK;
        }
        case MR_ISR_WR: {
#ifdef MR_USING_RDWR_CTL
            dev_lock_release(dev, MR_LOCK_NONBLOCK);
#endif /* MR_USING_RDWR_CTL */
            for (struct mr_list *list = dev->wr_call_list.next;
                 list != &dev->wr_call_list;
                 list = list->next) {
                struct mr_dev_desc *desc = (struct mr_dev_desc *)MR_CONTAINER_OF(list,
                                                                                 struct mr_dev_desc,
                                                                                 wr_call.list);
                if (desc->wr_call.fn != MR_NULL) {
                    desc->wr_call.fn((int)(desc - &desc_map[0]), args);
                }
            }
            return MR_EOK;
        }
        default: {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function open a device.
 *
 * @param path The path of the device.
 * @param flags The open flags.
 *
 * @return The descriptor of the device on success, the support flags of the device if flags is a query,
 *         otherwise an error code.
 *
 * @retval -1 not enough descriptor or memory.
 * @retval -3 the path is not found.
 * @retval -6 not supported flags.
 * @retval other error code.
 */
int mr_dev_open(const char *path, int flags)
{
    MR_ASSERT(path != MR_NULL);
    MR_ASSERT(flags >= MR_O_CLOSED);

    /* Query device flags */
    if (flags == MR_O_QUERY) {
        struct mr_dev *dev = dev_find(path);
        if (dev == MR_NULL) {
            return MR_ENOTFOUND;
        }
        return dev->flags;
    }

    /* Allocate descriptor and open device */
    int desc = desc_allocate(path);
    if (desc < 0) {
        return desc;
    }
    int ret = dev_open(DESC_OF(desc).dev, flags);
    if (ret < 0) {
        desc_free(desc);
        return ret;
    }

    /* Set descriptor flags */
    DESC_OF(desc).flags = flags;
    return desc;
}

/**
 * @brief This function close a device.
 *
 * @param desc The descriptor of the device.
 *
 * @return 0 on success, otherwise an error code.
 *
 * @retval -7 descriptor is invalid.
 * @retval other error code.
 */
int mr_dev_close(int desc)
{
    MR_DESC_CHECK(desc);

    /* Close the device and free the descriptor */
    int ret = dev_close(DESC_OF(desc).dev);
    if (ret < 0) {
        return ret;
    }
    desc_free(desc);
    return MR_EOK;
}

/**
 * @brief This function read a device.
 *
 * @param desc The descriptor of the device.
 * @param buf The buf buffer to be read.
 * @param count The count of read.
 *
 * @return The size of the actual read, otherwise an error code.
 *
 * @retval -4 is currently accessed by another.
 * @retval -6 not supported read.
 * @retval -7 descriptor is invalid.
 * @retval other error code.
 */
ssize_t mr_dev_read(int desc, void *buf, size_t count)
{
    MR_ASSERT((buf != MR_NULL) || (count == 0));
    MR_DESC_CHECK(desc);

#ifdef MR_USING_RDWR_CTL
    if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_RDONLY) == MR_DISABLE) {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Read buffer from the device */
    return dev_read(DESC_OF(desc).dev,
                    DESC_OF(desc).position,
                    MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_NONBLOCK),
                    buf,
                    count);
}

/**
 * @brief This function write a device.
 *
 * @param desc The descriptor of the device.
 * @param buf The buf buffer to be written.
 * @param count The count of write.
 *
 * @return The size of the actual write, otherwise an error code.
 *
 * @retval -4 is currently accessed by another.
 * @retval -6 not supported write.
 * @retval -7 descriptor is invalid.
 * @retval other error code.
 */
ssize_t mr_dev_write(int desc, const void *buf, size_t count)
{
    MR_ASSERT((buf != MR_NULL) || (count == 0));
    MR_DESC_CHECK(desc);

#ifdef MR_USING_RDWR_CTL
    if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_WRONLY) == MR_DISABLE) {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTL */

    /* Write buffer to the device */
    return dev_write(DESC_OF(desc).dev,
                     DESC_OF(desc).position,
                     MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_NONBLOCK),
                     buf,
                     count);
}

/**
 * @brief This function ioctl a device.
 *
 * @param desc The descriptor of the device.
 * @param cmd The command of the device.
 * @param args The arguments of the device.
 *
 * @return The size of the actual control, otherwise an error code.
 *
 * @retval -4 is currently accessed by another.
 * @retval -6 not supported ioctl or command.
 * @retval -7 descriptor or argument is invalid.
 * @retval other error code.
 */
int mr_dev_ioctl(int desc, int cmd, void *args)
{
    MR_DESC_CHECK(desc);

    switch (cmd) {
        case MR_IOC_SPOS: {
            if (args != MR_NULL) {
                int position = *(int *)args;

                DESC_OF(desc).position = position;
                return sizeof(position);
            }
            return MR_EINVAL;
        }
        case MR_IOC_SRCB: {
#ifdef MR_USING_RDWR_CTL
            if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_RDONLY) == MR_DISABLE) {
                return MR_ENOTSUP;
            }
#endif /* MR_USING_RDWR_CTL */
            void (*fn)(int desc, void *args) =  (void (*)(int desc, void *args))args;

            /* Link or unlink the callback function */
            DESC_OF(desc).rd_call.fn = fn;
            mr_interrupt_disable();
            if (fn != MR_NULL) {
                mr_list_insert_before(&DESC_OF(desc).dev->rd_call_list,
                                      &DESC_OF(desc).rd_call.list);
            } else {
                mr_list_remove(&DESC_OF(desc).rd_call.list);
            }
            mr_interrupt_enable();
            return sizeof(fn);
        }
        case MR_IOC_SWCB: {
#ifdef MR_USING_RDWR_CTL
            if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_WRONLY) == MR_DISABLE) {
                return MR_ENOTSUP;
            }
#endif /* MR_USING_RDWR_CTL */
            void (*fn)(int desc, void *args) =  (void (*)(int desc, void *args))args;

            /* Link or unlink the callback function */
            DESC_OF(desc).wr_call.fn = fn;
            mr_interrupt_disable();
            if (fn != MR_NULL) {
                mr_list_insert_before(&DESC_OF(desc).dev->wr_call_list,
                                      &DESC_OF(desc).wr_call.list);
            } else {
                mr_list_remove(&DESC_OF(desc).wr_call.list);
            }
            mr_interrupt_enable();
            return sizeof(fn);
        }
        case MR_IOC_GPOS: {
            if (args != MR_NULL) {
                int *position = (int *)args;

                *position = DESC_OF(desc).position;
                return sizeof(position);
            }
            return MR_EINVAL;
        }
        case MR_IOC_GRCB: {
#ifdef MR_USING_RDWR_CTL
            if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_RDONLY) == MR_DISABLE) {
                return MR_ENOTSUP;
            }
#endif /* MR_USING_RDWR_CTL */
            if (args != MR_NULL) {
                *(void (**)(int desc, void *args))args = DESC_OF(desc).rd_call.fn;
                return sizeof(DESC_OF(desc).rd_call.fn);
            }
            return MR_EINVAL;
        }
        case MR_IOC_GWCB: {
#ifdef MR_USING_RDWR_CTL
            if (MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_WRONLY) == MR_DISABLE) {
                return MR_ENOTSUP;
            }
#endif /* MR_USING_RDWR_CTL */
            if (args != MR_NULL) {
                *(void (**)(int desc, void *args))args = DESC_OF(desc).wr_call.fn;
                return sizeof(DESC_OF(desc).wr_call.fn);
            }
            return MR_EINVAL;
        }
        default: {
            /* I/O control to the device */
            return dev_ioctl(DESC_OF(desc).dev,
                             DESC_OF(desc).position,
                             MR_BIT_IS_SET(DESC_OF(desc).flags, MR_O_NONBLOCK),
                             cmd,
                             args);
        }
    }
}

#if defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD)
#include "include/components/mr_msh.h"

static int dev_get_path(struct mr_dev *dev, char *buf, size_t bufsz)
{
    int ret = 0;

    /* Continue iterating until reach the null parent */
    if (dev->parent != MR_NULL) {
        ret = dev_get_path(dev->parent, buf, bufsz);
    }

    /* Check whether the buffer is enough */
    if ((bufsz - ret) <= (strlen(dev->name) + 1)) {
        return ret;
    }
    ret += snprintf(buf + ret, bufsz - ret, "/%s", dev->name);
    return ret;
}

int msh_dev_get_path(int desc, char *buf, size_t size)
{
    MR_DESC_CHECK(desc);

    return dev_get_path(DESC_OF(desc).dev, buf, size);
}

void msh_dlist_tree(struct mr_dev *parent, int level)
{
    if (level == 0) {
        mr_msh_printf("|%s %-*s", mr_strflags(parent->flags), MR_CFG_DEV_NAME_LEN, parent->name);
    } else {
        mr_msh_printf("%*s|%s %-*s",
                      level,
                      " ",
                      mr_strflags(parent->flags),
                      MR_CFG_DEV_NAME_LEN,
                      parent->name);
    }
    for (size_t i = 0; i < MR_CFG_DESC_NUM; i++) {
        if (desc_map[i].dev == parent) {
            mr_msh_printf(" [%d]", i);
        }
    }
    mr_msh_printf("\r\n");
    for (struct mr_list *child = parent->clist.next; child != &parent->clist; child = child->next) {
        struct mr_dev *dev = MR_CONTAINER_OF(child, struct mr_dev, list);
        msh_dlist_tree(dev, level + 7);
    }
}

struct mr_dev *msh_get_root(void)
{
    return &root_dev;
}

#endif /* defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD) */
