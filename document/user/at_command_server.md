# AT指令服务文档

AT指令服务器包含两个部分:AT指令服务器和AT指令。

- AT指令服务器用于解析AT指令，内部维护这一块指令缓存和一个AT指令列表用于存储注册的AT指令。
- AT指令需要注册到AT指令服务器并提供一个回调函数。

当AT指令服务器解析到对应的AT指令时，会将指令及其参数打包到内部的指令缓存中。AT指令服务器会周期性的从指令缓存中取出AT指令，找到对应的AT指令回调进行处理。

## AT指令服务原型

```c
/* AT指令服务器 */
struct mr_at_command_server
{
    struct mr_object object;                                        /* AT指令服务对象 */

    mr_uint8_t type;                                                /* AT指令类型 */
    mr_avl_t list;                                                  /* AT指令链表 */
    void *buffer;                                                   /* AT指令缓存 */
    size_t queue_size;                                              /* AT指令队列长度 */
};
```

- object:对象(用于将AT指令服务器添加到容器中)。
- type:AT指令类型(AT指令服务器可分为主机和从机)。
- list:服务器链表(用于存储所有已注册的AT指令)。
- buffer:AT指令缓存(用于存储打包完成的指令)。
- queue_size:AT指令队列长度。

```c
struct mr_at_command
{
    struct mr_avl list;                                             /* AT指令链表 */
    mr_at_command_server_t server;                                  /* AT指令所属服务器 */
    const char *cmd;                                                /* AT指令 */

    mr_err_t (*cb)(mr_at_command_t at_command, void *args);         /* AT指令回调函数 */
};
```

- list:AT指令链表(用于将AT指令添加到服务器的AT指令列表中)。
- server:AT指令所属的服务器。
- cmd:AT指令。
- cb：回调函数。

### 查找服务器

```c
mr_at_command_server_t mr_at_command_server_find(const char *name);
```

| 参数      | 描述   |
|:--------|:-----|
| name    | 服务器名 |
| **返回**  |      |
| 服务器句柄   | 查找成功 |
| MR_NULL | 查找失败 |

### 添加服务器

```c
mr_err_t mr_at_command_server_add(mr_at_command_server_t server,
                                  const char *name,
                                  mr_uint8_t type,
                                  mr_size_t queue_length);
```

| 参数           | 描述                   |
|:-------------|:---------------------|
| server       | 服务器句柄                |
| name         | 服务器名                 |
| type         | 服务器类型                |
| queue_length | 服务器队列长度（同时处理AT指令的个数） |
| **返回**       |                      |
| MR_ERR_OK    | 添加成功                 |
| 错误码          | 添加失败                 |

### 移除服务器

```c
mr_err_t mr_at_command_server_remove(mr_at_command_server_t server);
```

| 参数        | 描述    |
|:----------|:------|
| server    | 服务器句柄 |
| **返回**    |       |
| MR_ERR_OK | 移除成功  |
| 错误码       | 移除失败  |

### 服务器处理

```c
void mr_at_command_server_handle(mr_at_command_server_t server);
```

| 参数     | 描述    |
|:-------|:------|
| server | 服务器句柄 |

### 服务器解析

```c
void mr_at_command_server_parse(mr_at_command_server_t server, mr_uint8_t data);
```

| 参数     | 描述     |
|:-------|:-------|
| server | 服务器句柄  |
| data   | 接收到的数据 |

### 创建新AT指令

```c
mr_err_t mr_at_command_create(const char *command,
                              mr_err_t (*cb)(mr_at_command_t at_command, void *args),
                              mr_at_command_server_t server);
```

| 参数            | 描述     |
|:--------------|:-------|
| command       | AT指令   |
| cb            | 回调函数   |
| server        | 服务器句柄  |
| **返回**        |        |
| MR_ERR_OK     | 创建成功   |
| 错误码           | 创建失败   |

### 删除AT指令

```c
mr_err_t mr_at_command_delete(const char *command, mr_at_command_server_t server);
```

| 参数          | 描述    |
|:------------|:------|
| command     | AT指令  |
| server      | 服务器句柄 |
| **返回**      |       |
| MR_ERR_OK   | 删除成功  |
| 错误码         | 删除失败  |

### 获取AT指令

```c
const char* mr_at_command_get_cmd(mr_at_command_t at_command);
```

| 参数          | 描述     |
|:------------|:-------|
| at_command  | AT指令句柄 |
| **返回**      |        |
| AT指令        |        |

### 获取AT参数

```c
mr_size_t mr_at_command_get_args(const char *source, const char *format, ...);
```

| 参数     | 描述   |
|:-------|:-----|
| source | 原始数据 |
| format | 格式   |
| **返回** |      |
| 获取的大小  |      |

### 设置AT应答

```c
mr_size_t mr_at_command_set_response(const char *buffer, mr_size_t bufsz, const char *format, ...);
```

| 参数     | 描述    |
|:-------|:------|
| buffer | 存储缓冲区 |
| bufsz  | 缓冲区大小 |
| format | 格式    |
| **返回** |       |
| 设置的大小  |       |

### 设置AT成功应答

```c
mr_size_t mr_at_command_set_response_ok(const char *buffer, mr_size_t bufsz);
```

| 参数     | 描述    |
|:-------|:------|
| buffer | 存储缓冲区 |
| bufsz  | 缓冲区大小 |
| **返回** |       |
| 设置的大小  |       |

### 设置AT错误应答

```c
mr_size_t mr_at_command_set_response_error(const char *buffer, mr_size_t bufsz);
```

| 参数     | 描述    |
|:-------|:------|
| buffer | 存储缓冲区 |
| bufsz  | 缓冲区大小 |
| **返回** |       |
| 设置的大小  |       |


## 主从机模式说明：

### 主机模式：
接收从机发送的URC指令或者从机回传的指令（不包括OK和ERROR），注册的指令需为+xxx。

### 从机模式：
接收主机发送的配置指令，并作出响应，注册的指令需为AT+xxx。

## 主机模式使用示例：

```c
/* 定义AT指令服务器 */
struct mr_at_command_server at_server;

mr_err_t at1_cb(mr_at_command_t at_command, void *args)
{
    printf("Callback:+Hi\r\n");

    return MR_ERR_OK;
}

mr_err_t at2_cb(mr_at_command_t at_command, void *args)
{
    printf("Callback:+Hello?\r\n");

    return MR_ERR_OK;
}

mr_err_t at3_cb(mr_at_command_t at_command, void *args)
{
    int ack = 0;

    /* 解析参数 */
    mr_at_command_get_args(args, "%d", &ack);

    printf("Callback:+Ack:, args:%d\r\n",ack);

    return MR_ERR_OK;
}

void receive(char *string)
{
    for (int i = 0; i < strlen(string); i++)
    {
        mr_at_command_server_parse(&at_server, string[i]);
    }
}

int main(void)
{
    mr_at_command_server_add(&at_server, "at-server", MR_AT_COMMAND_SERVER_TYPE_HOST, 3);
    mr_at_command_create("+Hi", at1_cb, &at_server);
    mr_at_command_create("+Hello?", at2_cb, &at_server);
    mr_at_command_create("+Ack:", at3_cb, &at_server);

    /* 压入测试AT指令 */
    receive("+Hi\n");
    receive("+Hello?\r");
    receive("+Ack:10\r\n");

    while (1)
    {
        mr_at_command_server_handle(&at_server);
    }
}
```

现象：

```c
Callback:+Hi
Callback:+Hello?
Callback:+Ack:, args:10
```

## 从机模式使用示例：

```c
/* 定义AT指令服务器 */
struct mr_at_command_server at_server;

mr_err_t at1_cb(mr_at_command_t at_command, void *args)
{
    char buffer[10];

    /* 应答OK */
    mr_at_command_set_response_ok(buffer, sizeof(buffer));
    printf("Callback:AT+Hi, response:%s\r\n",buffer);

    return MR_ERR_OK;
}

mr_err_t at2_cb(mr_at_command_t at_command, void *args)
{
    char buffer[10];

    /* 应答ERROR */
    mr_at_command_set_response_error(buffer, sizeof(buffer));
    printf("Callback:AT+Hello?, response:%s\r\n",buffer);

    return MR_ERR_OK;
}

mr_err_t at3_cb(mr_at_command_t at_command, void *args)
{
    int ack = 0;
    char buffer[10];

    /* 解析参数 */
    mr_at_command_get_args(args, "%d", &ack);

    /* 应答指令 */
    mr_at_command_set_response(buffer, sizeof(buffer), "+Ack=%d",ack);
    printf("Callback:AT+Ack=, response:%s\r\n",buffer);

    return MR_ERR_OK;
}

void receive(char *string)
{
    for (int i = 0; i < strlen(string); i++)
    {
        mr_at_command_server_parse(&at_server, string[i]);
    }
}

int main(void)
{
    mr_at_command_server_add(&at_server, "at-server", MR_AT_COMMAND_SERVER_TYPE_SLAVE, 3);
    mr_at_command_create("AT+Hi", at1_cb, &at_server);
    mr_at_command_create("AT+Hello?", at2_cb, &at_server);
    mr_at_command_create("AT+Ack=", at3_cb, &at_server);

    /* 压入测试AT指令 */
    receive("AT+Hi\n");
    receive("AT+Hello?\r");
    receive("AT+Ack=10\r\n");

    while (1)
    {
        mr_at_command_server_handle(&at_server);
    }
}
```

现象：

```c
Callback:AT+Hi, response:OK
Callback:AT+Hello?, response:ERROR
Callback:AT+Ack=, response:+Ack=10
```