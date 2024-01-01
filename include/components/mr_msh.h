/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-12-25    MacRsh       First version
 */

#ifndef _MR_MSH_H_
#define _MR_MSH_H_

#include "include/mr_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_MSH

/**
 * @brief MSH command structure.
 */
struct mr_msh_cmd
{
    const char *name;                                               /**< Name */
    int (*call)(int argc, void *args);                              /**< Callback function */
    const char *help;                                               /**< Help information */
};

/**
 * @brief Exports a MSH command with level.
 *
 * @param name The name of the command.
 * @param fn The callback function.
 * @param help The help information.
 * @param level The level of the command.
 */
#define MR_MSH_EXPORT(name, fn, help, level) \
    MR_USED const struct mr_msh_cmd _mr_msh_cmd_##name MR_SECTION(".mr_msh_cmd."level) = {#name, fn, help};

/**
 * @brief Exports a MSH command.
 *
 * @param name The name of the command.
 * @param fn The callback function.
 * @param help The help information.
 */
#define MR_MSH_CMD_EXPORT(name, fn, help) \
    MR_MSH_EXPORT(name, fn, help, "1")

/**
 * @brief This macro function gets the argument at the specified index.
 *
 * @param index The index of the argument.
 *
 * @note This macro must be called from a function where the first parameter is argc and the second parameter is args.
 *       1 -> argc, 2 -> args.
 */
#define MR_MSH_GET_ARG(index) \
    (((index) < (argc)) ? (((const char **)(argv))[index]) : MR_NULL)

/**
 * @brief This macro function prints a formatted string.
 *
 * @param fmt The format string.
 * @param ... The arguments.
 */
#define mr_msh_printf(fmt, ...) mr_printf(fmt, ##__VA_ARGS__)

/**
 * @addtogroup Msh.
 * @{
 */
void mr_msh_recv_char(char c);
/** @} */
#else
#define MR_MSH_EXPORT(name, fn, help, level)
#define MR_MSH_CMD_EXPORT(name, fn, help)
#define mr_msh_printf(...)
#endif /* MR_USING_MSH */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_MSH_H_ */
