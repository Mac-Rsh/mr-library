# 事件服务文档

事件服务器是一种异步事件处理机制,它通过事件分发和回调的方式,可以有效地提高系统的异步处理能力、解耦性和可扩展性。

事件服务器包含两个主要组件:事件服务器和事件客户端。
- 事件服务器用于接收和分发事件,它内部维护一个事件队列用于存储待处理事件和一个事件列表用于存储注册的事件客户端。
- 事件客户端用于处理特定类型的事件,它需要注册到事件服务器并提供一个回调函数。

当事件发生时,事件服务器会将事件插入到其事件队列中进行缓存。事件服务器会周期性地从事件队列中取出事件进行分发,找到对应的事件客户端,然后调用其注册的回调函数进行事件处理。

## 事件服务原型

```c
/* 事件服务器 */
struct mr_event_server
{
  struct mr_object object;                                        /* 事件服务对象 */
  
  struct mr_fifo queue;                                           /* 事件队列 */
  mr_avl_t list;                                                  /* 事件链表 */
};
```

- object:对象(用于将事件服务器添加到容器中)。
- queue:事件队列(用于存储要唤醒的客户端ID)。
- list:服务器链表(用于存储客户端)。

```
/* 事件客户端 */
struct mr_event_client
{
  struct mr_avl list;                                             /* 事件链表 */
  
  mr_err_t (*cb)(mr_event_server_t server, void *args);           /* 事件回调函数 */
  void *args;                                                     /* 事件回调函数参数 */
};
```

- list:客户端链表(用于将客户端添加到服务器的客户端列表中)。
- cb：回调函数。
- args：回调函数参数。

### 查找服务器

```c
mr_event_server_t mr_event_server_find(const char *name);
```

| 参数      | 描述   |
|:--------|:-----|
| name    | 服务器名 |
| **返回**  |      |
| 服务器句柄   | 查找成功 |
| MR_NULL | 查找失败 |

### 添加服务器

```c
mr_err_t mr_event_server_add(mr_event_server_t server, const char *name, mr_size_t queue_length);
```

| 参数           | 描述                 |
|:-------------|:-------------------|
| server       | 服务器句柄              |
| name         | 服务器名               |
| queue_length | 服务器队列长度（同时处理事件的个数） |
| **返回**       |                    |
| MR_ERR_OK    | 添加成功               |
| 错误码          | 添加失败               |

### 移除服务器

```c
mr_err_t mr_event_server_remove(mr_event_server_t server);
```

| 参数           | 描述    |
|:-------------|:------|
| server       | 服务器句柄 |
| **返回**       |       |
| MR_ERR_OK    | 移除成功  |
| 错误码          | 移除失败  |

### 通知服务器，事件发生

```c
mr_err_t mr_event_server_notify(mr_event_server_t server, mr_uint8_t id);
```

| 参数           | 描述         |
|:-------------|:-----------|
| server       | 服务器句柄      |
| id           | 发生事件的客户端id |
| **返回**       |            |
| MR_ERR_OK    | 通知成功       |
| 错误码          | 通知失败       |

### 服务器分发事件，唤醒客户端

```c
void mr_event_server_handle(mr_event_server_t server);
```

| 参数           | 描述    |
|:-------------|:------|
| server       | 服务器句柄 |

### 查找客户端

```c
mr_event_client_t mr_event_client_find(mr_uint8_t id, mr_event_server_t server);
```

| 参数       | 描述    |
|:---------|:------|
| id       | 客户端id |
| server   | 服务器句柄 |
| **返回**   |       |
| 客户端句柄    | 查找成功  |
| MR_NULL  | 查找失败  |


### 创建新客户端

```c
mr_err_t mr_event_client_create(mr_uint8_t id,
                                mr_err_t (*cb)(mr_event_server_t server, void *args),
                                void *args,
                                mr_event_server_t server);
```

| 参数        | 描述     |
|:----------|:-------|
| id        | 客户端id  |
| cb        | 回调函数   |
| args      | 回调函数参数 |
| server    | 服务器句柄  |
| **返回**    |        |
| MR_ERR_OK | 创建成功   |
| 错误码       | 创建失败   |

### 删除客户端

```c
mr_err_t mr_client_delete(mr_uint8_t id, mr_event_server_t server);
```

| 参数        | 描述    |
|:----------|:------|
| id        | 客户端id |
| server    | 服务器句柄 |
| **返回**    |       |
| MR_ERR_OK | 删除成功  |
| 错误码       | 删除失败  |

## 使用示例：

```c
/* 定义事件 */
#define EVENT1                          1
#define EVENT2                          2
#define EVENT3                          3

/* 定义事件服务器 */
struct mr_event_server event_server;

mr_err_t event1_cb(mr_event_server_t server, void *args)
{
    printf("event1_cb\r\n");

    /* 通知事件服务器事件2发生 */
    mr_event_server_notify(server, EVENT2);

    return MR_ERR_OK;
}

mr_err_t event2_cb(mr_event_server_t server, void *args)
{
    printf("event2_cb\r\n");

    /* 通知事件服务器事件3发生 */
    mr_event_server_notify(server, EVENT3);

    return MR_ERR_OK;
}

mr_err_t event3_cb(mr_event_server_t server, void *args)
{
    printf("event3_cb\r\n");

    return MR_ERR_OK;
}

int main(void)
{
	/* 添加事件服务器到内核容器 */
    mr_event_server_add(&event_server, "server", 4);

	/* 创建事件客户端到事件服务器 */
    mr_event_client_create(EVENT1, event1_cb, MR_NULL, &event_server);
    mr_event_client_create(EVENT2, event2_cb, MR_NULL, &event_server);
    mr_event_client_create(EVENT3, event3_cb, MR_NULL, &event_server);

    /* 通知事件服务器事件1发生 */
    mr_event_server_notify(&event_server, EVENT1);

    while (1)
    {
        mr_event_server_handle(&event_server);
    }
}
```

现象：

```c
event1_cb
event2_cb
event3_cb
```