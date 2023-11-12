/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "mr_api.h"

static struct mr_dev *dev_find_from_list(struct mr_list *list, const char *name)
{
    /* Disable interrupt */
    mr_interrupt_disable();

    /* Find the device */
    struct mr_list *l = MR_NULL;
    for (l = list->next; l != list; l = l->next)
    {
        struct mr_dev *dev = (struct mr_dev *)mr_container_of(l, struct mr_dev, list);
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

static void dev_register_list(struct mr_dev *dev, const char *name, struct mr_list *list)
{
    /* Disable interrupt */
    mr_interrupt_disable();

    /* Insert the list */
    strncpy(dev->name, name, MR_CFG_NAME_MAX);
    mr_list_insert_before(list, &dev->list);

    /* Set magic */
    dev->magic = MR_MAGIC_NUMBER;

    /* Enable interrupt */
    mr_interrupt_enable();
}

#define MR_FIND                         (0)
#define MR_REGISTER                     (1)

static struct mr_dev *dev_find_or_register(const char *name, struct mr_dev *dev, int find_or_register)
{
    static struct mr_list mr_dev_list = {&mr_dev_list, &mr_dev_list};
    struct mr_dev *dev_prev = MR_NULL;
    struct mr_dev *dev_next = MR_NULL;
    size_t i = 0, j = 0;

    for (i = 0; name[i] != '\0'; i += j)
    {
        char name_cpy[MR_CFG_NAME_MAX + 1] = {0};
        for (j = 0; j <= MR_CFG_NAME_MAX; j++)
        {
            if ((name[j + i] == '\0') || (name[j + i] == '/'))
            {
                name_cpy[j] = '\0';
                if (i == 0)
                {
                    /* First register */
                    dev_prev = dev_find_from_list(&mr_dev_list, name_cpy);
                    if (find_or_register == MR_FIND)
                    {
                        if (dev_prev == MR_NULL)
                        {
                            return MR_NULL;
                        }
                        if (name[j + i] == '\0')
                        {
                            return dev_prev;
                        }
                    } else
                    {
                        if (dev_prev == MR_NULL)
                        {
                            if (name[j + i] != '/')
                            {
                                dev_register_list(dev, name_cpy, &mr_dev_list);
                                return dev;
                            }
                            return MR_NULL;
                        }
                    }
                } else
                {
                    /* Second register */
                    dev_next = dev_find_from_list(&dev_prev->slist, name_cpy);
                    if (find_or_register == MR_FIND)
                    {
                        if (dev_next == MR_NULL)
                        {
                            return MR_NULL;
                        }
                        if (name[j + i] == '\0')
                        {
                            return dev_next;
                        }
                    } else
                    {
                        if (dev_next == MR_NULL)
                        {
                            if ((name[j + i] != '/') && (dev->type == dev_prev->type))
                            {
                                dev_register_list(dev, name_cpy, &dev_prev->slist);
                                dev->link = dev_prev;
                                return dev;
                            }
                            return MR_NULL;
                        }
                        dev_prev = dev_next;
                    }
                }
                if (name[j + i] == '/')
                {
                    j++;
                }
                break;
            }
            name_cpy[j] = name[j + i];
        }
    }
    return MR_NULL;
}

#ifdef MR_USING_RDWR_CTRL
MR_INLINE int dev_lock_take(struct mr_dev *dev, int take, int set)
{
    if (dev->link != MR_NULL)
    {
        int ret = dev_lock_take(dev->link, take, set);
        if (ret != MR_EOK)
        {
            return ret;
        }
    }

    if (dev->lflags & take)
    {
        return MR_EBUSY;
    }
    mr_bits_set(dev->lflags, set);
    return MR_EOK;
}

MR_INLINE void dev_lock_release(struct mr_dev *dev, int release)
{
    if (dev->link != MR_NULL)
    {
        dev_lock_release(dev->link, release);
    }
    mr_bits_clr(dev->lflags, release);
}
#endif /* MR_USING_RDWR_CTRL */

MR_INLINE int dev_register(struct mr_dev *dev, const char *name)
{
    if (dev_find_or_register(name, dev, MR_REGISTER) != MR_NULL)
    {
        mr_log("%s register", name);
        return MR_EOK;
    }
    return MR_EINVAL;
}

MR_INLINE int dev_open(struct mr_dev *dev, uint32_t oflags)
{
#ifdef MR_USING_RDWR_CTRL
    if (mr_bits_is_set(dev->sflags, oflags) != MR_ENABLE)
    {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTRL */

    if (dev->ref_count == 0)
    {
        if (dev->link != NULL)
        {
            int ret = dev_open(dev->link, oflags);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }

        if (dev->ops->open != MR_NULL)
        {
            int ret = dev->ops->open(dev);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }
    }
#ifdef MR_USING_RDWR_CTRL
    else if (mr_bits_is_set(dev->sflags, MR_SFLAG_ONLY) == MR_ENABLE)
    {
        return MR_EBUSY;
    }
#endif /* MR_USING_RDWR_CTRL */

    dev->ref_count++;
    return MR_EOK;
}

MR_INLINE int dev_close(struct mr_dev *dev)
{
    dev->ref_count--;
    if (dev->ref_count == 0)
    {
        if (dev->link != NULL)
        {
            int ret = dev_close(dev->link);
            if (ret != MR_EOK)
            {
                return ret;
            }
        }

        if (dev->ops->close != MR_NULL)
        {
            return dev->ops->close(dev);
        }
    }
    return MR_EOK;
}

MR_INLINE ssize_t dev_read(struct mr_dev *dev, int off, void *buf, size_t size, int sync_or_async)
{
#ifdef MR_USING_RDWR_CTRL
    do
    {
        int ret = dev_lock_take(dev, MR_LFLAG_RD | MR_LFLAG_SLEEP, MR_LFLAG_RD);
        if (ret != MR_EOK)
        {
            return ret;
        }
    } while (0);
#endif /* MR_USING_RDWR_CTRL */

    /* Read buffer from the device */
    ssize_t ret = dev->ops->read(dev, off, buf, size, sync_or_async);

#ifdef MR_USING_RDWR_CTRL
    dev_lock_release(dev, MR_LFLAG_RD);
#endif /* MR_USING_RDWR_CTRL */
    return ret;
}

MR_INLINE ssize_t dev_write(struct mr_dev *dev, int offset, const void *buf, size_t size, int sync_or_async)
{
#ifdef MR_USING_RDWR_CTRL
    do
    {
        int ret = dev_lock_take(dev,
                                MR_LFLAG_WR | MR_LFLAG_SLEEP | (sync_or_async == MR_SYNC ? MR_LFLAG_NONBLOCK : 0),
                                MR_LFLAG_WR);
        if (ret != MR_EOK)
        {
            return ret;
        }
    } while (0);
#endif /* MR_USING_RDWR_CTRL */

    /* Write buffer to the device */
    ssize_t ret = dev->ops->write(dev, offset, buf, size, sync_or_async);

#ifdef MR_USING_RDWR_CTRL
    dev_lock_release(dev, MR_LFLAG_WR);
    if ((sync_or_async == MR_ASYNC) && (ret != 0))
    {
        dev_lock_take(dev, 0, MR_LFLAG_NONBLOCK);
    }
#endif /* MR_USING_RDWR_CTRL */
    return ret;
}

static int dev_ioctl(struct mr_dev *dev, int desc, int off, int cmd, void *args)
{
    if (dev->ops->ioctl == MR_NULL)
    {
        return MR_ENOTSUP;
    }

    switch (cmd)
    {
        case MR_IOCTL_SET_RD_CB:
        {
            dev->rd_cb.desc = desc;
            dev->rd_cb.cb = (int (*)(int desc, void *args))args;
            return MR_EOK;
        }
        case MR_IOCTL_SET_WR_CB:
        {
            dev->wr_cb.desc = desc;
            dev->wr_cb.cb = (int (*)(int desc, void *args))args;
            return MR_EOK;
        }

        case MR_IOCTL_GET_RD_CB:
        {
            if (args != MR_NULL)
            {
                *(int (**)(int desc, void *args))args = dev->rd_cb.cb;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_IOCTL_GET_WR_CB:
        {
            if (args != MR_NULL)
            {
                *(int (**)(int desc, void *args))args = dev->wr_cb.cb;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
#ifdef MR_USING_RDWR_CTRL
            do
            {
                int ret =
                    dev_lock_take(dev, MR_LFLAG_RDWR | MR_LFLAG_SLEEP | MR_LFLAG_NONBLOCK, MR_LFLAG_RDWR);
                if (ret != MR_EOK)
                {
                    return ret;
                }
            } while (0);
#endif /* MR_USING_RDWR_CTRL */

            /* I/O control to the device */
            int ret = dev->ops->ioctl(dev, off, cmd, args);

#ifdef MR_USING_RDWR_CTRL
            dev_lock_release(dev, MR_LFLAG_RDWR);
#endif /* MR_USING_RDWR_CTRL */
            return ret;
        }
    }
}

/**
 * @brief This function register a device.
 *
 * @param dev The device.
 * @param name The name of the device.
 * @param type The type of the device.
 * @param sflags The support flags of the device.
 * @param ops The operations of the device.
 * @param drv The driver of the device.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_dev_register(struct mr_dev *dev,
                    const char *name,
                    uint32_t type,
                    uint32_t sflags,
                    struct mr_dev_ops *ops,
                    struct mr_drv *drv)
{
    static struct mr_dev_ops null_ops = {0};

    mr_assert(dev != MR_NULL);
    mr_assert(dev->magic != MR_MAGIC_NUMBER);
    mr_assert(name != MR_NULL);
    mr_assert((ops != MR_NULL) || (sflags == MR_SFLAG_NONRDWR));
    mr_assert((drv != MR_NULL) || (sflags & MR_SFLAG_NONDRV));
    mr_assert((drv == MR_NULL) || (drv->type == type));

    /* Initialize the fields */
    dev->magic = 0;
    memset(dev->name, '\0', MR_CFG_NAME_MAX);
    mr_list_init(&dev->list);
    mr_list_init(&dev->slist);
    dev->link = MR_NULL;
    dev->type = type;
#ifdef MR_USING_RDWR_CTRL
    dev->sflags = sflags;
#endif /* MR_USING_RDWR_CTRL */
    dev->ref_count = 0;
#ifdef MR_USING_RDWR_CTRL
    dev->lflags = 0;
#endif /* MR_USING_RDWR_CTRL */

    dev->rd_cb.desc = -1;
    dev->rd_cb.cb = MR_NULL;
    dev->wr_cb.desc = -1;
    dev->wr_cb.cb = MR_NULL;

    dev->drv = drv;
    dev->ops = (ops != MR_NULL) ? ops : &null_ops;

    return dev_register(dev, name);
}

/**
 * @brief This function handle device interrupt.
 *
 * @param dev The device to be handle.
 * @param event The event to be handle.
 * @param args The arguments of the event.
 */
void mr_dev_isr(struct mr_dev *dev, int event, void *args)
{
    mr_assert(dev != MR_NULL);

    if (dev->ops->isr != MR_NULL)
    {
        ssize_t ret = dev->ops->isr(dev, event, args);
        if (ret < 0)
        {
            return;
        }

        switch (event)
        {
            case MR_ISR_EVENT_RD_INTER:
            {
                if (ret != 0)
                {
                    if (dev->rd_cb.cb != MR_NULL)
                    {
                        dev->rd_cb.cb(dev->rd_cb.desc, &ret);
                    }
                }
                return;
            }
            case MR_ISR_EVENT_WR_INTER:
            {
                if (ret == 0)
                {
#ifdef MR_USING_RDWR_CTRL
                    dev_lock_take(dev, 0, MR_LFLAG_NONBLOCK);
#endif /* MR_USING_RDWR_CTRL */
                    if (dev->wr_cb.cb != MR_NULL)
                    {
                        dev->wr_cb.cb(dev->wr_cb.desc, &ret);
                    }
                }
                return;
            }

            default:
            {
                return;
            }
        }
    }
}

/**
 * @brief Device descriptor structure.
 */
struct mr_desc
{
    struct mr_dev *dev;                                             /* Device */
    uint32_t oflags;                                                /* Open flags */
    int offset;                                                     /* Offset */
#ifndef MR_CFG_DESC_MAX
#define MR_CFG_DESC_MAX                 (32)
#endif /* MR_CFG_DESC_MAX */
} mr_desc_map[MR_CFG_DESC_MAX] = {0};

#define desc_of(desc)                   (mr_desc_map[(desc)])
#define desc_is_valid(desc)             (((desc) >= 0 && (desc) < MR_CFG_DESC_MAX) && ((desc_of(desc).dev) != MR_NULL))

static int desc_allocate(const char *name)
{
    int desc = -1;

    /* Find a free descriptor */
    for (int i = 0; i < MR_CFG_DESC_MAX; i++)
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
    struct mr_dev *dev = dev_find_or_register(name, MR_NULL, MR_FIND);
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
 * @param name The name of the device.
 * @param oflags The open flags of the device.
 *
 * @return The descriptor of the device, otherwise an error code.
 */
int mr_dev_open(const char *name, uint32_t oflags)
{
    mr_assert(name != MR_NULL);
    mr_assert(oflags != MR_OFLAG_CLOSED);

    int desc = desc_allocate(name);
    if (desc < 0)
    {
        return desc;
    }

    int ret = dev_open(desc_of(desc).dev, oflags);
    if (ret != MR_EOK)
    {
        desc_free(desc);
        return ret;
    }
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

    int ret = dev_close(desc_of(desc).dev);
    if (ret != MR_EOK)
    {
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
 * @param size The size of read.
 *
 * @return The size of the actual read, otherwise an error code.
 */
ssize_t mr_dev_read(int desc, void *buf, size_t size)
{
    mr_assert(desc_is_valid(desc));
    mr_assert(buf != MR_NULL || size == 0);

#ifdef MR_USING_RDWR_CTRL
    if (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_RDONLY) == MR_DISABLE)
    {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTRL */

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

#ifdef MR_USING_RDWR_CTRL
    if (mr_bits_is_set(desc_of(desc).oflags, MR_OFLAG_WRONLY) == MR_DISABLE)
    {
        return MR_ENOTSUP;
    }
#endif /* MR_USING_RDWR_CTRL */

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
        case MR_IOCTL_SET_OFFSET:
        {
            if (args != MR_NULL)
            {
                desc_of(desc).offset = *(int *)args;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        case MR_IOCTL_GET_OFFSET:
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
