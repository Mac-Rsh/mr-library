/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-25    MacRsh       First version
 */

#include "include/components/msh.h"

#ifdef MR_USING_MSH

/**
 * @brief MSH structure.
 */
static struct
{
#ifndef MR_CFG_MSH_BUFSZ
#define MR_CFG_MSH_BUFSZ                (32)
#endif /* MR_CFG_MSHELL_BUFSZ */
    char buf[MR_CFG_MSH_BUFSZ];                                     /**< Buffer for reading */
#ifndef MR_CFG_MSH_ARGS_MAX
#define MR_CFG_MSH_ARGS_MAX             (8)
#endif /* MR_CFG_MSH_ARGS_MAX */
    char *args[MR_CFG_MSH_ARGS_MAX];                                /**< Arguments for the msh */
    size_t cursor;                                                  /**< Cursor position */
    size_t bytes;                                                   /**< Bytes in the buffer */
    char key_buf[8];                                                /**< Buffer for keys */
    size_t key_bytes;                                               /**< Bytes in the key buffer */
    int echo;                                                       /**< Echo or not */
} msh;

MR_MSH_EXPORT(start, MR_NULL, MR_NULL, "0");
MR_MSH_EXPORT(end, MR_NULL, MR_NULL, "1.end");

#define MSH_CURSOR_FORWARD(x)           "\x1b["#x"D"
#define MSH_CURSOR_BACKWARD(x)          "\x1b["#x"C"
#define MSH_DELETE_CHAR(x)              "\x1b["#x"P"
#define MSH_INSERT_CHAR(x)              "\x1b["#x"@"

static void msh_new_line(void)
{
    /* Move the cursor to the beginning of the line and print a new line */
    if (msh.echo == MR_ENABLE)
    {
#ifndef MR_CFG_MSH_PROMPT
#define MR_CFG_MSH_PROMPT               "msh>"
#endif /* MR_CFG_MSH_PROMPT */
        mr_printf("\r\n"MR_CFG_MSH_PROMPT" ");
    }
    msh.cursor = 0;
    msh.bytes = 0;
    msh.key_bytes = 0;
}

static void msh_move_cursor_left(void)
{
    if (msh.cursor > 0)
    {
        msh.cursor--;
        mr_printf(MSH_CURSOR_FORWARD(1));
    }
}

static void msh_move_cursor_right(void)
{
    if (msh.cursor < msh.bytes)
    {
        msh.cursor++;
        mr_printf(MSH_CURSOR_BACKWARD(1));
    }
}

static void msh_delete_char(void)
{
    if (msh.cursor > 0)
    {
        /* Move the cursor forward and delete the character */
        mr_printf(MSH_CURSOR_FORWARD(1)MSH_DELETE_CHAR(1));

        /* Check if you need to remove characters from the middle */
        if (msh.cursor != msh.bytes)
        {
            /* Readjust string */
            for (size_t i = msh.cursor; i < msh.bytes; i++)
            {
                msh.buf[i - 1] = msh.buf[i];
            }
        }
        msh.cursor--;
        msh.bytes--;
    }
}

static void msh_insert_char(char c)
{
    /* Check if there is enough space */
    if ((msh.bytes + 1) == MR_CFG_MSH_BUFSZ)
    {
        return;
    }

    /* Check whether the cursor is at the end of the buffer */
    if (msh.cursor == msh.bytes)
    {
        msh.buf[msh.bytes] = c;
    } else
    {
        /* Insert the character */
        mr_printf(MSH_INSERT_CHAR(1));

        /* Readjust string */
        for (size_t i = msh.cursor; i < msh.bytes; i++)
        {
            msh.buf[i + 1] = msh.buf[i];
        }
        msh.buf[msh.cursor] = c;
    }
    msh.cursor++;
    msh.bytes++;
    msh.buf[msh.bytes] = '\0';

    /* Echo the character */
    if (msh.echo == MR_ENABLE)
    {
        mr_printf("%c", c);
    }
}

static void msh_parse_cmd(void)
{
    /* Check whether the buffer is empty */
    if (msh.bytes == 0)
    {
        return;
    }

    /* Find the command */
    char *cmd = strchr(msh.buf, ' ');
    if (cmd != MR_NULL)
    {
        *cmd = '\0';
    }

    /* Execute the command */
    for (const struct mr_msh_cmd *msh_cmd = ((&_mr_msh_cmd_start) + 1); msh_cmd < &_mr_msh_cmd_end; msh_cmd++)
    {
        if (strcmp(msh_cmd->name, msh.buf) != 0)
        {
            continue;
        }

        /* Restore the command */
        if (cmd != MR_NULL)
        {
            *cmd = ' ';
        }

        /* Parse the arguments */
        char *old_arg = msh.buf;
        int args_num = 0;
        for (args_num = 0; args_num < MR_ARRAY_SIZE(msh.args); args_num++)
        {
            char *arg = strchr(old_arg, ' ');
            if (arg == MR_NULL)
            {
                break;
            }
            *arg = '\0';
            msh.args[args_num] = ++arg;
            old_arg = arg;
        }

        mr_printf("\r\n");
        msh_cmd->call(args_num, msh.args);
    }
}

#define KEY_BACKSPACE                   "\x7f"
#define KEY_ENTER                       "\r"
#define KEY_LEFT                        "\x1b[D"
#define KEY_RIGHT                       "\x1b[C"
#define KEY_DELETE                      "\x1B[3~"

#define MSH_IS_PRINTABLE(c)             ((c) >= 0x20 && (c) <= 0x7e)
#define MSH_IS_ESC_KEY(c)               ((c) == 0x1b)
#define MSH_IS_END_KEY(c) \
    ((((c) >= 'A') && ((c) <= 'D')) || (((c) >= 'P') && ((c) <= 'S')) || ((c) >= '~') || ((c) == 'H') || ((c) == 'F'))

static void msh_key_enter(void)
{
    msh_parse_cmd();
    msh_new_line();
}

static void msh_key_backspace(void)
{
    msh_delete_char();
}

static void msh_key_left(void)
{
    msh_move_cursor_left();
}

static void msh_key_right(void)
{
    msh_move_cursor_right();
}

static void msh_key_delete(void)
{
    msh_move_cursor_right();
    msh_delete_char();
}

/**
 * @brief Msh long character key map structure.
 */
static struct
{
    char *key;
    void (*exec)(void);
} msh_key_map[] =
    {
        {KEY_ENTER,     msh_key_enter},
        {KEY_BACKSPACE, msh_key_backspace},
        {KEY_LEFT,      msh_key_left},
        {KEY_RIGHT,     msh_key_right},
        {KEY_DELETE,    msh_key_delete},
    };

static void msh_parse_key(char c)
{
    int parse_flag = MR_DISABLE;

    /* key-bytes: 0 -> short character key, 1 -> long character key */
    if (msh.key_bytes == 0)
    {
        if (MSH_IS_ESC_KEY(c))
        {
            msh.key_buf[msh.key_bytes++] = c;
        } else
        {
            msh.key_buf[msh.key_bytes++] = c;
            msh.key_buf[msh.key_bytes] = '\0';
            parse_flag = MR_ENABLE;
        }
    } else
    {
        if (MSH_IS_END_KEY(c))
        {
            msh.key_buf[msh.key_bytes++] = c;
            msh.key_buf[msh.key_bytes] = '\0';
            parse_flag = MR_ENABLE;
        } else
        {
            msh.key_buf[msh.key_bytes++] = c;
            /* Check whether the key-buffer is full */
            if (msh.key_bytes >= sizeof(msh.key_buf) - 1)
            {
                msh.key_bytes = 0;
            }
        }
    }

    /* Parse the long character key */
    if (parse_flag == MR_ENABLE)
    {
        for (size_t i = 0; i < MR_ARRAY_SIZE(msh_key_map); i++)
        {
            if (strncmp(msh.key_buf, msh_key_map[i].key, msh.key_bytes) == 0)
            {
                msh_key_map[i].exec();
                break;
            }
        }
        msh.key_bytes = 0;
    }
}

static int msh_cmd_help(int argc, void *args)
{
    /* Print the help */
    for (const struct mr_msh_cmd *msh_cmd = ((&_mr_msh_cmd_start) + 1); msh_cmd < &_mr_msh_cmd_end; msh_cmd++)
    {
        mr_printf("%-*s - %s\r\n", MR_CFG_MSH_NAME_MAX, msh_cmd->name, msh_cmd->help);
    }
    return MR_EOK;
}

static int msh_cmd_clear(int argc, void *args)
{
    mr_printf("\033c");
    msh_new_line();
    return MR_EOK;
}

/**
 * @brief This function receives a character for the msh.
 *
 * @param c The character to receive.
 */
void mr_msh_recv_char(char c)
{
    /* Judgments are characters and keys */
    if (MSH_IS_PRINTABLE(c) && (msh.key_bytes == 0))
    {
        msh_insert_char(c);
    } else
    {
        msh_parse_key(c);
    }
}

/**
 * @brief This function initialize the msh.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_msh_init(void)
{
#ifdef MR_USING_MSH_ECHO
    msh.echo = MR_ENABLE;
#endif /* MR_USING_MSH_ECHO */
    /* Print the prompt */
    msh_new_line();
    return MR_EOK;
}
MR_INIT_DEV_EXPORT(mr_msh_init);

/**
 * @brief Exports default MSH commands.
 */
MR_MSH_CMD_EXPORT(help, msh_cmd_help, "Show help information.");
MR_MSH_CMD_EXPORT(clear, msh_cmd_clear, "Clear the screen.");

#endif /* MR_USING_MSH */
