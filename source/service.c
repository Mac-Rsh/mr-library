/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-20    MacRsh       First version
 */

#include <include/mr_service.h>

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
    const struct mr_init_item *item;

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
