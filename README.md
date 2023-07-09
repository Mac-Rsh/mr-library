# 背景

随着国产微控制器的崛起，市场上的微控制器种类越来越多。然而，以前的微控制器开发往往忽略了整体框架和程序分层，导致更换微控制器型号往往需要更改应用层代码，这使得开发工作变得繁重且乏味。常见的开发方式大多分为两种:
裸机编程和RTOS编程，由于两种方式的代码编写方式存在巨大差异，因此在两种方式之间切换意味着需要进行大规模的工程修改。
**mr-library** 的目标是帮助开发者提高开发效率和代码通用性，降低平台迁移的难度。

 ----------

# mr-library 简介

**mr-library** 是一个嵌入式软件库，完全采用C语言编写，使用面向对象的设计方法，代码框架清晰，可以快速移植到不同的平台。它包括以下部分：

* 内核层: **mr-library** 的核心部分，包含容器、对象、服务等。将各种对象注册到内核维护的容器中，使得应用更加高效有序。
* 设备框架层: 提供统一的设备接口，将不同的设备接入到内核中。在应用层，仅需调用内核设备I/O接口即可访问设备。
* 硬件驱动层: 为设备框架层设备提供必要的驱动，当硬件更换时仅修改驱动层。
* 组件层: 通过内核提供的API实现不同的功能。包括但不限于虚拟文件系统、通用传感器模块、网络框架等。
* 软件包: 可独立使用，无依赖的软件包。

 ----------

# 代码目录

**mr-library** 的代码目录结构如下表所示：

| 名称       | 描述    |
|:---------|:------|
| bsp      | 板级支持包 |
| device   | 设备文件  |
| document | 文档    |
| driver   | 驱动文件  |
| include  | 库头文件  |
| module   | 组件    |
| package  | 软件包   |
| src      | 库源文件  |

 ----------

# 内核

内核中包含了容器、对象、服务等。

## 容器

容器负责统一管理注册到内核中的对象。

### 容器原型

```c
struct mr_container
{
    mr_uint8_t type;                                                /* 容器类型 */
    struct mr_list list;                                            /* 容器链表 */
};
```

- 容器类型:指定容器类型用以存放指定类型对象。
- 容器链表:所有注册到容器的对象都将链接到容器链表上，当对象被移除容器时也将从容器链表上移除。

## 对象

### 对象原型

```c
struct mr_object
{
    char name[MR_CONF_NAME_MAX];                                /* 对象名 */
    mr_uint8_t type;                                            /* 对象类型 */
    struct mr_list list;                                        /* 对象链表 */
};
```

- 对象名:对象的名称，同一容器不允许出现同名对象，不同容器允许对象重名。
- 对象链表:用于将对象注册到容器中。
- 对象类型:用于标记对象类型。

### 对象类型

内核维护了以下几类对象的容器:

```c
enum mr_object_type
{
    MR_OBJECT_TYPE_NULL,                                         /* 杂类对象 */
    MR_OBJECT_TYPE_DEVICE,                                       /* 设备对象 */
    MR_OBJECT_TYPE_SERVER,                                       /* 服务对象 */
};
```

### 对象操作接口

| 接口                    | 描述        |
|:----------------------|:----------|
| mr_object_find        | 从内核容器查找对象 |
| mr_object_add         | 添加对象到内核容器 |
| mr_object_remove      | 从内核容器移除对象 |
| mr_object_change_type | 更改对象类型    |
| mr_object_rename      | 重命名对象     |


 ----------

# 设备

硬件抽象成设备，通过统一的设备操作接口进行交互。

## 设备原型

```c
struct mr_device
{
    struct mr_object object;                                        /* 设备对象基类 */

    enum mr_device_type type;                                       /* 设备类型 */
    mr_uint16_t support_flag;                                       /* 设备支持的打开方式 */
    mr_uint16_t open_flag;                                          /* 设备状态 */
    mr_size_t ref_count;                                            /* 设备被引用次数 */
    void *data;                                                     /* 设备数据 */

    mr_err_t (*rx_cb)(mr_device_t device, void *args);              /* 设备接收回调函数 */
    mr_err_t (*tx_cb)(mr_device_t device, void *args);              /* 设备发送回调函数 */

    const struct mr_device_ops *ops;                                /* 设备操作方法 */
};
```

- 设备支持的打开方式:设备只能以支持的打开方式打开。
- 设备被引用次数:设备每被打开一次，引用+1，设备引用次数为0时设备关闭。
- 设备数据:设备运行所需的数据。

### 设备类型

```c
enum mr_device_type
{
    MR_DEVICE_TYPE_NONE,                                            /* 无类型设备 */
    MR_DEVICE_TYPE_PIN,                                             /* GPIO设备 */
    MR_DEVICE_TYPE_SPI_BUS,                                         /* SPI总线设备 */
    MR_DEVICE_TYPE_SPI,                                             /* SPI设备 */
    MR_DEVICE_TYPE_I2C_BUS,                                         /* I2C总线设备 */
    MR_DEVICE_TYPE_I2C,                                             /* I2C设备 */
    MR_DEVICE_TYPE_SERIAL,                                          /* UART设备*/
    MR_DEVICE_TYPE_ADC,                                             /* ADC设备 */
    MR_DEVICE_TYPE_DAC,                                             /* DAC设备 */
    MR_DEVICE_TYPE_PWM,                                             /* PWM设备 */
    MR_DEVICE_TYPE_TIMER,                                           /* TIMER设备 */
    MR_DEVICE_TYPE_FLASH,                                           /* FLASH设备 */
    /* ... */
};
```

### 设备操作方法

设备通过设备操作接口，最终会调用设备数据块中的设备操作方法。设备仅需实现设备打开方式所必须的方法即可。

```c
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*read)(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
};
```

| 方法    | 描述                                                       |
|:------|:---------------------------------------------------------|
| open  | 打开设备，同时完成设备配置。仅当设备为首次被打开时，会调用此方法打开设备。                    |
| close | 关闭设备。仅当设备被所有用户关闭时（设备引用次数为0），会调用此方法关闭设备。                  |
| ioctl | 控制设备。根据cmd命令控制设备。                                        |
| read  | 从设备读取数据，pos是设备读取位置（不同设备所表示意义不同，请查看设备详细手册），size为设备读取字节大小。 |
| write | 向设备写入数据，pos是设备写入位置（不同设备所表示意义不同，请查看设备详细手册），size为设备写入字节大小。 |

### 设备操作接口

| 接口              | 描述        |
|:----------------|:----------|
| mr_device_add   | 添加设备到内核容器 |
| mr_device_find  | 从内核容器查找设备 |
| mr_device_open  | 打开设备      |
| mr_device_close | 关闭设备      |
| mr_device_ioctl | 控制设备      |
| mr_device_read  | 从设备读取数据   |
| mr_device_write | 向设备写入数据   |

### GPIO设备使用示例：

```c
/* 寻找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为推挽输出模式 */
struct mr_pin_config pin_config = { 29, MR_PIN_MODE_OUTPUT };
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);

/* 设置B13(编号29)为高电平 */
mr_uint8_t pin_level = 1;
mr_device_write(pin_device, 29, &pin_level, sizeof(pin_level));

/* 获取B13电平 */
mr_device_read(pin_device, 29, &pin_level, sizeof(pin_level));

/* 定义回调函数 */
mr_err_t pin_device_cb(mr_device_t device, void *args)
{
    mr_uint32_t number = *(mr_uint32_t *)args;    /* 获取中断源 */
    
    /* 判断中断源是B13 */
    if (number == 29)
    {
    	/* Do something */
    }
}

/* 绑定PIN函数回调函数 */
mr_device_ioctl(pin_device, MR_CTRL_SET_RX_CB, pin_device_cb);
```

## 服务

### 事件服务

事件服务器是一种异步事件处理机制,它通过事件分发和回调的方式,可以有效地提高系统的异步处理能力、解耦性和可扩展性。

事件服务器包含两个主要组件:事件服务器和事件客户端。

- 事件服务器用于接收和分发事件,它内部维护一个事件队列用于存储待处理事件和一个事件列表用于存储注册的事件客户端。
- 事件客户端用于处理特定类型的事件,它需要注册到事件服务器并提供一个回调函数。

当事件发生时,事件服务器会将事件插入到其事件队列中进行缓存。事件服务器会周期性地从事件队列中取出事件进行分发,找到对应的事件客户端,然后调用其注册的回调函数进行事件处理。

### 事件服务原型

```c
/* 事件服务器 */
struct mr_event_server
{
    struct mr_object object;                                        /* 事件服务对象 */

    struct mr_fifo queue;                                           /* 事件队列 */
    mr_avl_t list;                                                  /* 事件链表 */
};

/* 事件客户端 */
struct mr_event
{
    struct mr_avl list;                                             /* 事件链表 */

    mr_err_t (*cb)(mr_event_server_t server, void *args);           /* 事件回调函数 */
    void *args;                                                     /* 事件回调函数参数 */
};
```

### 事件服务操作接口

| 接口                     | 描述            |
|:-----------------------|:--------------|
| mr_event_server_find   | 从内核容器查找事件服务器  |
| mr_event_server_add    | 添加事件服务器到内核容器  |
| mr_event_server_remove | 从内核容器移除事件服务器  |
| mr_event_server_notify | 通知事件服务器事件发生   |
| mr_event_server_handle | 事件服务器分发事件     |
| mr_event_client_find   | 从事件服务器查找事件客户端 |
| mr_event_client_create | 创建事件客户端到事件服务器 |
| mr_client_delete       | 从事件服务器移除事件客户端 |

### 事件服务使用示例：

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

 ----------

### 软件定时器服务

软件定时器是一种在软件层面实现计时功能的机制，通过软件定时器，可以在特定时间点或时间间隔触发特定的事件。软件定时器常用于实现周期性任务、超时处理、定时器中断等功能。

软件定时器包含两个主要组件:定时服务器和定时客户端。

- 定时服务器用于时间管理和定时器处理。
- 定时客户端用于处理特定的超时处理,它需要注册到定时服务器并提供一个回调函数。

软件定时器支持多时基信号和无限扩展定时器。

### 软件定时器服务原型

```c
/* 定时服务器 */
struct mr_soft_timer_server
{
    struct mr_object object;                                        /* 软件定时器服务对象 */
    
    uint32_t time;                                                  /* 当前时间 */
    struct mr_list list;                                            /* 定时器链表 */
};

/* 定时客户端 */
struct mr_soft_timer_client
{
    struct mr_list list;                                            /* 定时器链表 */
    mr_soft_timer_server server;                                    /* 所属的服务器 */
    uint32_t interval;                                              /* 定时间隔时间 */
    uint32_t timeout;                                               /* 下次超时时间 */
    
    int (*cb)(mr_soft_timer_client client, void *args);             /* 超时回调函数 */
    void *args;                                                     /* 超时回调函数参数 */
};
```

### 软件定时器服务操作接口

| 接口                                  | 描述               |
|:------------------------------------|:-----------------|
| mr_soft_timer_server_find           | 从内核容器查找定时服务器     |
| mr_soft_timer_server_add            | 添加定时服务器到内核容器     |
| mr_soft_timer_server_remove         | 从内核容器移除定时服务器     |
| mr_soft_timer_server_update         | 定时服务器时基信号更新      |
| mr_soft_timer_server_handle         | 定时服务器处理定时器       |
| mr_event_client_find                | 从定时服务器查找定时客户端    |
| mr_soft_timer_client_add            | 添加定时客户端到定时服务器    |
| mr_soft_timer_client_remove         | 从定时服务器移除定时客户端    |
| mr_soft_timer_client_start          | 启动定时客户端          |
| mr_soft_timer_client_stop           | 暂停定时客户端          |
| mr_soft_timer_client_add_then_start | 添加定时客户端到定时服务器并启动 |

### 软件定时器服务使用示例：
```c
/* 定义定时器服务器和客户端 */
struct mr_soft_timer_server server;
struct mr_soft_timer_client timer1, timer2, timer3;

mr_err_t timer1_callback(mr_soft_timer_client client, void *args)
{
    printf("timer1_callback\r\n");
    return MR_ERR_OK;
}

mr_err_t timer2_callback(mr_soft_timer_client client, void *args)
{
    printf("timer2_callback\r\n");
    return MR_ERR_OK;
}

mr_err_t timer3_callback(mr_soft_timer_client client, void *args)
{
    printf("timer3_callback\r\n");
    soft_timer_client_stop(client);
    return MR_ERR_OK;
}

int main(void)
{
    /* 添加服务器 */
    mr_soft_timer_server_add(&server, "soft-timer");
    
    /* 添加客户端到服务器并启动 */
    mr_soft_timer_client_add_then_start(&timer1, 5, timer1_callback, NULL, &server);
    mr_soft_timer_client_add_then_start(&timer2, 10, timer2_callback, NULL, &server);
    mr_soft_timer_client_add_then_start(&timer3, 15, timer3_callback, NULL, &server);
            
    while (1)
    {
        /* 更新服务器时钟 */
        mr_soft_timer_server_update(&server, 1);
        
        /* 服务器处理客户端超时（放在哪里，回调就将在哪里被调用） */
        mr_soft_timer_server_handle(&server);
    }
}
```

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。

 ----------

# 贡献代码

如果您对 **mr-library** 项目感兴趣，欢迎参与开发并成为代码贡献者。欢迎加入讨论群 **199915649(QQ)** 分享您的观点。