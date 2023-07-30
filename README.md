# 简介

**mr-library** 是一个面向嵌入式系统的轻量级框架，提供统一的底层驱动设备模型以及基础服务功能，具有模块化设计、可配置性和扩展性的特点，
可帮助开发者快速构建嵌入式应用程序。

**mr-library** 框架支持互斥锁、对象管理等基础内核功能。集成异步事件驱动框架（event）、多时基软件定时器(soft-timer)
等服务。提供串口、SPI、I2C、ADC/DAC等常见外设的驱动设备模型，通过统一的驱动接口（open、close、ioctl、read、write）访问底层硬件设备，解耦底层驱动和应用。

### 应用场景

- MCU开发的低层驱动程序。
- RTOS实时操作系统的外挂框架（作为驱动设备框架使用）。
- 各类IoT和智能硬件产品的快速开发。

 ----------

# 驱动设备框架

开发者能以面向对象的方式访问外设，简化驱动逻辑的开发流程。框架实现了常用外设的通用驱动模板，开发者可以快速移植到不同的硬件平台。

驱动设备框架支持普通设备的通用接口、总线设备自动总线控制、多种设备的中断接管。

### 驱动设备接口

设备驱动框架提供统一的操作接口，设备的所有操作都需要通过以下接口实现：

| 接口              | 描述      |
|:----------------|:--------|
| mr_device_add   | 添加设备    |
| mr_device_find  | 查找设备    |
| mr_device_open  | 打开设备    |
| mr_device_close | 关闭设备    |
| mr_device_ioctl | 控制设备    |
| mr_device_read  | 从设备读取数据 |
| mr_device_write | 向设备写入数据 |

### SPI设备使用示例：

```c
/* 定义SPI设备 */
#define SPI_DEVICE0_CS_NUMBER           10
#define SPI_DEVICE1_CS_NUMBER           20
struct mr_spi_device spi_device0, spi_device1;

/* 添加SPI设备 */
mr_spi_device_add(&spi_device0, "spi10", SPI_DEVICE0_CS_NUMBER);
mr_spi_device_add(&spi_device1, "spi11", SPI_DEVICE1_CS_NUMBER);

/* 查找SPI设备 */
mr_device_t spi0_device = mr_device_find("spi10");
mr_device_t spi1_device = mr_device_find("spi11");

/* 挂载总线 */
mr_device_ioctl(spi0_device, MR_CTRL_ATTACH, "spi1");
mr_device_ioctl(spi1_device, MR_CTRL_ATTACH, "spi1");

/* 以可读可写的方式打开SPI设备 */
mr_device_open(spi0_device, MR_OPEN_RDWR);
mr_device_open(spi1_device, MR_OPEN_RDWR);

/* 发送数据 */
char buffer0[] = "hello";
char buffer1[] = "world";
mr_device_write(spi0_device, 0, buffer0, sizeof(buffer0) - 1);
mr_device_write(spi1_device, 0, buffer1, sizeof(buffer1) - 1);

/* 读取数据 */
mr_device_read(spi0_device, 0, buffer0, sizeof(buffer0) - 1);
mr_device_read(spi1_device, 0, buffer1, sizeof(buffer1) - 1);

/* 关闭设备 */
mr_device_close(spi0_device);
mr_device_close(spi1_device);
```

 ----------

# 服务框架

**mr-library** 框架集成了轻量级的服务框架，用于构建嵌入式开发中的应用服务，支持异步事件监听，多时基软件定时器等。
通过服务框架完成对应用层不同应用间的解耦，实现应用程序的模块化，可裁剪，业务逻辑清晰，开发快速，代码高度复用。

## 事件服务

事件服务是一种异步事件处理机制,它通过事件分发和回调的方式,可以有效地提高系统的异步处理能力、解耦性和可扩展性。

事件服务包含两个部分:事件服务器和事件。

- 事件服务器用于接收和分发事件，其内部维护一个事件队列用于存储待处理事件和一个事件列表用于存储已创建事件。
- 事件需要创建到事件服务器并提供一个回调函数。

当事件发生时,事件服务器会将事件插入到事件队列中进行缓存。事件服务器会周期性地从事件队列中取出事件进行分发,找到对应的事件回调进行事件处理。

### 事件服务操作接口

| 接口                      | 描述      |
|:------------------------|:--------|
| mr_event_server_find    | 查找事件服务器 |
| mr_event_server_add     | 添加事件服务器 |
| mr_event_server_remove  | 移除事件服务器 |
| mr_event_server_handle  | 事件服务器处理 |
| mr_event_create         | 创建事件    |
| mr_event_delete         | 移除事件    |
| mr_event_notify         | 通知事件发生  |
| mr_event_trigger        | 触发事件    |

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
    mr_event_notify(EVENT2, server);
    return MR_ERR_OK;
}

mr_err_t event2_cb(mr_event_server_t server, void *args)
{
    printf("event2_cb\r\n");

    /* 通知事件服务器事件3发生 */
    mr_event_notify(EVENT3, server)
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
    
    /* 创建事件到服务器 */
    mr_event_create(EVENT1, event1_cb, MR_NULL, &event_server);
    mr_event_create(EVENT2, event2_cb, MR_NULL, &event_server);
    mr_event_create(EVENT3, event3_cb, MR_NULL, &event_server);
    
    /* 通知事件服务器事件1发生 */
    mr_event_notify(EVENT1, &event_server);
    
    while (1)
    {
        /* 事件服务器处理 */
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

## 软件定时器服务

软件定时器是一种在软件层面实现计时功能的机制，通过软件定时器，可以在特定时间点或时间间隔触发特定的事件。软件定时器常用于实现周期性任务、超时处理、定时器中断等功能。

软件定时器包含两个主要组件:定时服务器和定时器。

- 定时服务器用于时间管理和定时器处理。
- 定时器用于处理特定的超时处理,它需要注册到定时服务器并提供一个回调函数。

### 软件定时器服务操作接口

| 接口                             | 描述          |
|:-------------------------------|:------------|
| mr_soft_timer_server_find      | 查找定时服务器     |
| mr_soft_timer_server_add       | 添加定时服务器     |
| mr_soft_timer_server_remove    | 移除定时服务器     |
| mr_soft_timer_server_update    | 定时服务器时基信号更新 |
| mr_soft_timer_server_handle    | 定时服务器处理     |
| mr_soft_timer_create           | 创建定时器       |
| mr_soft_timer_delete           | 删除定时器       |
| mr_soft_timer_start            | 启动定时器       |
| mr_soft_timer_stop             | 暂停定时器       |
| mr_soft_timer_create_and_start | 创建定时器并启动    |

### 软件定时器服务使用示例：

```c
/* 定义定时器 */
#define TIMER1                          1
#define TIMER2                          2
#define TIMER3                          3

/* 定义定时服务器 */
struct mr_soft_timer_server server;

mr_err_t timer1_callback(mr_soft_timer_server_t server, void *args)
{
    printf("timer1_callback\r\n");
    return MR_ERR_OK;
}

mr_err_t timer2_callback(mr_soft_timer_server_t server, void *args)
{
    printf("timer2_callback\r\n");
    return MR_ERR_OK;
}

mr_err_t timer3_callback(mr_soft_timer_server_t server, void *args)
{
    printf("timer3_callback\r\n");
    
    /* 暂停定时器3 */
    mr_soft_timer_stop(TIMER3, server);
    return MR_ERR_OK;
}

int main(void)
{
    /* 添加定时服务器 */
    mr_soft_timer_server_add(&server, "soft-timer");

    /* 创建定时器并启动 */
    mr_soft_timer_create_and_start(TIMER1, 5, timer1_callback, MR_NULL, &server);
    mr_soft_timer_create_and_start(TIMER2, 10, timer2_callback, MR_NULL, &server);
    mr_soft_timer_create_and_start(TIMER3, 15, timer3_callback, MR_NULL, &server);

    while (1)
    {
        /* 更新定时服务器时钟 */
        mr_soft_timer_server_update(&server, 1);
        
        /* 定时服务器处理（放在哪里，回调就将在哪里被调用） */
        mr_soft_timer_server_handle(&server);
    }
}
```

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

* 内核层: **mr-library** 的核心部分，实现对象管理，设备控制，服务接口等。
* 设备层: 提供统一的设备接口，将设备接入到内核中。
* 驱动层: 为设备提供底层硬件驱动，当硬件更换时仅需修改驱动层。
* 组件层: 通过框架提供的API实现不同的功能。包括但不限于虚拟文件系统、通用传感器模块、网络框架等。
* 软件包: 可独立使用，无依赖的软件包。

 ----------

# 更多文档请查看 `document` 目录

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。

 ----------

# 贡献代码

如果您对 **mr-library** 项目感兴趣，欢迎参与开发并成为代码贡献者。欢迎加入讨论群 **199915649(QQ)**。