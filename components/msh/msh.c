/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-25    MacRsh       First version
 */

#include "include/components/mr_msh.h"

#ifdef MR_USING_MSH

/**
 * @brief MSH structure.
 */
static struct mr_msh
{
#ifndef MR_CFG_MSH_BUFSZ
#define MR_CFG_MSH_BUFSZ                (32)
#endif /* MR_CFG_MSHELL_BUFSZ */
    char buf[MR_CFG_MSH_BUFSZ];                                     /**< Buffer for reading */
    uint32_t cursor;                                                /**< Cursor position */
    uint32_t bytes;                                                 /**< Bytes in the buffer */
    char key_buf[8];                                                /**< Buffer for keys */
    uint16_t key_bytes: 15;                                         /**< Bytes in the key buffer */
    uint16_t echo: 1;                                               /**< Echo or not */
    uint16_t history_index;                                         /**< History index */
    uint16_t history_pos;                                           /**< History position */
    uint16_t history_len;                                           /**< History length */
#ifndef MR_CFG_MSH_HISTORY_LINES
#define MR_CFG_MSH_HISTORY_LINES        (4)
#endif /* MR_CFG_MSH_HISTORY_LINES */
    char history[MR_CFG_MSH_HISTORY_LINES][MR_CFG_MSH_BUFSZ];       /**< History buffer */
    char *prompt;                                                   /**< Prompt string */
    int desc;                                                       /**< Device descriptor */
} msh;

#define MR_MSH_EXPORT(name, fn, help, level) \
    MR_USED const struct mr_msh_cmd _mr_msh_cmd_##name MR_SECTION("mr_msh_cmd."level) = {#name, fn, help};
MR_MSH_EXPORT(start, MR_NULL, MR_NULL, "0");                        /**< Start shell command */
MR_MSH_EXPORT(end, MR_NULL, MR_NULL, "1.end");                      /**< End shell command */

/**
 * @brief Shell commands.
 */
#define MSH_CURSOR_FORWARD              "\x1b[1D"                   /**< Move cursor forward */
#define MSH_CURSOR_BACKWARD             "\x1b[1C"                   /**< Move cursor backward */
#define MSH_DELETE_CHAR                 "\x1b[1P"                   /**< Delete chars */
#define MSH_INSERT_CHAR                 "\x1b[1@"                   /**< Insert chars */
#define MSH_CLEAR_SCREEN                "\x1b[2J""\x1b[1;1H"        /**< Clear screen */
#define MSH_CLEAR_LINE                  "\x1b[2K""\x1b[1G"          /**< Clear line */

MR_INLINE void msh_new_current_line(void)
{
    msh.cursor = 0;
    msh.bytes = 0;
    if (msh.echo == MR_ENABLE)
    {
#ifndef MR_CFG_MSH_PROMPT
#define MR_CFG_MSH_PROMPT               "msh"
#endif /* MR_CFG_MSH_PROMPT */
#ifdef MR_USING_MSH_PRINTF_COLOR
#define MR_MSH_COLOR_CYAN(str)          "\033[36m"str"\033[0m"
#else
#define MR_MSH_COLOR_CYAN(str)          str
#endif /* MR_USING_MSH_PRINTF_COLOR */
        mr_msh_printf(MR_MSH_COLOR_CYAN("%s ")MR_MSH_COLOR_YELLOW("%s> "), MR_CFG_MSH_PROMPT, msh.prompt);
    }
}

MR_INLINE void msh_refresh_line(void)
{
    if (msh.echo == MR_ENABLE)
    {
        mr_msh_printf(MSH_CLEAR_LINE);
        mr_msh_printf(MR_MSH_COLOR_CYAN("%s ")MR_MSH_COLOR_YELLOW("%s> ")"%s", MR_CFG_MSH_PROMPT, msh.prompt, msh.buf);
    }
}

MR_INLINE void msh_move_cursor_left(void)
{
    if (msh.cursor > 0)
    {
        msh.cursor--;
        mr_msh_printf(MSH_CURSOR_FORWARD);
    }
}

MR_INLINE void msh_move_cursor_right(void)
{
    if (msh.cursor < msh.bytes)
    {
        msh.cursor++;
        mr_msh_printf(MSH_CURSOR_BACKWARD);
    }
}

MR_INLINE void msh_delete_char(void)
{
    if (msh.cursor > 0)
    {
        /* Move the cursor forward and delete the character */
        mr_msh_printf(MSH_CURSOR_FORWARD MSH_DELETE_CHAR);

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
        msh.buf[msh.bytes] = '\0';
    }
}

MR_INLINE void msh_insert_char(char c)
{
    if ((msh.bytes + 1) == MR_CFG_MSH_BUFSZ)
    {
        return;
    }

    /* Check whether the cursor is at the end of the buffer */
    if (msh.cursor != msh.bytes)
    {
        if (msh.echo == MR_ENABLE)
        {
            mr_msh_printf(MSH_INSERT_CHAR);
        }

        /* Readjust string */
        for (size_t i = msh.bytes; i > msh.cursor; i--)
        {
            msh.buf[i] = msh.buf[i - 1];
        }
    }
    msh.buf[msh.cursor] = c;
    msh.cursor++;
    msh.bytes++;
    msh.buf[msh.bytes] = '\0';

    /* Echo the character */
    if (msh.echo == MR_ENABLE)
    {
        mr_msh_printf("%c", c);
    }
}

MR_INLINE void msh_auto_complete(void)
{
    const struct mr_msh_cmd *msh_comp = MR_NULL;

    /* Find the command */
    for (const struct mr_msh_cmd *msh_cmd = ((&_mr_msh_cmd_start) + 1); msh_cmd < &_mr_msh_cmd_end; msh_cmd++)
    {
        /* When cursor is equal to 0, all commands are satisfied, so there is no auto-completion */
        if (strncmp(msh_cmd->name, msh.buf, msh.cursor) == 0)
        {
            /* Check whether multiple commands are met */
            if (msh_comp != MR_NULL)
            {
                return;
            }
            msh_comp = msh_cmd;
        }
    }

    /* Complete the command */
    if (msh_comp != MR_NULL)
    {
        for (size_t i = msh.cursor; i < strlen(msh_comp->name); i++)
        {
            msh_insert_char(msh_comp->name[i]);
        }
    }
}

MR_INLINE void msh_history_push(void)
{
    uint16_t history_prev = (msh.history_index + (MR_CFG_MSH_HISTORY_LINES - 1)) % MR_CFG_MSH_HISTORY_LINES;

    /* Check whether this command is the same as the previous one */
    if ((msh.bytes != 0) && (strncmp(msh.history[history_prev], msh.buf, msh.bytes) != 0))
    {
        memcpy(msh.history[msh.history_index], msh.buf, (msh.bytes + 1));
        msh.history_index = (msh.history_index + 1) % MR_CFG_MSH_HISTORY_LINES;
        msh.history_len = MR_BOUND(msh.history_len + 1, 0, MR_CFG_MSH_HISTORY_LINES);
    }
    msh.history_pos = 0;
}

MR_INLINE void msh_history_pop_prev(void)
{
    if (msh.history_pos < msh.history_len)
    {
        msh.history_pos++;

        /* If the previous index is equal to the current index, no previous record is available */
        uint16_t index = (msh.history_index + MR_CFG_MSH_HISTORY_LINES - msh.history_pos) % MR_CFG_MSH_HISTORY_LINES;

        /* Refresh the line with the previous command */
        msh.cursor = msh.bytes = strlen(msh.history[index]);
        strncpy(msh.buf, msh.history[index], msh.bytes + 1);
        msh_refresh_line();
    }
}

MR_INLINE void msh_history_pop_next(void)
{
    if (msh.history_pos > 1)
    {
        msh.history_pos--;
        uint16_t index = (msh.history_index + MR_CFG_MSH_HISTORY_LINES - msh.history_pos) % MR_CFG_MSH_HISTORY_LINES;
        if (index >= msh.history_len)
        {
            index = 0;
        }

        /* Refresh the line with the next command */
        msh.cursor = msh.bytes = strlen(msh.history[index]);
        strncpy(msh.buf, msh.history[index], msh.bytes + 1);
        msh_refresh_line();
    }
}

MR_INLINE void msh_parse_cmd(void)
{
    mr_msh_printf("\r\n");

    if (msh.bytes == 0)
    {
        msh_new_current_line();
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
        if (strncmp(msh_cmd->name, msh.buf, strlen(msh_cmd->name)) != 0)
        {
            continue;
        }

        /* Restore the command */
        if (cmd != MR_NULL)
        {
            *cmd = ' ';
        }

        /* Parse the arguments */
        int argc;
#ifndef MR_CFG_MSH_ARGS_NUM
#define MR_CFG_MSH_ARGS_NUM             (8)
#endif /* MR_CFG_MSH_ARGS_NUM */
        char *argv[MR_CFG_MSH_ARGS_NUM] = {MR_NULL};
        char *old_arg = msh.buf;
        for (argc = 0; argc < MR_CFG_MSH_ARGS_NUM; argc++)
        {
            char *arg = strchr(old_arg, ' ');
            if ((arg == MR_NULL) || (*(arg + 1) == '\0') || (*(arg + 1) == ' '))
            {
                break;
            }
            *arg = '\0';
            argv[argc] = ++arg;
            old_arg = arg;
        }

        /* Execute the command */
        msh_cmd->fn(argc, argv);
        msh_new_current_line();
        return;
    }

    /* Command not found */
    mr_msh_printf("%s: command not found\r\n", msh.buf);
    msh_new_current_line();
}

static void msh_key_enter(void)
{
    msh_history_push();
    msh_parse_cmd();
}

static void msh_key_backspace(void)
{
    msh_delete_char();
}

static void msh_key_table(void)
{
    msh_auto_complete();
}

static void msh_key_up(void)
{
    msh_history_pop_prev();
}

static void msh_key_down(void)
{
    msh_history_pop_next();
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

#define KEY_ENTER                       "\r"                        /**< Enter key */
#define KEY_BACKSPACE                   "\b"                        /**< Backspace key */
#define KEY_TABLE                       "\t"                        /**< Tab key */
#define KEY_LEFT                        "\x1b[D"                    /**< Left key */
#define KEY_RIGHT                       "\x1b[C"                    /**< Right key */
#define KEY_UP                          "\x1B[A"                    /**< Up key */
#define KEY_DOWN                        "\x1B[B"                    /**< Down key */
#define KEY_DELETE                      "\x1B[3~"                   /**< Delete key */

#define MSH_IS_PRINTABLE(c)             ((c) >= 0x20 && (c) <= 0x7e)
#define MSH_IS_ESC_KEY(c)               ((c) == 0x1b)
#define MSH_IS_END_KEY(c)               ((((c) >= 'A') && ((c) <= 'Z')) || ((c) >= '~'))

/**
 * @brief Msh long character key map structure.
 */
static struct
{
    const char *key;
    void (*fn)(void);
} msh_key_map[] =
    {
        {KEY_ENTER,     msh_key_enter},
        {KEY_BACKSPACE, msh_key_backspace},
        {KEY_TABLE,     msh_key_table},
        {KEY_LEFT,      msh_key_left},
        {KEY_RIGHT,     msh_key_right},
        {KEY_UP,        msh_key_up},
        {KEY_DOWN,      msh_key_down},
        {KEY_DELETE,    msh_key_delete},
    };

MR_INLINE void msh_parse_key(char c)
{
    int parse_flag = MR_DISABLE;

    /* key-bytes: (0) -> short character key, (!0) -> long character key */
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
        for (size_t i = 0; i < MR_ARRAY_NUM(msh_key_map); i++)
        {
            if (strncmp(msh.key_buf, msh_key_map[i].key, msh.key_bytes) == 0)
            {
                msh_key_map[i].fn();
                break;
            }
        }
        msh.key_bytes = 0;
    }
}

/**
 * @brief This function input from the msh.
 *
 * @param c The buffer to receive.
 *
 * @return The actual input size.
 */
MR_WEAK int mr_msh_input(char *c)
{
    /* Try to open the serial port */
    if (msh.desc == -1)
    {
#ifndef MR_CFG_MSH_DEV_NAME
#define MR_CFG_MSH_DEV_NAME             "serial1"
#endif /* MR_CFG_MSH_DEV_NAME */
#ifndef MR_CFG_MSH_NONBLOCKING
        int ret = mr_dev_open(MR_CFG_MSH_DEV_NAME, MR_O_RDWR);
#else
        int ret = mr_dev_open(MR_CFG_MSH_DEV_NAME, MR_O_RDWR | MR_O_NONBLOCK);
#endif /* MR_CFG_MSH_NONBLOCKING */
        if (ret < 0)
        {
            return ret;
        }
        msh.desc = ret;
    }

    /* Read data from the serial port */
    return (int)mr_dev_read(msh.desc, c, sizeof(*c));
}

/**
 * @brief This function printf output to the msh.
 *
 * @param buf The buffer to receive.
 * @param size The size of the buffer.
 *
 * @return The actual output size.
 */
MR_WEAK int mr_msh_printf_output(const char *buf, size_t size)
{
    /* Try to open the serial port */
    if (msh.desc == -1)
    {
#ifndef MR_CFG_MSH_NONBLOCKING
        int ret = mr_dev_open(MR_CFG_MSH_DEV_NAME, MR_O_RDWR);
#else
        int ret = mr_dev_open(MR_CFG_MSH_DEV_NAME, MR_O_RDWR | MR_O_NONBLOCK);
#endif /* MR_CFG_MSH_NONBLOCKING */
        if (ret < 0)
        {
            return ret;
        }
        msh.desc = ret;
    }

    /* Write data to the serial port */
    return (int)mr_dev_write(msh.desc, buf, size);
}

/**
 * @brief This function printf to the msh.
 *
 * @param fmt The format string.
 * @param ... The arguments.
 *
 * @return The actual output size.
 */
int mr_msh_printf(const char *fmt, ...)
{
#ifndef MR_CFG_MSH_PRINTF_BUFSZ
#define MR_CFG_MSH_PRINTF_BUFSZ         (128)
#endif /* MR_CFG_MSH_PRINTF_BUFSZ */
    static char buf[MR_CFG_MSH_PRINTF_BUFSZ] = {0};
    va_list args;

    /* Format the string */
    va_start(args, fmt);
    int ret = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    /* Output the string */
    return mr_msh_printf_output(buf, ret);
}

/**
 * @brief This function sets the msh prompt.
 *
 * @param prompt The prompt string.
 */
void mr_msh_set_prompt(char *prompt)
{
    msh.prompt = prompt;
}

/**
 * @brief This function handles the msh.
 */
void mr_msh_handle(void)
{
    char c;
    while (mr_msh_input(&c) > 0)
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
}

/**
 * @brief This function initialize the msh.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
static void mr_msh_init(void)
{
    msh.cursor = 0;
    msh.bytes = 0;
    msh.key_bytes = 0;
#ifdef MR_USING_MSH_ECHO
    msh.echo = MR_ENABLE;
#endif /* MR_USING_MSH_ECHO */
    msh.history_index = 0;
    msh.history_len = 0;
    msh.history_pos = 0;
    msh.prompt = "/";
    msh.desc = -1;
    /* Print the prompt */
    mr_msh_printf(MSH_CLEAR_SCREEN);
    msh_new_current_line();
}
MR_INIT_DEV_EXPORT(mr_msh_init);

static void msh_cmd_help(int argc, void *argv)
{
    for (const struct mr_msh_cmd *msh_cmd = ((&_mr_msh_cmd_start) + 1); msh_cmd < &_mr_msh_cmd_end; msh_cmd++)
    {
        mr_msh_printf("%-*s - %s\r\n", 16, msh_cmd->name, msh_cmd->help);
    }
}

static void msh_cmd_clear(int argc, void *argv)
{
    mr_msh_printf(MSH_CLEAR_SCREEN);
}

static void msh_cmd_logo(int argc, void *argv)
{
    mr_msh_printf(" __  __                  _   _   _                                 \r\n");
    mr_msh_printf("|  \\/  |  _ __          | | (_) | |__    _ __    __ _   _ __   _   _\r\n");
    mr_msh_printf("| |\\/| | | '__|  _____  | | | | | '_ \\  | '__|  / _` | | '__| | | | |\r\n");
    mr_msh_printf("| |  | | | |    |_____| | | | | | |_) | | |    | (_| | | |    | |_| |\r\n");
    mr_msh_printf("|_|  |_| |_|            |_| |_| |_.__/  |_|     \\__,_| |_|     \\__, |\r\n");
    mr_msh_printf("                                                               |___/\r\n");
}

static void msh_cmd_echo(int argc, void *argv)
{
    if (argc == 1)
    {
        goto usage;
    }

    if (strncmp(MR_MSH_GET_ARG(1), "on", 2) == 0)
    {
        msh.echo = MR_ENABLE;
        return;
    } else if (strncmp(MR_MSH_GET_ARG(1), "off", 3) == 0)
    {
        msh.echo = MR_DISABLE;
        return;
    }

    usage:
    mr_msh_printf("usage: echo <on|off>\r\n");
}

/**
 * @brief Exports default MSH commands.
 */
MR_MSH_CMD_EXPORT(help, msh_cmd_help, "show help information.");
MR_MSH_CMD_EXPORT(clear, msh_cmd_clear, "clear the screen.");
MR_MSH_CMD_EXPORT(logo, msh_cmd_logo, "show the logo.");
MR_MSH_CMD_EXPORT(echo, msh_cmd_echo, "enable or disable echo.");

#endif /* MR_USING_MSH */
