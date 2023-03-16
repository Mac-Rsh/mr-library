//
// Created by 10632 on 2023/3/13.
//

#ifndef _MR_LOG_H_
#define _MR_LOG_H_

#include <mrdef.h>

MR_INLINE void mr_assert_handler()
{

}

#define MR_ASSERT(EX)           \
    do{                         \
        if (!(EX))              \
        {                       \
          printf("ASSERT: [%s]- [%d]\r\n", __FUNCTION__ , __LINE__);                      \
          mr_assert_handler();  \
        }                       \
    }while(0)


//#define MR_LOG_D(string)  \
//    printf("[ DEBUG ] [ %s ][ %d ][ %s ] { %s }\r\n", \
//    __FILE__,                 \
//    __LINE__,                 \
//    __FUNCTION__,              \
//    string                      \
//    )
//
//#define MR_LOG_W(string)    \
//    printf("[ WARING ] [ %s ][ %d ][ %s ] { %s }\r\n", \
//    __FILE__,                 \
//    __LINE__,                 \
//    __FUNCTION__,             \
//    string                      \
//    )
//
#define MR_LOG_E(string) \
    do{                  \
           printf(string);              \
                         \
    }while(0)

#define MR_LOG_D(string) do{}while(0)
//#define MR_LOG_E(string) do{}while(0)
#define MR_LOG_W(string) do{}while(0)

#endif
