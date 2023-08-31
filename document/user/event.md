# 事件使用文档

## 应用场景

事件服务适用于需要异步处理多个事件的场景,主要应用场景包括:

- RTOS操作系统的异步事件处理:
  - 线程和中断异步执行机制（将中断处理程序从中断环境转移到线程环境）。
  - 线程间同步（A线程数据准备完成后，通知B线程处理。不推荐使用，尽量使用RTOS自带的同步机制）。
  - 同级任务非实时性任务处理（同一级别事件处理交由同一线程管理，节约线程资源开支）。
- 裸机任务拆分、前后台同步（推荐使用）。
- RTOS和裸机间代码快速移植。

## 接口介绍

### 查找事件

```c
mr_event_t mr_event_find(const char *name);
```

| 参数      | 描述   |
|:--------|:-----|
| name    | 事件名  |
| **返回**  |      |
| 事件句柄    | 查找成功 |
| MR_NULL | 查找失败 |

### 添加事件

```c
mr_err_t mr_event_add(mr_event_t event, const char *name, mr_event_table_t table, mr_size_t table_size);
```

| 参数         | 描述     |
|:-----------|:-------|
| event      | 事件句柄   |
| name       | 事件名    |
| table      | 事件列表   |
| table_size | 事件列表个数 |
| **返回**     |        |
| MR_ERR_OK  | 添加成功   |
| 错误码        | 添加失败   |

事件ID占用4Byte，添加事件时会自动分配对应事件列表长度的队列以满足所有事件同时异步操作（malloc分配内存总和: table_size * 4Byte，请用户注意内存消耗，请勿频繁添加、移除）。

### 移除事件

```c
mr_err_t mr_event_remove(mr_event_t event);
```

| 参数        | 描述   |
|:----------|:-----|
| event     | 事件句柄 |
| **返回**    |      |
| MR_ERR_OK | 移除成功 |
| 错误码       | 移除失败 |

添加事件时所分配的内存将在此释放（移除后请勿再次操作该事件，如需再次使用请重新添加）。

### 发生事件处理

```c
void mr_event_handle(mr_event_t event);
```

| 参数     | 描述     |
|:-------|:-------|
| event  | 事件句柄   |

仅处理调用该函数前已经发生的事件，对于处理过程中发生的事件将在下次运行时处理（防止事件循环占用），事件处理顺序按发生顺序处理（FIFO）。

### 通知事件发生

```c
mr_err_t mr_event_notify(mr_event_t event, mr_uint32_t index);
```

| 参数        | 描述   |
|:----------|:-----|
| event     | 事件句柄 |
| index     | 事件索引 |
| **返回**    |      |
| MR_ERR_OK | 通知成功 |
| 错误码       | 通知失败 |

当通知事件发生失败（错误码 -5）时，请提高事件处理频率（事件队列溢出，无法及时响应事件，导致事件丢失）。

此操作为异步操作，实际处理将在事件处理中进行。

### 事件触发

```c
mr_err_t mr_event_trigger(mr_event_t event, mr_uint32_t index);
```

| 参数        | 描述   |
|:----------|:-----|
| event     | 事件句柄 |
| index     | 事件索引 |
| **返回**    |      |
| MR_ERR_OK | 触发成功 |
| 错误码       | 触发失败 |

此操作为同步操作，实际处理将在触发处立即执行。

## 事件列表介绍

```c
struct mr_event_table
```

| 参数       | 描述       |
|:---------|:---------|
| cb       | 事件发生回调   |
| args     | 事件发生回调参数 |

## 使用示例

```c
/* 定义事件索引 */
enum event_index
{
    Event_1 = 0,
    Event_2,
    Event_3,
};

/* 定义事件回调 */
mr_err_t event1_cb(mr_event_t event, void *args)
{
    printf("event1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event2_cb(mr_event_t event, void *args)
{
    printf("event2_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t event3_cb(mr_event_t event, void *args)
{
    printf("event3_cb\r\n");
    return MR_ERR_OK;
}

/* 定义事件列表 */
struct mr_event_table event_table[] =
{
    {event1_cb, MR_NULL},
    {event2_cb, MR_NULL},
    {event3_cb, MR_NULL},
};

/* 定义事件 */
struct mr_event event;

int main(void)
{
    /* 添加事件 */
    mr_event_add(&event, "event", event_table, MR_ARRAY_SIZE(event_table));
    
    /* 通知事件1 */
    mr_event_notify(&event, Event_1);
    /* 通知事件2 */
    mr_event_notify(&event, Event_2);
    /* 触发事件3 */
    mr_event_trigger(&event, Event_3);

    while (1)
    {
        /* 发生事件处理 */
        mr_event_handle(&event);
    }
}
```
现象：

```c
event3_cb
event1_cb
event2_cb
```