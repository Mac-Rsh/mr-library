/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-01-14    MacRsh       First version
 */

#include "include/components/mr_msh.h"

#if defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD)

static int msh_desc = -1;

#define MSH_SET_DESC(desc)  (msh_desc = (desc))
#define MSH_GET_DESC()      (msh_desc)

static int msh_update_path(int desc)
{
    static char msh_path[MR_CFG_DEV_NAME_MAX * 6];
    struct
    {
        char *buf;
        size_t bufsz;
    } path = {msh_path, sizeof(msh_path)};
    int offset;

    /* Update the path */
    int ret = mr_dev_ioctl(desc, MR_CTL_GET_PATH, &path);
    if (ret < 0)
    {
        return ret;
    }

    /* Update the offset */
    mr_dev_ioctl(desc, MR_CTL_GET_OFFSET, &offset);
    snprintf(msh_path + ret, sizeof(msh_path) - ret, "/%d", offset);

    /* Update the prompt */
    MSH_SET_DESC(desc);
    mr_msh_set_prompt(msh_path);
    return MR_EOK;
}

static int msh_cmd_dopen(int argc, void *argv)
{
    const char *path;
    int oflags = MR_OFLAG_CLOSED;
    int desc;

    /* Check the arguments and print usage */
    if (argc < 2)
    {
        goto usage;
    }

    /* Parse [r|w|rw] */
    if (strncmp(MR_MSH_GET_ARG(2), "rw", 2) == 0)
    {
        MR_BIT_SET(oflags, MR_OFLAG_RDWR);
    } else if (strncmp(MR_MSH_GET_ARG(2), "r", 1) == 0)
    {
        MR_BIT_SET(oflags, MR_OFLAG_RDONLY);
    } else if (strncmp(MR_MSH_GET_ARG(2), "w", 1) == 0)
    {
        MR_BIT_SET(oflags, MR_OFLAG_WRONLY);
    } else
    {
        goto usage;
    }
    if ((MR_MSH_GET_ARG(3) != MR_NULL) && (strncmp(MR_MSH_GET_ARG(3), "-n", 2) == 0))
    {
        MR_BIT_SET(oflags, MR_OFLAG_NONBLOCK);
    }

    /* Open the new device */
    path = MR_MSH_GET_ARG(1);
    desc = mr_dev_open(path, oflags);
    if (desc < 0)
    {
        mr_msh_printf("error: %s\r\n", mr_strerror(desc));
        return desc;
    }

    /* Switch to the new descriptor */
    return msh_update_path(desc);

    usage:
    mr_msh_printf("usage: dopen <path> <r|w|rw> [-n]\r\n");
    return MR_EINVAL;
}

static int msh_cmd_dclose(int argc, void *argv)
{
    int desc;
    int ret;

    /* Check the arguments and print usage */
    if ((MR_MSH_GET_ARG(1) != MR_NULL) && (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse [desc (>=0)] */
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
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
        return ret;
    }

    /* Switch to the root device */
    if (desc == MSH_GET_DESC())
    {
        MSH_SET_DESC(-1);
        mr_msh_set_prompt("/");
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dclose [desc (>=0)]\r\n");
    return MR_EINVAL;
}

static int msh_cmd_dselect(int argc, void *argv)
{
    int desc;
    int ret;

    /* Check the arguments and print usage */
    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <desc (>=0)> */
    ret = sscanf(MR_MSH_GET_ARG(1), "%d", &desc);
    if ((ret < 1) || (desc < 0))
    {
        goto usage;
    }

    /* Switch to the new descriptor */
    return msh_update_path(desc);

    usage:
    mr_msh_printf("usage: dselect <desc (>=0)>\r\n");
    return MR_EINVAL;
}

static int msh_dioctl_offset(int argc, void *argv)
{
    int cmd = MR_CTL_SET_OFFSET;
    int offset;
    int ret;

    /* Check the arguments and print usage */
    if (argc < 2)
    {
        goto usage;
    }

    /* Parse <-g|offset> */
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        cmd = -cmd;
    } else
    {
        ret = sscanf(MR_MSH_GET_ARG(2), "%d", &offset);
        if (ret < 1)
        {
            goto usage;
        }
    }

    /* Get/set the offset */
    mr_dev_ioctl(MSH_GET_DESC(), cmd, &offset);
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", offset);
    } else
    {
        /* Update the offset */
        msh_update_path(MSH_GET_DESC());
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dioctl off <offset|-g>\r\n");
    return MR_EINVAL;
}

static int msh_dioctl_bufsz(int argc, void *argv)
{
    int cmd;
    int bufsz;
    int ret;

    /* Check the arguments and print usage */
    if (argc < 3)
    {
        goto usage;
    }

    /* Parse <-r|-w> */
    if (strncmp(MR_MSH_GET_ARG(3), "-r", 2) == 0)
    {
        cmd = MR_CTL_SET_RD_BUFSZ;
    } else if (strncmp(MR_MSH_GET_ARG(3), "-w", 2) == 0)
    {
        cmd = MR_CTL_SET_WR_BUFSZ;
    } else
    {
        goto usage;
    }

    /* Parse <-g|bufsz (>=0)> */
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
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
        return ret;
    }

    /* If the command is <-g>, print the buffer size */
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", bufsz);
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dioctl bufsz <bufsz (>=0)|-g> <-r|-w>\r\n");
    return MR_EINVAL;
}

static int msh_dioctl_datasz(int argc, void *argv)
{
    int cmd = MR_CTL_SET_RD_BUFSZ;
    int datasz;
    int ret;

    /* Check the arguments and print usage */
    if (argc < 3)
    {
        goto usage;
    }

    /* Parse <-r|-w> */
    if (strncmp(MR_MSH_GET_ARG(3), "-r", 2) == 0)
    {
        cmd = MR_CTL_CLR_RD_BUF;
    } else if (strncmp(MR_MSH_GET_ARG(3), "-w", 2) == 0)
    {
        cmd = MR_CTL_CLR_WR_BUF;
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
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
        return ret;
    }

    /* If the command is <-g>, print the buffer size */
    if (cmd < 0)
    {
        mr_msh_printf("%d\r\n", datasz);
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dioctl datasz <-c|-g> <-r|-w>\r\n");
    return MR_EINVAL;
}

static int msh_cmd_dioctl_cfg(int argc, void *argv)
{
    int cfg[32];
    int ret;

    /* Check the arguments and print usage */
    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-g> */
    memset(cfg, 0, sizeof(cfg));
    if (strncmp(MR_MSH_GET_ARG(2), "-g", 2) == 0)
    {
        /* Get the config */
        ret = mr_dev_ioctl(MSH_GET_DESC(), MR_CTL_GET_CONFIG, cfg);
        if (ret < 0)
        {
            mr_msh_printf("error: %s\r\n", mr_strerror(ret));
            return ret;
        }
        for (size_t i = 0; i < ret / sizeof(int); i++)
        {
            mr_msh_printf("%d ", cfg[i]);
        }
        mr_msh_printf("\r\n");
        return MR_EOK;
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
    ret = mr_dev_ioctl(MSH_GET_DESC(), MR_CTL_SET_CONFIG, cfg);
    if (ret < 0)
    {
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dioctl cfg <args|-g>\r\n");
    return MR_EINVAL;
}

static int msh_dioctl_flags(int argc, void *argv)
{
    int cmd;

    /* Check the arguments and print usage */
    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <-o|-s> */
    if (strncmp(MR_MSH_GET_ARG(2), "-o", 2) == 0)
    {
        cmd = MR_CTL_GET_OFLAGS;
    } else if (strncmp(MR_MSH_GET_ARG(2), "-s", 2) == 0)
    {
        cmd = MR_CTL_GET_SFLAGS;
    } else
    {
        goto usage;
    }

    /* Parse <-g> */
    if ((MR_MSH_GET_ARG(3) != MR_NULL) && (strncmp(MR_MSH_GET_ARG(3), "-g", 2) == 0))
    {
        int flags;

        mr_dev_ioctl(MSH_GET_DESC(), cmd, &flags);
        if (MR_BIT_IS_SET(flags, MR_OFLAG_RDWR) == MR_ENABLE)
        {
            mr_msh_printf("rw");
        } else if (MR_BIT_IS_SET(flags, MR_OFLAG_WRONLY) == MR_ENABLE)
        {
            mr_msh_printf("w");
        } else
        {
            mr_msh_printf("r");
        }
        if (MR_BIT_IS_SET(flags, MR_OFLAG_NONBLOCK) == MR_ENABLE)
        {
            mr_msh_printf("n");
        }
        mr_msh_printf("\r\n");
        return MR_EOK;
    }

    usage:
    mr_msh_printf("usage: dioctl flags <-s|-o> <-g>\r\n");
    return MR_EINVAL;
}

static int msh_dioctl_cmd(int argc, void *argv)
{
    int cmd;
    int args[32];
    int ret;

    /* Check the arguments and print usage */
    if ((argc < 2) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
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
            mr_msh_printf("error: %s\r\n", mr_strerror(ret));
            return ret;
        }
        for (size_t i = 0; i < ret / sizeof(int); i++)
        {
            mr_msh_printf("%d ", args[i]);
        }
        mr_msh_printf("\r\n");
        return MR_EOK;
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
            mr_msh_printf("error: %s\r\n", mr_strerror(ret));
            return ret;
        }
    }
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dioctl <cmd> <args|-g>\r\n");
    return MR_EINVAL;
}

static int msh_cmd_dioctl(int argc, void *argv)
{
    /* Check the arguments and print usage */
    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <off|cfg|bufsz|datasz|flags> */
    if (strncmp(MR_MSH_GET_ARG(1), "off", 6) == 0)
    {
        return msh_dioctl_offset(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "cfg", 3) == 0)
    {
        return msh_cmd_dioctl_cfg(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "bufsz", 5) == 0)
    {
        return msh_dioctl_bufsz(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "datasz", 6) == 0)
    {
        return msh_dioctl_datasz(argc, argv);
    } else if (strncmp(MR_MSH_GET_ARG(1), "flags", 6) == 0)
    {
        return msh_dioctl_flags(argc, argv);
    } else
    {
        return msh_dioctl_cmd(argc, argv);
    }

    usage:
    mr_msh_printf("usage: dioctl <cmd> <args|-g>\r\n");
    mr_msh_printf("       dioctl off <offset|-g>\r\n");
    mr_msh_printf("       dioctl cfg <args|-g>\r\n");
    mr_msh_printf("       dioctl bufsz <bufsz (>=0)|-g> <-r|-w>\r\n");
    mr_msh_printf("       dioctl datasz <-c|-g> <-r|-w>\r\n");
    mr_msh_printf("       dioctl flags <-s|-o> <-g>\r\n");
    return MR_EINVAL;
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

static int msh_cmd_dread(int argc, void *argv)
{
    int printf_index = 0;
    int itemsz = 1;
    char format = 'x';
    int size;
    uint8_t buf[128];
    int ret;

    /* Check the arguments and print usage */
    if ((argc < 1) || (strncmp(MR_MSH_GET_ARG(1), "-h", 2) == 0))
    {
        goto usage;
    }

    /* Parse <size> */
    ret = sscanf(MR_MSH_GET_ARG(1), "%d", &size);
    if ((ret < 1) || (size < 0))
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
    size = MR_BOUND(size *itemsz, 0, sizeof(buf));
    ret = (int)mr_dev_read(MSH_GET_DESC(), buf, size);
    if (ret < 0)
    {
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
        return ret;
    }

    /* Print data */
    msh_printf_fn[printf_index](buf, size, format);
    mr_msh_printf("\r\n");
    return MR_EOK;

    usage:
    mr_msh_printf("usage: dread <size> [-1|-2|-4] [-x|-d|-u|-c]\r\n");
    mr_msh_printf("       [-1]: <size (1-128)>\r\n");
    mr_msh_printf("       [-2]: <size (1-64)>\r\n");
    mr_msh_printf("       [-4]: <size (1-32)>\r\n");
    return MR_EINVAL;
}

static int msh_cmd_dwrite(int argc, void *argv)
{
    int data_index = 1;
    int itemsz = 1;
    char format = 'x';
    int size;
    uint8_t buf[128];
    int ret;

    /* Check the arguments and print usage */
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
    size = MR_BOUND((argc - data_index + 1) * itemsz, 0, sizeof(buf));
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
    ret = (int)mr_dev_write(MSH_GET_DESC(), buf, size);
    if (ret < 0)
    {
        mr_msh_printf("error: %s\r\n", mr_strerror(ret));
    }
    return ret;

    usage:
    mr_msh_printf("usage: dwrite [-1|-2|-4] [-x|-d|-u|-c] <data>\r\n");
    return MR_EINVAL;
}

/**
 * @brief Exports device MSH commands.
 */
MR_MSH_CMD_EXPORT(dopen, msh_cmd_dopen, "open a device.");
MR_MSH_CMD_EXPORT(dclose, msh_cmd_dclose, "close a device.");
MR_MSH_CMD_EXPORT(dselect, msh_cmd_dselect, "select a device.");
MR_MSH_CMD_EXPORT(dioctl, msh_cmd_dioctl, "ioctl a device.");
MR_MSH_CMD_EXPORT(dread, msh_cmd_dread, "read from a device.");
MR_MSH_CMD_EXPORT(dwrite, msh_cmd_dwrite, "write to a device.");

#endif /* defined(MR_USING_MSH) && defined(MR_USING_MSH_DEV_CMD) */
