# etask使用指南

事件任务框架是是一种用于处理异步事件和任务的编程模型。它提供了一种结构化的方式来管理和调度事件和任务，以便在单线程、多线程或多任务环境中实现高效的并发处理。

使用事件任务框架，可以将复杂的并发处理逻辑进行抽象和管理，提高代码的可维护性和可扩展性。它使得开发人员能够更加专注于事件的处理逻辑，而无需过多关注底层的并发细节。

事件任务框架支持动态添加和移除事件任务的能力。当系统需要新增功能或修改业务逻辑时，可以通过添加或替换相应的事件任务来实现，而无需修改其他部分的代码。这种可插拔性使得系统更加灵活，能够快速响应变化的需求。

----------

## 准备

1. 在 `mrconfig.h` 中 `Module config` 下添加宏开关启用etask组件。

```c
//<------------------------------------ Module config ------------------------------------>

#define MR_CFG_ETASK                    MR_CFG_ENABLE
```

2. 在 `mrlib.h` 中引用头文件.

```c
#include "etask.h"
```

----------

## 查找事件任务

```c
mr_etask_t mr_etask_find(const char *name);
```

| 参数      | 描述    |
|:--------|:------|
| name    | 事件任务名 |
| **返回**  |       |
| 事件任务    | 查找成功  |
| MR_NULL | 查找失败  |

----------

## 添加事件任务

```c
mr_err_t mr_etask_add(mr_etask_t etask, const char *name, mr_size_t size);
```

| 参数        | 描述     |
|:----------|:-------|
| etask     | 事件任务   |
| name      | 事件任务名  |
| size      | 缓存队列大小 |
| **返回**    |        |
| MR_ERR_OK | 添加成功   |
| 错误码       | 添加失败   |

缓冲队列将对发生的事件进行缓存，当出现事件丢失，请增加缓冲队列大小或提高事件循环处理频率。

缓冲队列将通过 `mr_malloc` 动态申请内存，请不要频繁添加移除事件任务。

----------

## 移除事件任务

```c
mr_err_t mr_etask_remove(mr_etask_t etask);
```

| 参数        | 描述   |
|:----------|:-----|
| etask     | 事件任务 |
| **返回**    |      |
| MR_ERR_OK | 移除成功 |
| 错误码       | 移除失败 |

----------

## 更新事件任务时钟

```c
void mr_etask_tick_update(mr_etask_t etask);
```

| 参数    | 描述   |
|:------|:-----|
| etask | 事件任务 |

----------

## 处理事件任务

```c
void mr_etask_handle(mr_etask_t etask);
```

| 参数    | 描述   |
|:------|:-----|
| etask | 事件任务 |

按事件发生顺序处理（FIFO），仅会处理进入处理函数前已发生的事件，防止单一事件循环触发阻塞系统。

状态事件将在每次事件任务处理结束后运行。

----------

## 启动事件

```c
mr_err_t mr_etask_start(mr_etask_t etask,
                        mr_uint32_t id,
                        mr_uint8_t sflags,
                        mr_uint32_t time,
                        mr_err_t (*cb)(mr_etask_t et, void *args),
                        void *args);
```

| 参数        | 描述     |
|:----------|:-------|
| etask     | 事件任务   |
| id        | 事件ID   |
| sflags    | 启动标志   |
| time      | 定时时间   |
| cb        | 事件回调函数 |
| args      | 回调函数参数 |
| **返回**    |        |
| MR_ERR_OK | 启动成功   |
| 错误码       | 启动失败   |

- sflags: 事件分为普通事件与定时事件。定时事件可分为单次/周期和软件/硬件，单次定时事件在发生完成后将释放自身，周期定时事件在发生完成后将自动重启直至用户停止该事件。
  软件定时事件在超时发生时将事件加入事件队列中等待处理，而硬件定时器将在任务时钟更新处直接处理。

```c
MR_ETASK_SFLAG_EVENT                                                /* 普通事件 */
MR_ETASK_SFLAG_TIMER                                                /* 定时事件（周期、软件） */
MR_ETASK_SFLAG_TIMER | MR_ETASK_SFLAG_HARD                          /* 定时事件（周期、硬件） */
MR_ETASK_SFLAG_TIMER | MR_ETASK_SFLAG_ONESHOT                       /* 定时事件（单次、软件） */
MR_ETASK_SFLAG_TIMER | MR_ETASK_SFLAG_ONESHOT | MR_ETASK_SFLAG_HARD /* 定时事件（单次、硬件） */
```

当与RTOS线程组合使用时可实现真正意义上的高效并发，并且可根据线程优先级控制事件的响应速度。
但是也会引入新的问题，当事件回调中使用任何能使线程挂起的功能时，可能会导致其余所有事件无法响应（等同于所有事件被挂起等待），因此事件处理函数中不因使用类似功能（事件回调应能直接处理无需等待）。

事件回调函数禁止阻塞（while死循环等）。一旦有事件阻塞，该etask将无法正常响应其余事件。

----------

## 停止事件

```c
mr_err_t mr_etask_stop(mr_etask_t etask, mr_uint32_t id);
```

| 参数        | 描述   |
|:----------|:-----|
| etask     | 事件任务 |
| id        | 事件ID |
| **返回**    |      |
| MR_ERR_OK | 停止成功 |
| 错误码       | 停止失败 |

----------

## 唤醒事件

```c
mr_err_t mr_etask_wakeup(mr_etask_t etask, mr_uint32_t id, mr_uint8_t wflag);
```

| 参数        | 描述   |
|:----------|:-----|
| etask     | 事件任务 |
| id        | 事件ID |
| wflag     | 唤醒标志 |
| **返回**    |      |
| MR_ERR_OK | 通知成功 |
| 错误码       | 通知失败 |

- wflag：事件唤醒标志可分为立即唤醒、延迟唤醒和状态唤醒，立即唤醒将在调用处直接处理，延迟唤醒将事件加入事件队列中等待处理，状态唤醒将事件转换为etask状态事件。

```c
MR_ETASK_WFLAG_NOW                                                  /* 立即唤醒 */
MR_ETASK_WFLAG_DELAY                                                /* 延迟唤醒 */
MR_ETASK_WFLAG_STATE                                                /* 状态唤醒 */
```

状态事件将在每次事件任务处理结束后运行（有且仅有一个状态事件，默认无状态事件）。

当唤醒事件发生失败（错误码 -5）时，请增加缓冲队列大小或提高事件处理频率（缓冲队列已满，无法及时响应事件，导致事件丢失）。

----------

## 字符串转事件ID

```c
mr_uint32_t mr_etask_str2id(const char *string);
```

| 参数     | 描述  |
|:-------|:----|
| string | 字符串 |
| **返回** |     |
| 事件ID   |     |

当事件数量过多且字符串内容相近时可能会出现哈希碰撞。

----------

使用示例：

```c
/* 定义事件 */
#define EVENT_1                         1234
#define EVENT_2                         "event_3"
#define EVENT_3                         3456

/* 定义事件回调 */
mr_err_t event1_cb(mr_etask_t etask, void *args)
{
    printf("event1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event2_cb(mr_etask_t etask, void *args)
{
    printf("event2_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event3_cb(mr_etask_t etask, void *args)
{
    printf("event3_cb\r\n");
    return MR_ERR_OK;
}

/* 定义事件任务 */
struct mr_etask etask;

int main(void)
{
    /* 添加事件任务 */
    mr_etask_add(&etask, "etask", 3);
    
    /* 启动普通事件 */
    mr_etask_start(&etask, EVENT_1, MR_ETASK_SFLAG_EVENT, 0, event1_cb, NULL);
    mr_etask_start(&etask, mr_etask_str2id(EVENT_2), MR_ETASK_SFLAG_EVENT, 0, event2_cb, NULL);
    
    /* 启动定时事件 */
    mr_etask_start(&etask, EVENT_3, MR_ETASK_SFLAG_TIMER | MR_ETASK_SFLAG_HARD, 5, event3_cb, NULL);
    
    /* 延迟唤醒事件1 */
    mr_etask_wakeup(&etask, EVENT_1, MR_ETASK_WFLAG_DELAY);
    /* 立即唤醒事件2 */
    mr_etask_wakeup(&etask, mr_etask_str2id(EVENT_2), MR_ETASK_WFLAG_NOW);
    /* 状态唤醒事件2 */
    mr_etask_wakeup(&etask, mr_etask_str2id(EVENT_2), MR_ETASK_WFLAG_STATE);

    while (1)
    {
        /* 更新事件任务时钟 */
        mr_etask_tick_update(&etask);
        /* 事件任务处理 */
        mr_etask_handle(&etask);
    }
}
```