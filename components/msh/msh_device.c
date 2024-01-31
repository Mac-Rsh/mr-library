/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-14    MacRsh       First version
 */

#include "include/components/mr_msh.h"

#if defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD)

static int msh_desc = -1;                                           /**< MSH device descriptor */

#define MSH_SET_DESC(desc)  (msh_desc = (desc))                     /**< Set MSH device descriptor */
#define MSH_GET_DESC()      (msh_desc)                              /**< Get MSH device descriptor */

int msh_dev_get_path(int desc, char *buf, size_t size);
void msh_dlist_tree(struct mr_dev *parent, int level);
struct mr_dev *msh_get_root(void);

static int msh_update_path(int desc)
{
    static char msh_path[MR_CFG_DEV_NAME_LEN * 6] = {0};
    int position;

    /* Get the path and position */
    int ret = msh_dev_get_path(desc, msh_path, sizeof(msh_path));
    if (ret < 0)
    {
        return ret;
    }
    mr_dev_ioctl(desc, MR_IOC_GPOS, &position);
    snprintf(msh_path + ret, sizeof(msh_path) - ret, "/%d", position);

    /* Update the prompt */
    MSH_SET_DESC(desc);
    mr_msh_set_prompt(msh_path);
    return MR_EOK;
}

static void msh_cmd_dlist(int argc, void *argv)
{
    msh_dlist_tree(msh_get_root(), 0);
}

static void msh_cmd_dselect(int argc, void *argv)
{
    int desc, ret;

    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-g|desc (>=0)> */
    if (strncmp(MR_MSH_GET_ARG(1), "-g", 2) == 0)
    {
        mr_msh_printf("%d\r\n", MSH_GET_DESC());
    } else
    {
        ret = sscanf(MR_MSH_GET_ARG(1), "%d", &desc);
        if ((ret < 1) || (desc < 0))
        {
            goto usage;
        }

        /* Switch to the new descriptor */
        ret = msh_update_path(desc);
        if (ret < 0)
        {
            mr_msh_printf("dselect: %d: %s\r\n", desc, mr_strerror(ret));
        }
    }
    return;

    usage:
    mr_msh_printf("usage: dselect <desc (>=0)|-g>\r\n");
}

static void msh_cmd_dopen(int argc, void *argv)
{
    const char *path;
    int flags, ret;

    if (argc < 2)
    {
        goto usage;
    }

    /* Parse <-g> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        flags = MR_O_QUERY;

        /* Parse <r|w|rw> */
    } else if (strncmp(MR_MSH_GET_ARG(2), "rw", 2) == 0)
    {
        flags = MR_O_RDWR;
    } else if (strncmp(MR_MSH_GET_ARG(2), "r", 1) == 0)
    {
        flags = MR_O_RDONLY;
    } else if (strncmp(MR_MSH_GET_ARG(2), "w", 1) == 0)
    {
        flags = MR_O_WRONLY;
    } else
    {
        goto usage;
    }
    /* Parse [-n] */
    if ((MR_MSH_GET_ARG(3) != MR_NULL) && (strncmp(MR_MSH_GET_ARG(3), "-n", 2) == 0))
    {
        flags |= MR_O_NONBLOCK;
    }

    /* Open the new device */
    path = MR_MSH_GET_ARG(1);
    ret = mr_dev_open(path, flags);
    if (ret < 0)
    {
        mr_msh_printf("dopen: %s: %s\r\n", path, mr_strerror(ret));
        return;
    }

    if (flags != MR_O_QUERY)
    {
        msh_update_path(ret);
    } else
    {
        mr_msh_printf("%s %s\r\n", mr_strflags(ret), path);
    }
    return;

    usage:
    mr_msh_printf("usage: dopen <path> <r|w|rw|-g> [-n]\r\n");
}

static void msh_cmd_dclose(int argc, void *argv)
{
    int desc, ret;

    if ((MR_MSH_GET_ARG(1) != MR_NULL) && (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse [desc] */
    if (MR_MSH_GET_ARG(1) != MR_NULL)
    {
        ret = sscanf(MR_MSH_GET_ARG(1), "%d", &desc);
        if ((ret < 1) || (desc < 0))
        {
            goto usage;
        }
    } else
    {
        /* Use the current descriptor */
        desc = MSH_GET_DESC();
    }

    /* Close the device */
    ret = mr_dev_close(desc);
    if (ret < 0)
    {
        mr_msh_printf("dclose: %d: %s\r\n", desc, mr_strerror(ret));
        return;
    }

    /* Switch to the root device */
    if (desc == MSH_GET_DESC())
    {
        MSH_SET_DESC(-1);
        mr_msh_set_prompt("/");
    }
    return;

    usage:
    mr_msh_printf("usage: dclose [desc (>=0)]\r\n");
}

MR_INLINE void msh_dioctl_pos(int argc, void *argv)
{
    int cmd = MR_IOC_SPOS, position, ret;

    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(2), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-g|position> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        cmd = -cmd;
    } else
    {
        ret = sscanf(MR_MSH_GET_ARG(2), "%d", &position);
        if (ret < 1)
        {
            goto usage;
        }
    }

    /* Get/set the position */
    mr_dev_ioctl(MSH_GET_DESC(), cmd, &position);
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", position);
    } else
    {
        /* Update the path */
        msh_update_path(MSH_GET_DESC());
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl pos <position|-g>\r\n");
}

MR_INLINE void msh_dioctl_bufsz(int argc, void *argv)
{
    int cmd, bufsz, ret;

    if ((argc < 3) || (strncmp(MR_MSH_GET_ARG(2), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-r|-w> */
    if (strncmp(MR_MSH_GET_ARG(3), "-r", 2) == 0)
    {
        cmd = MR_IOC_SRBSZ;
    } else if (strncmp(MR_MSH_GET_ARG(3), "-w", 2) == 0)
    {
        cmd = MR_IOC_SWBSZ;
    } else
    {
        goto usage;
    }

    /* Parse <-g|bufsz> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        cmd = -cmd;
    } else
    {
        ret = sscanf(MR_MSH_GET_ARG(2), "%d", &bufsz);
        if ((ret < 1) || (bufsz < 0))
        {
            goto usage;
        }
    }

    /* Set/get the buffer size */
    ret = mr_dev_ioctl(MSH_GET_DESC(), cmd, &bufsz);
    if (ret < 0)
    {
        mr_msh_printf("dioctl: bufsz: %s\r\n", mr_strerror(ret));
        return;
    }

    /* If the command is <-g>, print the buffer size */
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", bufsz);
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl bufsz <bufsz (>=0)|-g> <-r|-w>\r\n");
}

MR_INLINE void msh_dioctl_datasz(int argc, void *argv)
{
    int cmd, datasz, ret;

    /* Check the arguments and print usage */
    if ((argc < 3) || (strncmp(MR_MSH_GET_ARG(2), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-r|-w> */
    if (strncmp(MR_MSH_GET_ARG(3), "-r", 2) == 0)
    {
        cmd = MR_IOC_CRBD;
    } else if (strncmp(MR_MSH_GET_ARG(3), "-w", 2) == 0)
    {
        cmd = MR_IOC_CWBD;
    } else
    {
        goto usage;
    }

    /* Parse <-g|-c> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        cmd = -cmd;
    } else if (strncmp(MR_MSH_GET_ARG(2), "-c", 2) != 0)
    {
        goto usage;
    }

    /* Set/get the buffer size */
    ret = mr_dev_ioctl(MSH_GET_DESC(), cmd, &datasz);
    if (ret < 0)
    {
        mr_msh_printf("dioctl: datasz: %s\r\n", mr_strerror(ret));
        return;
    }

    /* If the command is <-g>, print the buffer size */
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", datasz);
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl datasz <-c|-g> <-r|-w>\r\n");
}

MR_INLINE void msh_cmd_dioctl_cfg(int argc, void *argv)
{
#ifndef MR_CFG_MSH_ARGS_NUM
#define MR_CFG_MSH_ARGS_NUM             (8)
#endif /* MR_CFG_MSH_ARGS_NUM */
    int cfg[MR_CFG_MSH_ARGS_NUM] = {0}, ret;

    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(2), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-g> */
    memset(cfg, 0, sizeof(cfg));
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        /* Get the config */
        ret = mr_dev_ioctl(MSH_GET_DESC(), MR_IOC_GCFG, cfg);
        if (ret < 0)
        {
            mr_msh_printf("dioctl: cfg: %s\r\n", mr_strerror(ret));
            return;
        }
        for (size_t i = 0; i < ret / sizeof(int); i++)
        {
            mr_msh_printf("%d ", cfg[i]);
        }
        mr_msh_printf("\r\n");
        return;
    }

    /* Parse <args> */
    for (size_t i = 2; i <= argc; i++)
    {
        ret = sscanf(MR_MSH_GET_ARG(i), "%d", &cfg[i - 2]);
        if (ret < 1)
        {
            goto usage;
        }
    }

    /* Set the config */
    ret = mr_dev_ioctl(MSH_GET_DESC(), MR_IOC_SCFG, cfg);
    if (ret < 0)
    {
        mr_msh_printf("dioctl: cfg: %s\r\n", mr_strerror(ret));
        return;
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl cfg <args|-g>\r\n");
}

MR_INLINE void msh_dioctl_cmd(int argc, void *argv)
{
    int args[MR_CFG_MSH_ARGS_NUM] = {0}, cmd, ret;

    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(2), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <other> */
    ret = sscanf(MR_MSH_GET_ARG(1), "%x", &cmd);
    if (ret < 1)
    {
        goto usage;
    }

    /* Parse <-g> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        if (cmd > 0)
        {
            cmd = -cmd;
        }
    } else
    {
        if (cmd < 0)
        {
            cmd = -cmd;
        }
    }

    /* Parse <args> */
    memset(args, 0, sizeof(args));
    if (cmd < 0)
    {
        /* Get the arguments */
        ret = mr_dev_ioctl(MSH_GET_DESC(), cmd, args);
        if (ret < 0)
        {
            mr_msh_printf("dioctl: %s: %s\r\n", MR_MSH_GET_ARG(1), mr_strerror(ret));
            return;
        }
        for (size_t i = 0; i < ret / sizeof(int); i++)
        {
            mr_msh_printf("%d ", args[i]);
        }
        mr_msh_printf("\r\n");
    } else
    {
        /* Set the arguments */
        for (size_t i = 2; i <= argc; i++)
        {
            ret = sscanf(MR_MSH_GET_ARG(i), "%d", &args[i - 2]);
            if (ret < 1)
            {
                goto usage;
            }
        }
        ret = mr_dev_ioctl(MSH_GET_DESC(), cmd, args);
        if (ret < 0)
        {
            mr_msh_printf("dioctl: %s: %s\r\n", MR_MSH_GET_ARG(1), mr_strerror(ret));
        }
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl <cmd> <args|-g>\r\n");
}

static void msh_cmd_dioctl(int argc, void *argv)
{
    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <pos|cfg|bufsz|datasz|flags> */
    if (strncmp(MR_MSH_GET_ARG(1), "pos", 6) == 0)
    {
        msh_dioctl_pos(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "cfg", 3) == 0)
    {
        msh_cmd_dioctl_cfg(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "bufsz", 5) == 0)
    {
        msh_dioctl_bufsz(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "datasz", 6) == 0)
    {
        msh_dioctl_datasz(argc, argv);
    } else
    {
        msh_dioctl_cmd(argc, argv);
    }
    return;

    usage:
    mr_msh_printf("usage: dioctl <cmd> <args|-g>\r\n");
    mr_msh_printf("       dioctl pos <position|-g>\r\n");
    mr_msh_printf("       dioctl cfg <args|-g>\r\n");
    mr_msh_printf("       dioctl bufsz <bufsz (>=0)|-g> <-r|-w>\r\n");
    mr_msh_printf("       dioctl datasz <-c|-g> <-r|-w>\r\n");
}

static void msh_printf_1(void *buf, size_t size, char format)
{
    uint8_t *b = (uint8_t *)buf;
    for (size_t i = 0; i < size; i++)
    {
        if (format == 'd')
        {
            mr_msh_printf("%d ", (int)b[i]);
        } else if (format == 'u')
        {
            mr_msh_printf("%u ", b[i]);
        } else if (format == 'c')
        {
            mr_msh_printf("%c ", b[i]);
        } else
        {
            mr_msh_printf("0x%02x ", b[i]);
        }
    }
}

static void msh_printf_2(void *buf, size_t size, char format)
{
    uint16_t *b = (uint16_t *)buf;
    for (size_t i = 0; i < (size / sizeof(*b)); i++)
    {
        if (format == 'd')
        {
            mr_msh_printf("%d ", (int)b[i]);
        } else if (format == 'u')
        {
            mr_msh_printf("%u ", b[i]);
        } else
        {
            mr_msh_printf("0x%04x ", b[i]);
        }
    }
}

static void msh_printf_4(void *buf, size_t size, char format)
{
    uint32_t *b = (uint32_t *)buf;
    for (size_t i = 0; i < (size / sizeof(*b)); i++)
    {
        if (format == 'd')
        {
            mr_msh_printf("%d ", b[i]);
        } else if (format == 'u')
        {
            mr_msh_printf("%u ", (int)b[i]);
        } else
        {
            mr_msh_printf("0x%08x ", b[i]);
        }
    }
}

static void (*msh_printf_fn[])(void *buf, size_t size, char format) =
    {
        msh_printf_1,
        msh_printf_2,
        msh_printf_4,
    };

static void msh_cmd_dread(int argc, void *argv)
{
    uint8_t buf[128] = {0};
    int printf_index = 0, itemsz = 1, count, ret;
    char format = 'x';

    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <size> */
    ret = sscanf(MR_MSH_GET_ARG(1), "%d", &count);
    if ((ret < 1) || (count < 0))
    {
        goto usage;
    }

    /* Parse [-1|-2|-4] */
    if (MR_MSH_GET_ARG(2) != MR_NULL)
    {
        int arg_index = 3;

        /* Parse item size */
        if (strncmp(MR_MSH_GET_ARG(2), "-1", 2) == 0)
        {
            printf_index = 0;
            itemsz = 1;
        } else if (strncmp(MR_MSH_GET_ARG(2), "-2", 2) == 0)
        {
            printf_index = 1;
            itemsz = 2;
        } else if (strncmp(MR_MSH_GET_ARG(2), "-4", 2) == 0)
        {
            printf_index = 2;
            itemsz = 4;
        } else
        {
            arg_index = 2;
        }

        /* Parse [-x|-d|-u|-c] */
        if ((arg_index == 2) || (MR_MSH_GET_ARG(3) != MR_NULL))
        {
            /* Parse format */
            if (strncmp(MR_MSH_GET_ARG(arg_index), "-x", 2) == 0)
            {
                format = 'x';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-d", 2) == 0)
            {
                format = 'd';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-u", 2) == 0)
            {
                format = 'u';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-c", 2) == 0)
            {
                printf_index = 0;
                itemsz = 1;
                format = 'c';
            } else
            {
                goto usage;
            }
        }
    }

    /* Read data */
    count = MR_BOUND(count * itemsz, 0, sizeof(buf));
    ret = (int)mr_dev_read(MSH_GET_DESC(), buf, count);
    if (ret < 0)
    {
        mr_msh_printf("dread: %s\r\n", mr_strerror(ret));
        return;
    } else if (ret == 0)
    {
        mr_msh_printf("dread: no data\r\n");
        return;
    }

    /* Print data */
    msh_printf_fn[printf_index](buf, ret, format);
    mr_msh_printf("\r\n");
    return;

    usage:
    mr_msh_printf("usage: dread <count> [-1|-2|-4] [-x|-d|-u|-c]\r\n");
    mr_msh_printf("       [-1]: <count (1-128)>\r\n");
    mr_msh_printf("       [-2]: <count (1-64)>\r\n");
    mr_msh_printf("       [-4]: <count (1-32)>\r\n");
}

static void msh_cmd_dwrite(int argc, void *argv)
{
    uint8_t buf[128];
    int data_index = 1, itemsz = 1, count, ret;
    char format = 'x';

    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse [-1|-2|-4] */
    if (MR_MSH_GET_ARG(1) != MR_NULL)
    {
        int arg_index = 2;

        /* Parse item size */
        if (strncmp(MR_MSH_GET_ARG(1), "-1", 2) == 0)
        {
            itemsz = 1;
        } else if (strncmp(MR_MSH_GET_ARG(1), "-2", 2) == 0)
        {
            itemsz = 2;
        } else if (strncmp(MR_MSH_GET_ARG(1), "-4", 2) == 0)
        {
            itemsz = 4;
        } else
        {
            arg_index = 1;
        }
        data_index = arg_index + 1;

        /* Parse [-x|-d|-u|-c] */
        if ((arg_index == 1) || (MR_MSH_GET_ARG(2) != MR_NULL))
        {
            data_index = arg_index + 1;

            /* Parse format */
            if (strncmp(MR_MSH_GET_ARG(arg_index), "-x", 2) == 0)
            {
                format = 'x';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-d", 2) == 0)
            {
                format = 'd';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-u", 2) == 0)
            {
                format = 'u';
            } else if (strncmp(MR_MSH_GET_ARG(arg_index), "-c", 2) == 0)
            {
                itemsz = 1;
                format = 'c';
            } else
            {
                data_index--;
            }
        }
    }

    /* Parse data and write */
    count = MR_BOUND((argc - data_index + 1) * itemsz, 0, sizeof(buf));
    for (size_t i = data_index;
         i <= (argc < (sizeof(buf) / itemsz + data_index) ? argc : (sizeof(buf) / itemsz + data_index));
         i++)
    {
        int arg;

        if (format == 'd')
        {
            ret = sscanf(MR_MSH_GET_ARG(i), "%d", &arg);
        } else if (format == 'u')
        {
            ret = sscanf(MR_MSH_GET_ARG(i), "%u", &arg);
        } else if (format == 'c')
        {
            ret = sscanf(MR_MSH_GET_ARG(i), "%c", (char *)&arg);
        } else
        {
            ret = sscanf(MR_MSH_GET_ARG(i), "%x", &arg);
        }
        memcpy(buf + (i - data_index) * itemsz, &arg, itemsz);
        if (ret < 1)
        {
            goto usage;
        }
    }
    ret = (int)mr_dev_write(MSH_GET_DESC(), buf, count);
    if (ret < 0)
    {
        mr_msh_printf("dwrite: %s\r\n", mr_strerror(ret));
    }
    return;

    usage:
    mr_msh_printf("usage: dwrite [-1|-2|-4] [-x|-d|-u|-c] <data>\r\n");
}

/**
 * @brief Exports device MSH commands.
 */
MR_MSH_CMD_EXPORT(dlist, msh_cmd_dlist, "list all devices.");
MR_MSH_CMD_EXPORT(dselect, msh_cmd_dselect, "select a device by descriptor.");
MR_MSH_CMD_EXPORT(dopen, msh_cmd_dopen, "open a device.");
MR_MSH_CMD_EXPORT(dclose, msh_cmd_dclose, "close a device.");
MR_MSH_CMD_EXPORT(dioctl, msh_cmd_dioctl, "ioctl a device.");
MR_MSH_CMD_EXPORT(dread, msh_cmd_dread, "read from a device.");
MR_MSH_CMD_EXPORT(dwrite, msh_cmd_dwrite, "write to a device.");

#endif /* defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD) */
