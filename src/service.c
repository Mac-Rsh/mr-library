/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include "mr_api.h"

static int start(void)
{
    return 0;
}
MR_INIT_EXPORT(start, "0");

static int end(void)
{
    return 0;
}
MR_INIT_EXPORT(end, "5.end");

/**
 * @brief This function is auto initialized.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
void mr_auto_init(void)
{
    volatile const mr_init_fn_t *fn = MR_NULL;

    /* Auto-initialization */
    for (fn = &_mr_auto_init_start; fn < &_mr_auto_init_end; fn++)
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
 * @brief This function allocate memory.
 *
 * @param size The size of the memory.
 *
 * @return The allocated memory.
 */
MR_WEAK void *mr_malloc(size_t size)
{
    return malloc(size);
}

/**
 * @brief This function free memory.
 *
 * @param memory The memory to free.
 */
MR_WEAK void mr_free(void *memory)
{
    free(memory);
}

/**
 * @brief This function delay us.
 *
 * @param us The delay time.
 */
MR_WEAK void mr_delay_us(uint32_t us)
{
    volatile uint32_t i = 0;

#ifndef MR_CFG_SYSCLK_FREQ
#define MR_CFG_SYSCLK_FREQ              (72000000)
#endif /* MR_CFG_SYSCLK_FREQ */
#if (MR_CFG_SYSCLK_FREQ > 1000000)
#define MR_DELAY_COUNT                  (MR_CFG_SYSCLK_FREQ / 1000000)
#else
#define MR_DELAY_COUNT                  (1)
#endif /* (MR_CFG_SYSCLK_FREQ > 1000000) */
    for (i = 0; i < us * MR_DELAY_COUNT; i++)
    {
        __asm__("nop");
    }
#undef MR_DELAY_COUNT
#undef MR_CFG_SYSCLK_FREQ
}

/**
 * @brief This function delay ms.
 *
 * @param ms The delay time.
 */
MR_WEAK void mr_delay_ms(uint32_t ms)
{
    volatile uint32_t i = 0;

    for (i = 0; i < ms; i++)
    {
        mr_delay_us(1000);
    }
}

/**
 * @brief This function printf output.
 *
 * @param buf The buffer to output.
 * @param size The size of the buffer.
 *
 * @return The actual output size.
 */
MR_WEAK int mr_printf_output(const char *buf, size_t size)
{
#ifdef MR_CFG_CONSOLE_NAME
    static int console = -1;

    if (console < 0)
    {
        console = mr_dev_open(MR_CFG_CONSOLE_NAME, MR_OFLAG_RDWR);
        if (console < 0)
        {
            return console;
        }
    }
    return (int)mr_dev_write(console, buf, size);
#else
    return 0;
#endif /* MR_CFG_CONSOLE_NAME */
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
    char buf[256] = {0};
    va_list args = MR_NULL;

    va_start(args, fmt);
    int ret = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    ret = mr_printf_output(buf, ret);
    va_end(args);
    return ret;
}

/**
 * @brief This function get the error message.
 *
 * @param err The error code.
 *
 * @return The error message.
 */
const char *mr_strerror(int err)
{
    switch (err)
    {
        case MR_EOK:
            return "no error";
        case MR_ENOMEM:
            return "no enough memory";
        case MR_EIO:
            return "I/O error";
        case MR_ENOTFOUND:
            return "not found";
        case MR_EBUSY:
            return "resource busy";
        case MR_EEXIST:
            return "exists";
        case MR_ENOTSUP:
            return "operation not supported";
        case MR_EINVAL:
            return "invalid argument";
        default:
            return "unknown error";
    }
}

/**
 * @brief This function initialize the ringbuffer.
 *
 * @param ringbuf The ringbuffer to initialize.
 * @param pool The pool of buf.
 * @param size The size of the pool.
 */
void mr_ringbuf_init(struct mr_ringbuf *ringbuf, void *pool, size_t size)
{
    mr_assert(ringbuf != MR_NULL);
    mr_assert((pool != MR_NULL || size == 0));

    ringbuf->read_index = 0;
    ringbuf->write_index = 0;
    ringbuf->read_mirror = 0;
    ringbuf->write_mirror = 0;

    ringbuf->size = size;
    ringbuf->buffer = pool;
}

/**
 * @brief This function allocate memory for the ringbuffer.
 *
 * @param ringbuf The ringbuffer to allocate.
 * @param size The size of the memory.
 *
 * @return MR_ERR_OK on success, otherwise an error code.
 */
int mr_ringbuf_allocate(struct mr_ringbuf *ringbuf, size_t size)
{
    void *pool = MR_NULL;

    mr_assert(ringbuf != MR_NULL);

    /* Check the buffer size */
    if (mr_ringbuf_get_bufsz(ringbuf) == size)
    {
        mr_ringbuf_reset(ringbuf);
        return MR_EOK;
    }

    /* Free old buffer */
    if (ringbuf->size != 0)
    {
        mr_free(ringbuf->buffer);
    }

    /* Allocate new buffer */
    pool = mr_malloc(size);
    if (pool == MR_NULL && size != 0)
    {
        return MR_ENOMEM;
    }
    mr_ringbuf_init(ringbuf, pool, size);

    return MR_EOK;
}

/** 
 * @brief This function free the ringbuffer.
 * 
 * @param ringbuf The ringbuffer to free. 
 */
void mr_ringbuf_free(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    mr_free(ringbuf->buffer);
    mr_ringbuf_init(ringbuf, MR_NULL, 0);
}

/**
 * @brief This function reset the ringbuffer.
 *
 * @param ringbuf The ringbuffer to reset.
 */
void mr_ringbuf_reset(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    ringbuf->read_index = 0;
    ringbuf->write_index = 0;

    ringbuf->read_mirror = 0;
    ringbuf->write_mirror = 0;
}

/**
 * @brief This function get the buf size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the buf size.
 *
 * @return The buf size.
 */
size_t mr_ringbuf_get_data_size(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    /* Empty or full according to the mirror flag */
    if (ringbuf->read_index == ringbuf->write_index)
    {
        if (ringbuf->read_mirror == ringbuf->write_mirror)
        {
            return 0;
        } else
        {
            return ringbuf->size;
        }
    }

    if (ringbuf->write_index > ringbuf->read_index)
    {
        return ringbuf->write_index - ringbuf->read_index;
    } else
    {
        return ringbuf->size - ringbuf->read_index + ringbuf->write_index;
    }
}

/**
 * @brief This function get the space size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the space size.
 *
 * @return The space size.
 */
size_t mr_ringbuf_get_space_size(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    return ringbuf->size - mr_ringbuf_get_data_size(ringbuf);
}

/**
 * @brief This function get the buffer size from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to get the buffer size.
 *
 * @return  The buffer size.
 */
size_t mr_ringbuf_get_bufsz(struct mr_ringbuf *ringbuf)
{
    mr_assert(ringbuf != MR_NULL);

    return ringbuf->size;
}

/**
 * @brief This function pop the buf from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to pop the buf.
 * @param data The buf to pop.
 *
 * @return The size of the actual pop.
 */
size_t mr_ringbuf_pop(struct mr_ringbuf *ringbuf, uint8_t *data)
{
    /* Get the buf size */
    if (mr_ringbuf_get_data_size(ringbuf) == 0)
    {
        return 0;
    }

    *data = ringbuf->buffer[ringbuf->read_index];

    if (ringbuf->read_index == ringbuf->size - 1)
    {
        ringbuf->read_mirror = ~ringbuf->read_mirror;
        ringbuf->read_index = 0;
    } else
    {
        ringbuf->read_index++;
    }

    return 1;
}

/**
 * @brief This function reads from the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be read.
 * @param buffer The buf buffer to be read from the ringbuffer.
 * @param size The size of the read.
 *
 * @return The size of the actual read.
 */
size_t mr_ringbuf_read(struct mr_ringbuf *ringbuf, void *buffer, size_t size)
{
    uint8_t *read_buffer = (uint8_t *)buffer;
    size_t data_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert(buffer != MR_NULL);

    /* Get the buf size */
    data_size = mr_ringbuf_get_data_size(ringbuf);
    if (data_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to read if it exceeds the available buf */
    if (size > data_size)
    {
        size = data_size;
    }

    /* Copy the buf from the ringbuf to the buffer */
    if ((ringbuf->size - ringbuf->read_index) > size)
    {
        memcpy(read_buffer, &ringbuf->buffer[ringbuf->read_index], size);
        ringbuf->read_index += size;

        return size;
    }

    memcpy(read_buffer, &ringbuf->buffer[ringbuf->read_index], ringbuf->size - ringbuf->read_index);
    memcpy(&read_buffer[ringbuf->size - ringbuf->read_index],
           &ringbuf->buffer[0],
           size - (ringbuf->size - ringbuf->read_index));

    ringbuf->read_mirror = ~ringbuf->read_mirror;
    ringbuf->read_index = size - (ringbuf->size - ringbuf->read_index);

    return size;
}

/**
 * @brief This function push the buf to the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be pushed.
 * @param data The buf to be pushed.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_push(struct mr_ringbuf *ringbuf, uint8_t data)
{
    /* Get the space size */
    if (mr_ringbuf_get_space_size(ringbuf) == 0)
    {
        return 0;
    }

    ringbuf->buffer[ringbuf->write_index] = data;

    if (ringbuf->write_index == ringbuf->size - 1)
    {
        ringbuf->write_mirror = ~ringbuf->write_mirror;
        ringbuf->write_index = 0;
    } else
    {
        ringbuf->write_index++;
    }

    return 1;
}

/**
 * @brief This function force to push the buf to the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be pushed.
 * @param data The buf to be pushed.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_push_force(struct mr_ringbuf *ringbuf, uint8_t data)
{
    int state = 0;

    /* Get the buffer size */
    if (mr_ringbuf_get_bufsz(ringbuf) == 0)
    {
        return 0;
    }

    /* Get the space size */
    if (mr_ringbuf_get_space_size(ringbuf) == 0)
    {
        state = 1;
    }

    ringbuf->buffer[ringbuf->write_index] = data;

    if (ringbuf->write_index == ringbuf->size - 1)
    {
        ringbuf->write_mirror = ~ringbuf->write_mirror;
        ringbuf->write_index = 0;
        if (state == 1)
        {
            ringbuf->read_mirror = ~ringbuf->read_mirror;
            ringbuf->read_index = ringbuf->write_index;
        }
    } else
    {
        ringbuf->write_index++;
        if (state == 1)
        {
            ringbuf->read_index = ringbuf->write_index;
        }
    }

    return 1;
}

/**
 * @brief This function write the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be written.
 * @param buffer The buf buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_write(struct mr_ringbuf *ringbuf, const void *buffer, size_t size)
{
    uint8_t *write_buffer = (uint8_t *)buffer;
    size_t space_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert(buffer != MR_NULL);

    /* Get the space size */
    space_size = mr_ringbuf_get_space_size(ringbuf);
    if (space_size == 0)
    {
        return 0;
    }

    /* Adjust the number of bytes to write if it exceeds the available buf */
    if (size > space_size)
    {
        size = space_size;
    }

    /* Copy the buf from the buffer to the ringbuf */
    if ((ringbuf->size - ringbuf->write_index) > size)
    {
        memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, size);
        ringbuf->write_index += size;

        return size;
    }

    memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, ringbuf->size - ringbuf->write_index);
    memcpy(&ringbuf->buffer[0],
           &write_buffer[ringbuf->size - ringbuf->write_index],
           size - (ringbuf->size - ringbuf->write_index));

    ringbuf->write_mirror = ~ringbuf->write_mirror;
    ringbuf->write_index = size - (ringbuf->size - ringbuf->write_index);

    return size;
}

/**
 * @brief This function force write the ringbuffer.
 *
 * @param ringbuf The ringbuffer to be written.
 * @param buffer The buf buffer to be written to ringbuffer.
 * @param size The size of write.
 *
 * @return The size of the actual write.
 */
size_t mr_ringbuf_write_force(struct mr_ringbuf *ringbuf, const void *buffer, size_t size)
{
    uint8_t *write_buffer = (uint8_t *)buffer;
    size_t space_size = 0;

    mr_assert(ringbuf != MR_NULL);
    mr_assert(buffer != MR_NULL);

    if ((mr_ringbuf_get_bufsz(ringbuf) == 0) || (size == 0))
    {
        return 0;
    }

    /* Get the space size */
    space_size = mr_ringbuf_get_space_size(ringbuf);

    /* If the buf exceeds the buffer space_size, the front buf is discarded */
    if (size > ringbuf->size)
    {
        write_buffer = &write_buffer[size - ringbuf->size];
        size = ringbuf->size;
    }

    /* Copy the buf from the buffer to the ringbuf */
    if ((ringbuf->size - ringbuf->write_index) > size)
    {
        memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, size);
        ringbuf->write_index += size;
        if (size > space_size)
        {
            ringbuf->read_index = ringbuf->write_index;
        }

        return size;
    }

    memcpy(&ringbuf->buffer[ringbuf->write_index], write_buffer, ringbuf->size - ringbuf->write_index);
    memcpy(&ringbuf->buffer[0],
           &write_buffer[ringbuf->size - ringbuf->write_index],
           size - (ringbuf->size - ringbuf->write_index));

    ringbuf->write_mirror = ~ringbuf->write_mirror;
    ringbuf->write_index = size - (ringbuf->size - ringbuf->write_index);

    if (size > space_size)
    {
        if (ringbuf->write_index <= ringbuf->read_index)
        {
            ringbuf->read_mirror = ~ringbuf->read_mirror;
        }

        ringbuf->read_index = ringbuf->write_index;
    }

    return size;
}
