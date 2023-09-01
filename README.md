# 简介

**mr-library** 是一个面向嵌入式系统的轻量级框架，提供统一的底层驱动设备模型以及基础服务功能，具有模块化设计、可配置性和扩展性的特点，
可帮助开发者快速构建嵌入式应用程序。

**mr-library** 框架支持互斥锁、对象管理等基础内核功能。集成状态机（fsm）、异步事件驱动框架（event）、多时基软件定时器（soft-timer）
等服务。提供串口、SPI、I2C、ADC/DAC等常见外设的驱动设备模型，通过统一的驱动接口（open、close、ioctl、read、write）访问底层硬件设备，解耦底层驱动和应用。

### 应用场景

- MCU开发的低层驱动程序。
- RTOS实时操作系统的外挂框架（作为驱动设备框架使用）。
- 各类Iot和智能硬件产品的快速开发。

### 项目结构

![项目结构图](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/framework.jpg)

设备框架可作为中间层，为上层提供操作硬件的方法。

事件框架、软件定时器框架、状态机框架可外挂在实时操作系统上，协同运行。

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

## 状态机

状态机通过状态和事件来描述系统在不同场景下的行为。

状态机包含两个部分:状态机和状态。

- 状态机负责维护系统当前的状态，接收和分发事件。它内部会保存当前状态和可能产生的状态转换规则表。
- 状态代表系统在某一时间点的状态，,如“就绪”、“运行”等。每个状态定义了几种事件（触发状态转换）以及相应的状态转换效果。

当收到一个事件时，状态机器会根据当前状态和发生的事件，查找状态转换表决定应该变换到哪一个状态。相应地调用目标状态的处理方法来完成状态的切换。

### 状态机操作接口

| 接口            | 描述        |
|:--------------|:----------|
| mr_fsm_find   | 查找状态机     |
| mr_fsm_add    | 添加状态机     |
| mr_fsm_remove | 移除状态机     |
| mr_fsm_handle | 状态机处理     |
| mr_fsm_signal | 发送状态机信号   |
| mr_fsm_shift  | 切换状态机当前状态 |

### 状态机使用示例：

```c
/* 定义状态索引 */
enum fsm_state_index
{
    Fsm_State_1 = 0,
    Fsm_State_2,
};

/* 定义信号 */
enum fsm_signal
{
    Fsm_Signal_1 = 0,
    Fsm_Signal_2,
};

/* 定义状态回调 */
mr_err_t fsm1_cb(mr_fsm_t fsm, void *args)
{
    printf("fsm1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t fsm2_cb(mr_fsm_t fsm, void *args)
{
    printf("fsm2_cb\r\n");
    return MR_ERR_OK;
}

/* 定义信号敏感函数 */
mr_err_t fsm1_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    switch (signal)
    {
        case Fsm_Signal_2:
        {
            mr_fsm_shift(fsm, Fsm_State_2);
            return MR_ERR_OK;
        }
    
        default:
        return -MR_ERR_UNSUPPORTED;
    }
}

mr_err_t fsm2_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    switch (signal)
    {
        case Fsm_Signal_1:
        {
            mr_fsm_shift(fsm, Fsm_State_1);
            return MR_ERR_OK;
        }
    
        default:
        return -MR_ERR_UNSUPPORTED;
    }
}

/* 定义状态列表 */
struct mr_fsm_table fsm_table[] =
{
    {fsm1_cb, MR_NULL, fsm1_signal},
    {fsm2_cb, MR_NULL, fsm2_signal},
};

/* 定义状态机 */
struct mr_fsm fsm;

int main(void)
{
    /* 添加状态机 */
    mr_fsm_add(&fsm, "fsm", fsm_table, MR_ARRAY_SIZE(fsm_table));

    while (1)
    {
        /* 发送事件信号2，状态1->状态2 */
        mr_fsm_signal(&fsm, Fsm_Signal_2);
        
        /* 状态机处理 */
        mr_fsm_handle(&fsm);
        
        /* 发送事件信号1，状态2->状态1 */
        mr_fsm_signal(&fsm, Fsm_Signal_1);
    }
}
```

## 事件

事件是一种异步事件处理机制，它通过事件分发和回调的方式,可以有效地提高系统的异步处理能力、解耦性和可扩展性。

当事件发生时，其将缓存至事件队列。周期性地从事件队列中取出事件进行处理。

### 事件操作接口

| 接口               | 描述         |
|:-----------------|:-----------|
| mr_event_find    | 查找事件       |
| mr_event_add     | 添加事件       |
| mr_event_remove  | 移除事件       |
| mr_event_handle  | 发生事件处理     |
| mr_event_notify  | 通知事件发生（异步） |
| mr_event_trigger | 触发事件（同步）   |

### 事件使用示例：

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

## 软件定时器

软件定时器是一种在软件层面实现计时功能的机制，通过软件定时器，可以在特定时间点或时间间隔触发特定的事件。软件定时器常用于实现周期性任务、超时处理、定时器中断等功能。

### 软件定时器操作接口

| 接口                             | 描述        |
|:-------------------------------|:----------|
| mr_soft_timer_find             | 查找定时器     |
| mr_soft_timer_add              | 添加定时器     |
| mr_soft_timer_remove           | 移除定时器     |
| mr_soft_timer_update           | 定时器时间更新   |
| mr_soft_timer_handle           | 定时器超时处理   |
| mr_soft_timer_start            | 启动定时器     |
| mr_soft_timer_stop             | 停止定时器     |
| mr_soft_timer_restart          | 重启定时器     |
| mr_soft_timer_set_time         | 设置定时器时间间隔 |
| mr_soft_timer_get_current_time | 获取定时器当前时间 |

### 软件定时器使用示例：

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