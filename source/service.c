/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include <include/mr_library.h>

static int start(void)
{
    return MR_EOK;
}
__MR_INIT_EXPORT("start", start, "0");

static int end(void)
{
    return MR_EOK;
}
__MR_INIT_EXPORT("end", end, "5.end");

/**
 * @brief This function is auto initialized.
 */
void mr_auto_init(void)
{
    const mr_init_item_t *item;

    for (item = &_mr_init_item_start; item <= &_mr_init_item_end; item++)
    {
        /* Skip null function */
        if (item->fn == NULL)
        {
            continue;
        }

        /* Call initialization function */
        item->fn();
    }
}

/**
 * @brief This function disable interrupt.
 */
MR_WEAK size_t mr_irq_disable(void)
{
    return 0;
}

/**
 * @brief This function enable interrupt.
 *
 * @param mask The interrupt mask.
 */
MR_WEAK void mr_irq_enable(MR_UNUSED size_t mask)
{

}

/**
 * @brief This function delay us.
 *
 * @param us The us to delay.
 */
MR_WEAK void mr_delay_us(size_t us)
{
    volatile size_t i;

#ifndef MR_CFG_SYSCLK_FREQ
#define MR_CFG_SYSCLK_FREQ                  (100000000)
#endif /* MR_CFG_SYSCLK_FREQ */
    for (i = 0; i < us * (MR_CFG_SYSCLK_FREQ / 1000000); i += 1)
    {
        __asm__("nop");
    }
#undef MR_CFG_SYSCLK_FREQ
}

/**
 * @brief This function delay ms.
 *
 * @param ms The ms to delay.
 */
MR_WEAK void mr_delay_ms(size_t ms)
{
    size_t i;

    for (i = 0; i < ms; i++)
    {
        mr_delay_us(1000);
    }
}

/**
 * @brief This function returns the error message.
 *
 * @param error The error code.
 *
 * @return The error message.
 */
const char *mr_strerror(int error)
{
    switch (error)
    {
        case MR_EOK:
        {
            return "no error";
        }
        case MR_EPERM:
        {
            return "operation not permitted";
        }
        case MR_ENOENT:
        {
            return "no such file or directory";
        }
        case MR_EIO:
        {
            return "input/output error";
        }
        case MR_ENOMEM:
        {
            return "Out of resources";
        }
        case MR_EACCES:
        {
            return "permission denied";
        }
        case MR_EBUSY:
        {
            return "resource busy";
        }
        case MR_EEXIST:
        {
            return "resource already exists";
        }
        case MR_EINVAL:
        {
            return "invalid argument";
        }
        case MR_ETIMEOUT:
        {
            return "operation timed out";
        }
        case MR_EAGAIN:
        {
            return "resource temporarily unavailable";
        }
        default:
        {
            return "unknown error";
        }
    }
}

/**
 * @brief This function printf output.
 *
 * @param buf The buffer to output.
 * @param size The size of the buffer.
 *
 * @return The size of the actual output, otherwise an error code.
 */
MR_WEAK int mr_printf_output(MR_UNUSED const char *buf, MR_UNUSED size_t size)
{
    return 0;
}

/**
 * @brief This function printf.
 *
 * @param fmt The format string.
 * @param ... The arguments.
 *
 * @return The actual output size.
 */
int mr_printf(const char *fmt, ...)
{
#ifndef MR_CFG_PRINTF_BUF_SIZE
#define MR_CFG_PRINTF_BUF_SIZE              (256)
#endif /* MR_CFG_PRINTF_BUF_SIZE */
    char buf[MR_CFG_PRINTF_BUF_SIZE];
    va_list args;
    int ret;

    /* Format the string */
    va_start(args, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Output the string */
    return mr_printf_output(buf, ret);
}

/**
 * @brief This function log printf output.
 *
 * @param buf The buffer to output.
 * @param size The size of the buffer.
 *
 * @return The size of the actual output, otherwise an error code.
 */
MR_WEAK int mr_log_printf_output(MR_UNUSED const char *buf, MR_UNUSED size_t size)
{
    return 0;
}

/**
 * @brief This function log printf.
 *
 * @param tag The log tag.
 * @param fmt The format string.
 * @param ... The arguments.
 *
 * @return The actual output size.
 */
int mr_log_printf(const char *tag, const char *fmt, ...)
{
#ifndef MR_CFG_LOG_PRINTF_BUF_SIZE
#define MR_CFG_LOG_PRINTF_BUF_SIZE          (256)
#endif /* MR_CFG_LOG_PRINTF_BUF_SIZE */
    char buf[MR_CFG_LOG_PRINTF_BUF_SIZE];
    va_list args;
    int ret;

    /* Filter the log tag */
    if (strcmp(tag, "null") == 0)
    {
        return 0;
    }

    /* Format the string */
    va_start(args, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Output the string */
    return mr_log_printf_output(buf, ret);
}
