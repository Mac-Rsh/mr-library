/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 * @date 2024-05-07    MacRsh       Modify fifo implementation
 */

#include "../mr-library/include/mr_api.h"
#include <stdio.h>

static volatile int _mr_critical_level = 0;

static void start(void)
{
}
MR_INIT_EXPORT(start, "0");

static void end(void)
{
}
MR_INIT_EXPORT(end, "5.end");

/**
 * @brief This function is auto initialized.
 */
void mr_auto_init(void)
{
    /* Auto-initialization */
    for (const mr_init_fn_t *fn = &_mr_auto_init_start; fn < &_mr_auto_init_end;
         fn++)
    {
        (*fn)();
    }
}

/**
 * @brief This function disable the interrupt.
 */
MR_WEAK void mr_interrupt_disable(void)
{
}

/**
 * @brief This function enable the interrupt.
 */
MR_WEAK void mr_interrupt_enable(void)
{
}

/**
 * @brief This function enter the critical section.
 */
void mr_critical_enter(void)
{
    if (_mr_critical_level == 0)
    {
        mr_interrupt_disable();
        _mr_critical_level++;
    }
}

/**
 * @brief This function exit the critical section.
 */
void mr_critical_exit(void)
{
    if (_mr_critical_level > 0)
    {
        _mr_critical_level--;
        if (_mr_critical_level == 0)
        {
            mr_interrupt_enable();
        }
    }
}


/**
 * @brief This function delay us.
 *
 * @param us The us to delay.
 */
MR_WEAK void mr_delay_us(size_t us)
{
#ifndef MR_CFG_SYSCLK_FREQ
#define MR_CFG_SYSCLK_FREQ (72000000)
#endif /* MR_CFG_SYSCLK_FREQ */
    for (volatile size_t i = 0; i < us * (MR_CFG_SYSCLK_FREQ / 1000000); i++)
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
    for (size_t i = 0; i < ms; i++)
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
MR_WEAK int mr_printf_output(const char *buf, size_t size)
{
    static int descriptor = -1;

    /* Try to open the serial device */
    if (descriptor == -1)
    {
#ifndef MR_CFG_PRINTF_NAME
#define MR_CFG_PRINTF_NAME              ("serial1")
#endif /* MR_CFG_PRINTF_NAME */
        int ret = mr_device_open(MR_CFG_PRINTF_NAME, MR_FLAG_WRONLY);
        if (ret < 0)
        {
            return ret;
        }
        descriptor = ret;
    }

    /* Write data to the device */
    return (int)mr_device_write(descriptor, buf, size);
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
#define MR_CFG_PRINTF_BUF_SIZE          (256)
#endif /* MR_CFG_PRINTF_BUF_SIZE */
    char buf[MR_CFG_PRINTF_BUF_SIZE] = {0};
    va_list args;

    /* Format the string */
    va_start(args, fmt);
    int ret = vsnprintf(buf, sizeof(buf), fmt, args);
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
MR_WEAK int mr_log_printf_output(const char *buf, size_t size)
{
    static int descriptor = -1;

    /* Try to open the serial device */
    if (descriptor == -1)
    {
#ifndef MR_CFG_LOG_PRINTF_NAME
#define MR_CFG_LOG_PRINTF_NAME          ("serial1")
#endif /* MR_CFG_LOG_PRINTF_NAME */
        int ret = mr_device_open(MR_CFG_LOG_PRINTF_NAME, MR_FLAG_WRONLY);
        if (ret < 0)
        {
            return ret;
        }
        descriptor = ret;
    }

    /* Write data to the device */
    return (int)mr_device_write(descriptor, buf, size);
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
int mr_log_printf(const char *tag, const char *fmt,  ...)
{
#ifndef MR_CFG_LOG_PRINTF_BUF_SIZE
#define MR_CFG_LOG_PRINTF_BUF_SIZE      (256)
#endif /* MR_CFG_LOG_PRINTF_BUF_SIZE */
    char buf[MR_CFG_LOG_PRINTF_BUF_SIZE] = {0};
    va_list args;

    if (strcmp(tag, "null") == 0)
    {
        return 0;
    }

    /* Format the string */
    va_start(args, fmt);
    int ret = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Output the string */
    return mr_log_printf_output(buf, ret);
}

/**
 * @brief This function assert handler.
 *
 * @param ex The assert expression.
 * @param tag The assert tag.
 * @param fn The assert function.
 * @param file The assert file.
 * @param line The assert line.
 */
MR_WEAK void mr_assert_handler(const char *ex, const char *tag, const char *fn,
                               const char *file, int line)
{
    mr_log_printf("[A/%s] : %s %s %s:%d\n", ex, tag, fn, file, line);

    while (1)
    {
    }
}

/**
 * @brief This function initializes a fifo.
 *
 * @param fifo The fifo.
 * @param buf The fifo buffer.
 * @param size The fifo buffer size.
 *
 * @return The error code.
 */
int mr_fifo_init(struct mr_fifo *fifo, void *buf, size_t size)
{
    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (size == 0));

    /* Initialize the fifo */
    fifo->in = 0;
    fifo->out = 0;
    fifo->dynamic = false;
    fifo->buf = buf;
    fifo->size = size;
    return MR_EOK;
}

/**
 * @brief This function resets a fifo.
 *
 * @param fifo The fifo.
 */
void mr_fifo_reset(struct mr_fifo *fifo)
{
    MR_ASSERT(fifo != NULL);

    fifo->in = 0;
    fifo->out = 0;
}

/**
 * @brief This function allocates a fifo.
 *
 * @param fifo The fifo.
 * @param size The fifo buffer size.
 *
 * @return The error code.
 */
int mr_fifo_allocate(struct mr_fifo *fifo, size_t size)
{
    MR_ASSERT(fifo != NULL);

    /* Free the old buffer, if buffer is dynamic */
    if ((fifo->dynamic == true) && (fifo->buf != NULL))
    {
        mr_free(fifo->buf);
        mr_fifo_init(fifo, NULL, 0);
    }

    /* If not allocate a new buffer */
    if (size == 0)
    {
        return MR_EOK;
    }

    /* Allocate a new buffer */
    void *buf = mr_malloc(size);
    if (buf == NULL)
    {
        return MR_ENOMEM;
    }
    mr_fifo_init(fifo, buf, size);
    fifo->dynamic = true;
    return MR_EOK;
}

/**
 * @brief This function frees a fifo.
 *
 * @param fifo The fifo.
 */
void mr_fifo_free(struct mr_fifo *fifo)
{
    MR_ASSERT(fifo != NULL);

    /* Free the old buffer, if buffer is dynamic */
    if (fifo->dynamic == true)
    {
        mr_free(fifo->buf);
        mr_fifo_init(fifo, NULL, 0);
    }
}

/**
 * @brief This function gets the used space of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The used space.
 */
size_t mr_fifo_used_get(const struct mr_fifo *fifo)
{
    MR_ASSERT(fifo != NULL);

    uint32_t in = fifo->in, out = fifo->out;
    size_t used;

    /* Calculate the used space */
    if (in >= out)
    {
        used = in - out;
    } else
    {
        used = fifo->size - (out - in);
    }

    /* Return the used space */
    return used;
}

/**
 * @brief This function gets the free space of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The free space.
 */
size_t mr_fifo_space_get(const struct mr_fifo *fifo)
{
    MR_ASSERT(fifo != NULL);

    uint32_t in = fifo->in, out = fifo->out;
    uint32_t free;

    /* Calculate the free space */
    if (in >= out)
    {
        free = fifo->size - (in - out);
    } else
    {
        free = out - in;
    }

    /* Return the free space */
    return free - 1;
}

/**
 * @brief This function gets the size of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The size.
 */
size_t mr_fifo_size_get(const struct mr_fifo *fifo)
{
    MR_ASSERT(fifo != NULL);

    /* Return the buffer size that can be used */
    return (fifo->size == 0) ? 0 : fifo->size - 1;
}

/**
 * @brief This function peeks data from a fifo.
 *
 * @param fifo The fifo.
 * @param buf The buffer to store the data.
 * @param count The number of bytes to peek.
 *
 * @return The number of bytes peeked.
 */
size_t mr_fifo_peek(const struct mr_fifo *fifo, void *buf, size_t count)
{
    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    uint8_t *_buf = (uint8_t *)buf;

    /* Get used space, limit by count */
    uint32_t used = mr_fifo_used_get(fifo);
    if (used < count)
    {
        count = used;
    }
    if (count == 0)
    {
        return 0;
    }

    /* Peek data */
    uint32_t out = fifo->out;
    uint32_t end = fifo->size - out;
    if (end > count)
    {
        /* If there is enough data at the end, peek it all at once */
        memcpy(_buf, &fifo->buf[out], count);
    } else
    {
        /* If there is not enough data at the end, peek it in two parts */
        memcpy(_buf, &fifo->buf[out], end);
        memcpy(&_buf[end], fifo->buf, count - end);
    }

    /* Return the number of bytes peeked */
    return count;
}

/**
 * @brief This function discards data from a fifo.
 *
 * @param fifo The fifo.
 * @param count The number of bytes to discard.
 *
 * @return The number of bytes discarded.
 */
size_t mr_fifo_discard(struct mr_fifo *fifo, size_t count)
{
    MR_ASSERT(fifo != NULL);

    /* Get used space, limit by count */
    uint32_t used = mr_fifo_used_get(fifo);
    if (used < count)
    {
        count = used;
    }
    if (count == 0)
    {
        return 0;
    }

    /* Discard data */
    uint32_t out = fifo->out;
    uint32_t end = fifo->size - out;
    if (end > count)
    {
        /* If there is enough data at the end, discard it all at once */
        out += count;
    } else
    {
        /* If there is not enough data at the end, discard it in two parts */
        out = count - end;
    }

    /* Update output index */
    fifo->out = (out >= fifo->size) ? 0 : out;

    /* Return the number of bytes discarded */
    return count;
}

/**
 * @brief This function reads data from a fifo.
 *
 * @param fifo The fifo.
 * @param buf The buffer to store the data.
 * @param count The number of bytes to read.
 *
 * @return The number of bytes read.
 */
size_t mr_fifo_read(struct mr_fifo *fifo, void *buf, size_t count)
{
    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    uint8_t *_buf = (uint8_t *)buf;

    /* Get used space, limit by count */
    uint32_t used = mr_fifo_used_get(fifo);
    if (used < count)
    {
        count = used;
    }
    if (count == 0)
    {
        return 0;
    }

    /* Read data */
    uint32_t out = fifo->out;
    uint32_t end = fifo->size - out;
    if (end > count)
    {
        /* If there is enough data at the end, read it all at once */
        memcpy(_buf, &fifo->buf[out], count);
        out += count;
    } else
    {
        /* If there is not enough data at the end, read it in two parts */
        memcpy(_buf, &fifo->buf[out], end);
        memcpy(&_buf[end], fifo->buf, count - end);
        out = count - end;
    }

    /* Update output index */
    fifo->out = (out >= fifo->size) ? 0 : out;

    /* Return the number of bytes read */
    return count;
}

/**
 * @brief This function writes data to a fifo.
 *
 * @param fifo The fifo.
 * @param buf The buffer to store the data.
 * @param count The number of bytes to write.
 *
 * @return The number of bytes written.
 */
size_t mr_fifo_write(struct mr_fifo *fifo, const void *buf, size_t count)
{
    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    uint8_t *_buf = (uint8_t *)buf;

    /* Get free space, limit by count */
    uint32_t free = mr_fifo_space_get(fifo);
    if (free < count)
    {
        count = free;
    }
    if (count == 0)
    {
        return 0;
    }

    /* Write data */
    uint32_t in = fifo->in;
    uint32_t end = fifo->size - in;
    if (end > count)
    {
        /* If there is enough space at the end, write it all at once */
        memcpy(&fifo->buf[in], _buf, count);
        in += count;
    } else
    {
        /* If there is not enough space at the end, write it in two parts */
        memcpy(&fifo->buf[in], _buf, end);
        memcpy(fifo->buf, &_buf[end], count - end);
        in = count - end;
    }

    /* Update input index */
    fifo->in = (in >= fifo->size) ? 0 : in;

    /* Return the number of bytes written */
    return count;
}

/**
 * @brief This function force write data to a fifo.
 *
 * @param fifo The fifo.
 * @param buf The buffer to store the data.
 * @param count The number of bytes to write.
 *
 * @return The number of bytes written.
 *
 * @note When you use this function, the single read single write condition of
 *       an unlocked fifo may not be met.
 */
size_t mr_fifo_write_force(struct mr_fifo *fifo, const void *buf, size_t count)
{
    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    uint8_t *_buf = (uint8_t *)buf;

    /* If there is no data to write, return immediately */
    if (count == 0)
    {
        return 0;
    }

    /* Skip data that exceeds the size */
    uint32_t size = mr_fifo_size_get(fifo);
    if (count > size)
    {
        _buf = &_buf[count - size];
        count = size;
    }

    /* Discard data that will be overwritten */
    uint32_t free = mr_fifo_space_get(fifo);
    if (free < count)
    {
        mr_fifo_discard(fifo, count - free);
    }

    /* Write data */
    return mr_fifo_write(fifo, _buf, count);
}
