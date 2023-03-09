//
// Created by 10632 on 2023/3/8.
//

#ifndef DESIGN_MR_LIBRARY_INCLUDE_MRSERVE_H_
#define DESIGN_MR_LIBRARY_INCLUDE_MRSERVE_H_

#include <mrdef.h>

#define mr_hw_interrupt_enable() \
do{                              \
}while(0)

#define mr_hw_interrupt_disable()\
do{                              \
}while(0)

#define mr_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

MR_INLINE void mr_list_init(mr_list_t list)
{
    list->next = list;
    list->prev = list;
}

MR_INLINE void mr_list_insert_after(mr_list_t list, mr_list_t node)
{
    list->next->prev = node;
    node->next = list->next;

    list->next = node;
    node->prev = list;
}

MR_INLINE void mr_list_remove(mr_list_t node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;

    node->next = node->prev = node;
}

#endif //DESIGN_MR_LIBRARY_INCLUDE_MRSERVE_H_
