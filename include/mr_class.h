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
    const char *name;
    void *(*__init__)(void *self, ...);
    void *(*__del__)(void *self);
};

/**
 * @brief This macro defines a class.
 * 
 * @param _name The name of the class.
 * 
 * @code 
 * MR_CLASS(String)
 * {
 *     const char *str;
 * };
 * @endcode
 */
#define MR_CLASS(_name)                                                        \
    extern struct mr_class _name;                                              \
    struct _name

/**
 * @brief This macro defines an interface of a class.
 * 
 * @param _cls The class.
 * 
 * @code 
 * MR_CLASS(String)
 * {
 *     MR_INTERFACE(String)
 *     {
 *         void (*print)(struct String *self);
 *     } *impl;
 *     const char *str;
 * };
 * @endcode
 */
#define MR_INTERFACE(_cls)               struct _cls##intf

/**
 * @brief This macro defines an implementation of a class.
 * 
 * @param _cls The class.
 * @param _fn The function name.
 * @param _ret The return type.
 * @param ... The arguments.
 * 
 * @code 
 * MR_IMPL(String, __init__, void *, const char *str)
 * {
 *     self->str = str;
 *     return self;
 * }
 * @endcode
 */
#define MR_IMPL(_cls, _fn, _ret, ...)                                          \
    static inline _ret _fn(struct _cls *self, ##__VA_ARGS__)

/**
 * @brief This macro defines an implementation of an interface of a class.
 * 
 * @param _cls The class.
 * @param _mbr The interface pointer member name.
 * @param ... The arguments.
 * 
 * @code
 * MR_IMPL(String, __init__, const char *str)
 * {
 *     MR_IMPL_INTERFACE(String, f, print);
 *     self->f = &f;
 *     self->str = str;
 * }
 * @endcode
 */
#define MR_IMPL_INTERFACE(_cls, _mbr, ...)                                     \
    static struct _cls##intf _mbr = {__VA_ARGS__}

/**
 * @brief This macro defines a class with an implementation.
 * 
 * @param _cls The class.
 * 
 * @code
 * MR_IMPL_CLASS(String);
 * @endcode
 */
#define MR_IMPL_CLASS(_cls)                                                    \
    struct mr_class _cls = {#_cls, (void *)__init__, (void *)__del__};

/**
 * @brief This macro function initializes an instance of a class.
 * 
 * @param cls The class.
 * @param self The instance.
 * @param ... The arguments.
 * 
 * @return A pointer to the instance.
 * 
 * @code
 * struct String string;
 * MR_INIT(String, &string, "hello");
 * @endcode
 */
#define MR_INIT(_cls, _self, ...)       (_cls.__init__(_self, ##__VA_ARGS__))

/**
 * @brief This macro function uninitializes an instance of a class.
 * 
 * @param _cls The class.
 * @param _self The instance.
 * 
 * @return A pointer to the instance.
 * 
 * @code
 * struct String string;
 * MR_INIT(String, &string, "hello");
 * MR_UNINIT(String, &string);
 * @endcode
 */
#define MR_UNINIT(_cls, _self)          (_cls.__del__(_self))     

/**
 * @brief This macro function creates a new instance of a class.
 * 
 * @param _cls The class.
 * @param ... The arguments.
 * 
 * @return A pointer to the new instance.
 * 
 * @code
 * struct String *string = MR_NEW(String, "hello");
 * @endcode
 */
#define MR_NEW(_cls, ...)                                                       \
    (_cls.__init__(mr_malloc(sizeof(_cls)), ##__VA_ARGS__))

/**
 * @brief This macro function deletes an instance of a class.
 * 
 * @param cls The class.
 * @param self The instance.
 */
#define MR_DEL(_cls, _self)        (mr_free(_cls.__del__(_self)), _self = NULL)

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_CLASS_H__ */
