# 软件定时器文档

软件定时器是一种在软件层面实现计时功能的机制，通过软件定时器，可以在特定时间点或时间间隔触发特定的事件。软件定时器常用于实现周期性任务、超时处理、定时器中断等功能。

软件定时器包含两个主要组件:定时服务器和定时器。

- 定时服务器用于时间管理和定时器处理。
- 定时器用于处理特定的超时处理,它需要注册到定时服务器并提供一个回调函数。

## 软件定时服务原型

```c
/* 定时服务器 */
struct soft_timer_server
{
    uint32_t time;                                                  /* 当前时间 */
    struct soft_timer_list list;                                    /* 定时器链表 */
};
```

- time:当前时间(从定时器启动到现在过去的时间)。
- list:定时器链表(用于存储定时器)。

```c
struct soft_timer
{
    struct soft_timer_list list;                                    /* 定时器链表 */
    soft_timer_server_t server;                                     /* 所属的服务器 */
    uint32_t interval;                                              /* 定时间隔时间 */
    uint32_t timeout;                                               /* 下次超时时间 */

    int (*cb)(soft_timer_t timer, void *args);                      /* 超时回调函数 */
    void *args;                                                     /* 超时回调函数参数 */        
};
```

- list:定时器链表(用于挂载到服务器)。
- server:所属的服务器。
- interval:定时间隔时间(初始化时设置)。
- timeout:下次超时时间。
- cb：回调函数。
- args：回调函数参数。

### 初始化服务器

```c
void soft_timer_server_init(soft_timer_server_t server);
```

| 参数           | 描述                 |
|:-------------|:-------------------|
| server       | 服务器句柄              |

### 更新服务器时间

```c
void soft_timer_server_update(soft_timer_server_t server, uint32_t time);
```

| 参数     | 描述      |
|:-------|:--------|
| server | 服务器句柄   |
| time   | 更新过去的时间 |

### 服务器检查超时定时器，唤醒定时器

```c
void soft_timer_server_handle(soft_timer_server_t server);
```

| 参数     | 描述    |
|:-------|:------|
| server | 服务器句柄 |

### 添加新定时器

```c
int soft_timer_add(soft_timer_t timer,
                   uint32_t time,
                   int (*cb)(soft_timer_t timer, void *args),
                   void *args,
                   soft_timer_server_t server);
```

| 参数                | 描述      |
|:------------------|:--------|
| timer             | 定时器句柄   |
| time              | 定时器超时时间 |
| cb                | 回调函数    |
| args              | 回调函数参数  |
| server            | 服务器句柄   |
| **返回**            |         |
| SOFT_TIMER_ERR_OK | 添加成功    |
| 错误码               | 添加失败    |

### 移除定时器

```c
int soft_timer_remove(soft_timer_t timer);
```

| 参数                | 描述      |
|:------------------|:--------|
| timer             | 定时器句柄   |
| **返回**            |         |
| SOFT_TIMER_ERR_OK | 移除成功    |
| 错误码               | 移除失败    |

### 启动定时器

```c
int soft_timer_start(soft_timer_t timer);
```

| 参数                | 描述      |
|:------------------|:--------|
| timer             | 定时器句柄   |
| **返回**            |         |
| SOFT_TIMER_ERR_OK | 启动成功    |
| 错误码               | 启动失败    |

### 暂停定时器

```c
int soft_timer_stop(soft_timer_t timer);
```

| 参数                | 描述      |
|:------------------|:--------|
| timer             | 定时器句柄   |
| **返回**            |         |
| SOFT_TIMER_ERR_OK | 暂停成功    |
| 错误码               | 暂停失败    |

### 添加并启动新定时器

```c
int soft_timer_add_then_start(soft_timer_t timer,
                              uint32_t time,
                              int (*cb)(soft_timer_t client, void *args),
                              void *args,
                              soft_timer_server_t server);
```

| 参数                | 描述      |
|:------------------|:--------|
| timer             | 定时器句柄   |
| time              | 定时器超时时间 |
| cb                | 回调函数    |
| args              | 回调函数参数  |
| server            | 服务器句柄   |
| **返回**            |         |
| SOFT_TIMER_ERR_OK | 添加成功    |
| 错误码               | 添加失败    |

## 使用示例：

```c
/* 定义定时器服务器和定时器 */
struct soft_timer_server server;
struct soft_timer timer1, timer2, timer3;

int timer1_callback(soft_timer_t timer, void *args)
{
    printf("timer1_callback\r\n");
    return SOFT_TIMER_ERR_OK;
}

int timer2_callback(soft_timer_t timer, void *args)
{
    printf("timer2_callback\r\n");
    return SOFT_TIMER_ERR_OK;
}

int timer3_callback(soft_timer_t timer, void *args)
{
    printf("timer3_callback\r\n");
    soft_timer_stop(timer);
    return SOFT_TIMER_ERR_OK;
}

int main(void)
{
    /* 初始化服务器 */
    soft_timer_server_init(&server);
    
    /* 添加定时器到服务器并启动 */
    soft_timer_add_then_start(&timer1, 5, timer1_callback, NULL, &server);
    soft_timer_add_then_start(&timer2, 10, timer2_callback, NULL, &server);
    soft_timer_add_then_start(&timer3, 15, timer3_callback, NULL, &server);

    while (1)
    {
        /* 更新服务器时钟 */
        soft_timer_server_update(&server, 1);
        
        /* 服务器处理定时器超时（放在哪里，回调就将在哪里被调用） */
        soft_timer_server_handle(&server);
    }
}
```