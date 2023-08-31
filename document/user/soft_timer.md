# 软件定时器使用文档

## 应用场景

软件定时器适用于和时间相关的操作，主要应用场景包括:

- 定时周期和定时单次任务。
- 超时检测。

## 接口介绍

### 查找定时器

```c
mr_soft_timer_t mr_soft_timer_find(const char *name);
```

| 参数      | 描述   |
|:--------|:-----|
| name    | 定时器名 |
| **返回**  |      |
| 定时器句柄   | 查找成功 |
| MR_NULL | 查找失败 |

### 添加定时器

```c
mr_err_t mr_soft_timer_add(mr_soft_timer_t timer, const char *name, mr_soft_timer_table_t table, mr_size_t table_size);
```

| 参数         | 描述      |
|:-----------|:--------|
| timer      | 定时器句柄   |
| name       | 定时器名    |
| table      | 定时器列表   |
| table_size | 定时器列表个数 |
| **返回**     |         |
| MR_ERR_OK  | 添加成功    |
| 错误码        | 添加失败    |

定时器信息占用12Byte，添加定时器时会自动分配对应定时器列表长度的信息以满足定时器运行需求（malloc分配内存总和: table_size * 12Byte，请用户注意内存消耗，请勿频繁添加、移除）。

### 移除定时器

```c
mr_err_t mr_soft_timer_remove(mr_soft_timer_t timer);
```

| 参数        | 描述    |
|:----------|:------|
| timer     | 定时器句柄 |
| **返回**    |       |
| MR_ERR_OK | 移除成功  |
| 错误码       | 移除失败  |

添加定时器时所分配的内存将在此释放（移除后请勿再次操作该定时器，如需再次使用请重新添加）。

### 定时器时间更新

```c
void mr_soft_timer_update(mr_soft_timer_t timer, mr_uint32_t time);
```

| 参数      | 描述                |
|:--------|:------------------|
| timer   | 定时器句柄             |
| time    | 更新的时间（与上次更新的时间间隔） |

定时器时间更新需要有相对稳定的时基信号（tick、硬件定时间、RTOS系统节拍、RTOS软件定时器、while循环），time不能等于0。

### 定时器超时处理

```c
void mr_soft_timer_handle(mr_soft_timer_t timer);
```

| 参数      | 描述                |
|:--------|:------------------|
| timer   | 定时器句柄             |

仅处理调用该函数前已超时的定时器，对于定时器处理过程中超时的定时器将在下次运行时处理（防止定时器循环占用）。

### 启动定时器

```c
mr_err_t mr_soft_timer_start(mr_soft_timer_t timer, mr_uint32_t index);
```

| 参数        | 描述    |
|:----------|:------|
| timer     | 定时器句柄 |
| index     | 定时器索引 |
| **返回**    |       |
| MR_ERR_OK | 启动成功  |
| 错误码       | 启动失败  |

如果定时器已经启动，再次启动将不会生效。

### 停止定时器

```c
mr_err_t mr_soft_timer_stop(mr_soft_timer_t timer, mr_uint32_t index);
```

| 参数        | 描述    |
|:----------|:------|
| timer     | 定时器句柄 |
| index     | 定时器索引 |
| **返回**    |       |
| MR_ERR_OK | 停止成功  |
| 错误码       | 停止失败  |

当定时器被停止后将无法从原来的时间恢复，只能按设定的间隔重新启动。

### 重启定时器

```c
mr_err_t mr_soft_timer_restart(mr_soft_timer_t timer, mr_uint32_t index);
```

| 参数        | 描述    |
|:----------|:------|
| timer     | 定时器句柄 |
| index     | 定时器索引 |
| **返回**    |       |
| MR_ERR_OK | 重启成功  |
| 错误码       | 重启失败  |

### 设置定时器时间间隔

```c
mr_err_t mr_soft_timer_set_time(mr_soft_timer_t timer, mr_uint32_t index, mr_uint32_t time);
```

| 参数        | 描述    |
|:----------|:------|
| timer     | 定时器句柄 |
| index     | 定时器索引 |
| time      | 时间间隔  |
| **返回**    |       |
| MR_ERR_OK | 设置成功  |
| 错误码       | 设置失败  |

如果定时器已经启动，设置的新间隔将在下次启动时生效。

### 获取定时器当前时间

```c
void mr_soft_timer_get_current_time(mr_soft_timer_t timer);
```

| 参数          | 描述      |
|:------------|:--------|
| timer       | 定时器句柄   |
| **返回**      |         |
| mr_uint32_t | 定时器当前时间 |

## 定时器列表介绍

```c
struct mr_soft_timer_table
```

| 参数       | 描述           |
|:---------|:-------------|
| time     | 定时器超时时间      |
| type     | 定时器类型（周期/单次） |
| cb       | 定时器超时回调      |
| args     | 定时器超时回调参数    |

## 使用示例

```c
/* 定义定时器索引 */
enum timer_index
{
    Timer_1 = 0,
    Timer_2,
    Timer_3,
};

/* 定义定时器超时回调 */
mr_err_t timer1_cb(mr_soft_timer_t timer, void *args)
{
    printf("timer1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t timer2_cb(mr_soft_timer_t timer, void *args)
{
    printf("timer2_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t timer3_cb(mr_soft_timer_t timer, void *args)
{
    printf("timer3_cb\r\n");
    
    /* 暂停定时器3 */
    mr_soft_timer_stop(timer, Timer_3);
    return MR_ERR_OK;
}

/* 定义定时器列表 */
struct mr_soft_timer_table timer_table[] =
{
    {5, Mr_Soft_Timer_Type_Period, timer1_cb, MR_NULL},
    {10, Mr_Soft_Timer_Type_Period, timer2_cb, MR_NULL},
    {15, Mr_Soft_Timer_Type_Oneshot, timer3_cb, MR_NULL},
};

/* 定义定时器 */
struct mr_soft_timer timer;

int main(void)
{
    /* 添加定时器 */
    mr_soft_timer_add(&timer, "timer", timer_table, MR_ARRAY_SIZE(timer_table));
    
    /* 修改定时器1时间间隔*/
    mr_soft_timer_set_time(&timer, Timer_1, 2);
    
    /* 启动定时器 */
    mr_soft_timer_start(&timer, Timer_1);
    mr_soft_timer_start(&timer, Timer_2);
    mr_soft_timer_start(&timer, Timer_3);

    while (1)
    {
        /* 更新时间 */
        mr_soft_timer_update(&timer, 1);
        
        /* 定时器超时处理 */
        mr_soft_timer_handle(&timer);
    }
}
```