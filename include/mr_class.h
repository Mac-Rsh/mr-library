/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-07-27    MacRsh       First version
 */

#ifndef __MR_CLASS_H__
#define __MR_CLASS_H__

#include <include/mr_def.h>
#include <include/mr_memory.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup Class
 * @{
 */

/**
 * @brief Class structure.
 */
struct mr_class
{
    const char *name;                                                                                   /**< Class name */
    void *(*__init__)(void *self, ...);                                                                 /**< Init function */
    void *(*__del__)(void *self, ...);                                                                  /**< Delete function */
};

/**
 * @brief This macro import a class.
 * 
 * @param cls The class.
 */
#define MR_IMPORT(cls)                      extern const struct mr_class cls;

/**
 * @brief This macro defines a class.
 * 
 * @param name The name of the class.
 * 
 * @code 
 * MR_CLASS(String)
 * {
 *     const char *str;
 *     size_t len;
 * };
 * @endcode
 */
#define MR_CLASS(name)                                                                             \
    void *name##__init__(void *self, ...);                                                         \
    void *name##__del__(void *self, ...);                                                          \
    const struct mr_class name = {#name, name##__init__, name##__del__};                           \
    struct name

/**
 * @brief This macro defines an implementation of a class.
 * 
 * @param fn The function name.
 * @param cls The class.
 * 
 * @code 
 * MR_IMPL(__init__, String)
 * {
 *     struct String *string = self;
 *     va_list args;
 *     va_start(args, self);
 *     string->str = va_arg(args, const char *);
 *     string->len = va_arg(args, int);
 *     va_end(args);
 *     return self;
 * }
 * @endcode
 */
#define MR_IMPL(fn, cls)                    void *cls##fn(MR_UNUSED void *self, ...)

/**
 * @brief This macro defines an unimplemented function of a class.
 * 
 * @param fn The function name.
 * @param cls The class.
 * 
 * @code 
 * MR_UNIMPL(__del__, String)
 * @endcode
 */
#define MR_UNIMPL(fn, cls)                  void *cls##fn(void *self, ...){return self;}

/**
 * @brief This macro function creates a new instance of a class.
 * 
 * @param cls The class.
 * @param ... The arguments.
 * 
 * @return A pointer to the new instance.
 */
#define MR_NEW(cls, ...)                    (cls.__init__(mr_malloc(sizeof(cls)), ##__VA_ARGS__))

/**
 * @brief This macro function deletes an instance of a class.
 * 
 * @param cls The class.
 * @param self The instance.
 */
#define MR_DEL(cls, self)                   (mr_free(cls.__del__(self, NULL)), self = NULL)

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_CLASS_H__ */
