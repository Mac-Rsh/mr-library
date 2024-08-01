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
    void *intf;
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
 * @brief This macro defines a function of a class.
 * 
 * @param _cls The class.
 * 
 * @code 
 * MR_CLASS(String)
 * {
 *     MR_FUNCTION(String)
 *     {
 *         void (*print)(struct String *self);
 *     } *fn;
 *     const char *_str;
 * };
 * @endcode
 */
#define MR_FUNCTION(_cls)               struct _cls##fn

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
 *         void (*output)(const char *str);
 *     } *_if;
 *     const char *_str;
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
 *     MR_ASSERT(self != NULL);
 * 
 *     self->_str = str;
 *     return self;
 * }
 * @endcode
 */
#define MR_IMPL(_cls, _fn, _ret, ...)                                          \
    static _ret _fn(struct _cls *self, ##__VA_ARGS__)

/**
 * @brief This macro defines an implementation of a function of a class.
 * 
 * @param _cls The class.
 * @param ... The function.
 * 
 * @code
 * MR_IMPL(String, __init__, const char *str)
 * {
 *     MR_IMPL_FUNCTION(String, print);
 * 
 *     MR_ASSERT(self != NULL);
 * 
 *     self->fn = &fn;
 *     self->_str = str;
 * }
 * @endcode
 */
#define MR_IMPL_FUNCTION(_cls, ...)                                            \
    static struct _cls##fn fn = {__VA_ARGS__}

/**
 * @brief This macro defines an implementation of an interface of a class.
 * 
 * @param _cls The class.
 * @param ... The interface.
 * 
 * @code
 * MR_IMPL(String, __init__, void *, const char *str)
 * {
 *     MR_ASSERT(self != NULL);
 * 
 *     self->_if = String.intf;
 *     self->_str = str;
 *     return self;
 * }
 * 
 * MR_IMPL_INTERFACE(String, output);
 * 
 * int main(void)
 * {
 *     String.intf = &intf;
 *     struct String *string = MR_NEW(String, "hello");
 *     string->_if->output(string->str);
 * }
 * @endcode
 */
#define MR_IMPL_INTERFACE(_cls, ...)                                           \
    static struct _cls##intf intf = {__VA_ARGS__}

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
    struct mr_class _cls = {#_cls, NULL, (void *)__init__, (void *)__del__};

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
#define MR_DEL(_cls, _self)                                                    \
    (mr_free(_cls.__del__(_self)), _self = NULL)

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MR_CLASS_H__ */
