/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include <include/mr_library.h>

/**
 * @brief This function initializes a fifo.
 *
 * @param fifo The fifo.
 * @param buf The fifo buffer.
 * @param size The fifo buffer size.
 *
 * @return The error code.
 */
int mr_fifo_init(mr_fifo_t *fifo, void *buf, size_t size)
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
 * @brief This function allocates a fifo.
 *
 * @param fifo The fifo.
 * @param size The fifo buffer size.
 *
 * @return The error code.
 */
int mr_fifo_allocate(mr_fifo_t *fifo, size_t size)
{
    void *buf;

    MR_ASSERT(fifo != NULL);

    /* Free the old buffer, if buffer is dynamic */
    if ((fifo->dynamic == true) && (fifo->buf != NULL))
    {
        mr_free(fifo->buf);
        mr_fifo_init(fifo, NULL, 0);
    }

    /* Allocate a new buffer */
    if (size != 0)
    {
        buf = mr_malloc(size);
        if (buf == NULL)
        {
            return MR_ENOMEM;
        }
        mr_fifo_init(fifo, buf, size);
        fifo->dynamic = true;
    }
    return MR_EOK;
}

/**
 * @brief This function frees a fifo.
 *
 * @param fifo The fifo.
 */
void mr_fifo_free(mr_fifo_t *fifo)
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
 * @brief This function resets a fifo.
 *
 * @param fifo The fifo.
 */
void mr_fifo_reset(mr_fifo_t *fifo)
{
    MR_ASSERT(fifo != NULL);

    fifo->in = 0;
    fifo->out = 0;
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
size_t mr_fifo_peek(const mr_fifo_t *fifo, void *buf, size_t count)
{
    uint32_t used, out, end;

    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    /* Get used space, limit by count */
    used = mr_fifo_get_used(fifo);
    if (used < count)
    {
        count = used;
    }

    /* Get out and end */
    out = fifo->out;
    end = fifo->size - out;

    /* Peek data */
    if (end > count)
    {
        /* If there is enough data at the end, peek it all at once */
        memcpy((uint8_t *)buf, &fifo->buf[out], count);
    } else
    {
        /* If there is not enough data at the end, peek it in two parts */
        memcpy(buf, &fifo->buf[out], end);
        memcpy(&((uint8_t *)buf)[end], fifo->buf, count - end);
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
size_t mr_fifo_discard(mr_fifo_t *fifo, size_t count)
{
    uint32_t used, out, end;

    MR_ASSERT(fifo != NULL);

    /* Get used space, limit by count */
    used = mr_fifo_get_used(fifo);
    if (used < count)
    {
        count = used;
    }

    /* Get out and end */
    out = fifo->out;
    end = fifo->size - out;

    /* Discard data */
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
size_t mr_fifo_read(mr_fifo_t *fifo, void *buf, size_t count)
{
    uint32_t used, out, end;

    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    /* Get used space, limit by count */
    used = mr_fifo_get_used(fifo);
    if (used < count)
    {
        count = used;
    }

    /* Get out and end */
    out = fifo->out;
    end = fifo->size - out;

    /* Read data */
    if (end > count)
    {
        /* If there is enough data at the end, read it all at once */
        memcpy(buf, &fifo->buf[out], count);
        out += count;
    } else
    {
        /* If there is not enough data at the end, read it in two parts */
        memcpy(buf, &fifo->buf[out], end);
        memcpy(&((uint8_t *)buf)[end], fifo->buf, count - end);
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
size_t mr_fifo_write(mr_fifo_t *fifo, const void *buf, size_t count)
{
    uint32_t free, in, end;

    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    /* Get free space, limit by count */
    free = mr_fifo_get_free(fifo);
    if (free < count)
    {
        count = free;
    }

    /* Get in and end */
    in = fifo->in;
    end = fifo->size - in;

    /* Write data */
    if (end > count)
    {
        /* If there is enough space at the end, write it all at once */
        memcpy(&fifo->buf[in], buf, count);
        in += count;
    } else
    {
        /* If there is not enough space at the end, write it in two parts */
        memcpy(&fifo->buf[in], buf, end);
        memcpy(fifo->buf, &((const uint8_t *)buf)[end], count - end);
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
size_t mr_fifo_write_force(mr_fifo_t *fifo, const void *buf, size_t count)
{
    uint32_t size, free;

    MR_ASSERT(fifo != NULL);
    MR_ASSERT((buf != NULL) || (count == 0));

    /* Skip data that exceeds the size */
    size = mr_fifo_get_size(fifo);
    if (count > size)
    {
        buf = &((const uint8_t *)buf)[count - size];
        count = size;
    }

    /* Discard data that will be overwritten */
    free = mr_fifo_get_free(fifo);
    if (free < count)
    {
        mr_fifo_discard(fifo, count - free);
    }

    /* Write data */
    return mr_fifo_write(fifo, buf, count);
}

/**
 * @brief This function gets the used space of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The used space.
 */
size_t mr_fifo_get_used(const mr_fifo_t *fifo)
{
    uint32_t in, out;

    MR_ASSERT(fifo != NULL);

    /* Get the in and out */
    in = fifo->in;
    out = fifo->out;

    /* Calculate the used space */
    return (in >= out) ? in - out : fifo->size - (out - in);
}

/**
 * @brief This function gets the free space of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The free space.
 */
size_t mr_fifo_get_free(const mr_fifo_t *fifo)
{
    uint32_t in, out;

    MR_ASSERT(fifo != NULL);

    /* Get the in and out */
    in = fifo->in;
    out = fifo->out;

    /* Calculate the free space */
    return (in >= out) ? fifo->size - (in - out) - 1 : out - in - 1;
}

/**
 * @brief This function gets the size of a fifo.
 *
 * @param fifo The fifo.
 *
 * @return The size.
 */
size_t mr_fifo_get_size(const mr_fifo_t *fifo)
{
    MR_ASSERT(fifo != NULL);

    /* Return the buffer size that can be used */
    return (fifo->size == 0) ? 0 : fifo->size - 1;
}
