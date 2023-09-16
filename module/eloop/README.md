# 事件循环使用指南

事件是一种异步处理机制，通过事件分发和回调的方式，可以显著提升系统的异步处理能力、解耦性和可扩展性。

在事件驱动的模型中，当事件发生时，它们被放置在事件队列中等待处理。事件队列充当一个缓冲区，它存储了即将被处理的事件。这种分离事件产生和事件处理的方式使得系统能够更加高效地处理并发事件。

通过周期性地从事件队列中获取事件，并将其分派给相应的事件处理器（回调函数），系统异步执行相关任务。这种异步处理方式允许系统在等待某些资源的同时继续处理其他事件，提高了系统的响应能力和整体性能。

使用事件驱动的架构，不同模块之间的耦合度降低。模块之间通过事件进行通信，而不是直接依赖于彼此。这种解耦性使得系统更加灵活，模块之间可以独立地进行开发、测试和维护，同时也方便了新模块的加入和旧模块的替换。

![流程图](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/eloop.jpg)

----------

## 准备

1. 在 `mrconfig.h` 中 `Module config` 下添加宏开关启用Eloop组件。

```c
//<------------------------------------ Module config ------------------------------------>

#define MR_CFG_ELOOP                    MR_CFG_ENABLE
```

2. 在 `mrlib.h` 中引用头文件.

```c
#include "eloop.h"
```

----------

## 查找事件循环

```c
mr_eloop_t mr_eloop_find(const char *name);
```

| 参数      | 描述    |
|:--------|:------|
| name    | 事件循环名 |
| **返回**  |       |
| 事件循环名   | 查找成功  |
| MR_NULL | 查找失败  |

----------

## 添加事件循环

```c
mr_err_t mr_eloop_add(mr_eloop_t eloop, const char *name, mr_size_t queue_size);
```

| 参数         | 描述     |
|:-----------|:-------|
| eloop      | 事件循环   |
| name       | 事件循环名  |
| queue_size | 缓存队列大小 |
| **返回**     |        |
| MR_ERR_OK  | 添加成功   |
| 错误码        | 添加失败   |

缓冲队列将对发生的事件进行缓存，当出现事件丢失，请增加缓冲队列大小或提高事件循环处理频率。

缓冲队列将通过 `mr_malloc` 动态申请内存，请不要频繁添加移除事件循环。

----------

## 移除事件循环

```c
mr_err_t mr_eloop_remove(mr_eloop_t eloop);
```

| 参数         | 描述     |
|:-----------|:-------|
| eloop      | 事件循环   |
| **返回**     |        |
| MR_ERR_OK  | 移除成功   |
| 错误码        | 移除失败   |

----------

## 处理事件

```c
void mr_eloop_handle(mr_eloop_t eloop);
```

| 参数         | 描述     |
|:-----------|:-------|
| eloop      | 事件循环   |

按事件发生顺序处理（FIFO），仅会处理进入处理函数前已发生的事件，防止单一事件循环触发阻塞系统。

----------

## 创建事件

```c
mr_err_t mr_eloop_create_event(mr_eloop_t eloop, mr_uint32_t id, mr_err_t (*cb)(mr_eloop_t ep, void *args), void *args);
```

| 参数           | 描述       |
|:-------------|:---------|
| eloop        | 事件循环     |
| id           | 事件ID     |
| cb           | 事件回调函数   |
| args         | 回调函数参数   |
| **返回**       |          |
| MR_ERR_OK    | 创建成功     |
| 错误码          | 创建失败     |

当与RTOS线程组合使用时可实现真正意义上的高效并发，并且可根据线程优先级控制事件的响应速度。
但是也会引入新的问题，当事件回调中使用任何能使线程挂起的功能时，可能会导致其余所有事件无法响应（等同于所有事件被挂起等待），因此事件处理函数中不因使用类似功能（事件回调应能直接处理无需等待）。

事件回调函数禁止阻塞（while死循环等）。一旦有事件阻塞，该eloop无法正常响应其余事件。

----------

## 删除事件

```c
mr_err_t mr_eloop_delete_event(mr_eloop_t eloop, mr_uint32_t id);
```

| 参数           | 描述       |
|:-------------|:---------|
| eloop        | 事件循环     |
| id           | 事件ID     |
| **返回**       |          |
| MR_ERR_OK    | 删除成功     |
| 错误码          | 删除失败     |

----------

## 通知事件发生

```c
mr_err_t mr_eloop_notify_event(mr_eloop_t eloop, mr_uint32_t id);
```

| 参数           | 描述       |
|:-------------|:---------|
| eloop        | 事件循环     |
| id           | 事件ID     |
| **返回**       |          |
| MR_ERR_OK    | 通知成功     |
| 错误码          | 通知失败     |

通知事件为异步操作，实际处理将在事件处理中进行。
当通知事件发生失败（错误码 -5）时，请增加缓冲队列大小或提高事件处理频率（缓冲队列已满，无法及时响应事件，导致事件丢失）。

----------

## 触发事件

```c
mr_err_t mr_eloop_trigger_event(mr_eloop_t eloop, mr_uint32_t id);
```

| 参数           | 描述       |
|:-------------|:---------|
| eloop        | 事件循环     |
| id           | 事件ID     |
| **返回**       |          |
| MR_ERR_OK    | 触发成功     |
| 错误码          | 触发失败     |

此操作为同步操作，实际处理将在触发处立即执行。

----------

## 字符串转事件ID

```c
mr_uint32_t mr_eloop_string_to_id(const char *string, mr_size_t size);
```

| 参数     | 描述    |
|:-------|:------|
| string | 字符串   |
| size   | 字符串长度 |
| **返回** |       |
| 事件ID   |       |

当事件数量过多且字符串内容相近时可能会出现哈希碰撞。

----------

使用示例：

```c
/* 定义事件 */
#define EVENT_1                         1234
#define EVENT_2                         3456
#define EVENT_3                         "event_3"

/* 定义事件回调 */
mr_err_t event1_cb(mr_eloop_t eloop, void *args)
{
    printf("event1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event2_cb(mr_eloop_t eloop, void *args)
{
    printf("event2_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event3_cb(mr_eloop_t eloop, void *args)
{
    printf("event3_cb\r\n");
    return MR_ERR_OK;
}

/* 定义事件循环 */
struct mr_eloop eloop;

int main(void)
{
    /* 添加事件循环 */
    mr_eloop_add(&event, "eloop", 3);
    
    /* 向事件循环创建事件 */
    mr_eloop_create_event(&eloop, EVENT_1, event1_cb, NULL);
    mr_eloop_create_event(&eloop, EVENT_2, event2_cb, NULL);
    mr_eloop_create_event(&eloop, mr_eloop_string_to_id(EVENT_3, strlen(EVENT_3)), event3_cb, NULL);

    /* 通知事件1发生 */
    mr_eloop_notify_event(&event, Event_1);
    /* 通知事件2发生 */
    mr_eloop_notify_event(&event, Event_2);
    /* 触发事件3 */
    mr_eloop_trigger_event(&event, mr_eloop_string_to_id(EVENT_3, strlen(EVENT_3)));
    
    while (1)
    {
        /* 事件处理 */
        mr_eloop_handle(&event);
    }
}
```

现象：

```c
event3_cb
event1_cb
event2_cb
```