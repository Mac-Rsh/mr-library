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
 * @brief Exports a MSH command.
 *
 * @param name The name of the command.
 * @param fn The callback function.
 * @param help The help information.
 */
#define MR_MSH_CMD_EXPORT(name, fn, help) \
    MR_USED const struct mr_msh_cmd _mr_msh_cmd_##name MR_SECTION("mr_msh_cmd.1") = {#name, fn, help};

/**
 * @brief This macro function gets the argument at the specified index.
 *
 * @param index The index of the argument.
 *
 * @note This macro must be called from a function where the first parameter is argc and the second parameter is args.
 *       1 -> argc, 2 -> args.
 */
#define MR_MSH_GET_ARG(index) \
    (((index) <= (argc)) ? (((const char **)(argv))[(index) - 1]) : MR_NULL)

/**
 * @brief MSH printf with color.
 */
#ifdef MR_USING_MSH_PRINTF_COLOR
#define MR_MSH_COLOR_RED(str)           "\033[31m"str"\033[0m"
#define MR_MSH_COLOR_YELLOW(str)        "\033[33m"str"\033[0m"
#define MR_MSH_COLOR_BLUE(str)          "\033[34m"str"\033[0m"
#define MR_MSH_COLOR_PURPLE(str)        "\033[35m"str"\033[0m"
#define MR_MSH_COLOR_GREEN(str)         "\033[32m"str"\033[0m"
#else
#define MR_MSH_COLOR_RED(str)           str
#define MR_MSH_COLOR_YELLOW(str)        str
#define MR_MSH_COLOR_BLUE(str)          str
#define MR_MSH_COLOR_PURPLE(str)        str
#define MR_MSH_COLOR_GREEN(str)         str
#endif /* MR_USING_MSH_PRINTF_COLOR */

/**
 * @addtogroup Msh.
 * @{
 */
int mr_msh_printf_output(const char *buf, size_t size);
int mr_msh_input(char *c);
int mr_msh_printf(const char *fmt, ...);
void mr_msh_set_prompt(char *prompt);
void mr_msh_handle(void);
/** @} */
#endif /* MR_USING_MSH */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_MSH_H_ */
