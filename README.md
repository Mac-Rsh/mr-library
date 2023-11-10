# 简介

**mr-library** 是一个专为嵌入式系统设计的轻量级框架，模块化的软件架构充分考虑了嵌入式系统的资源和性能要求。
通过提供标准化的设备管理等，极大地简化了嵌入式应用的开发难度，帮助开发者快速构建嵌入式应用程序。

**设备框架** 为开发者提供标准化的接口（open、close、ioctl、read、write），解耦应用程序与底层硬件驱动，使应用开发无需硬件驱动的实现细节。
当硬件变动时，只需要适配底层驱动，上层应用可以无缝地迁移到新硬件上，大大提高了软硬件的可重用性和可扩展性。

### 应用场景

- MCU开发中硬件（gpio、uart_dev、spi、i2c、pwm等）标准化管理。
- RTOS实时操作系统的外挂设备框架。
- 各类Iot和智能硬件产品的快速开发。

### 项目结构

![项目结构图](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/framework.png)

 ----------

# 设备框架

通过标准化的接口，开发者能以面向对象的方式访问硬件设备，简化开发流程。框架实现了各种常用设备的通用设备模型，开发者可快速移植到不同的硬件平台。

使用组件化和分层设备思想，可对模型自由添加裁剪，帮助开发者快速开发出高可靠、可维护的程序。

### 标准化设备接口

设备框架提供标准化的操作接口，设备的所有操作都可通过以下接口实现：

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

SPI设备模型实现了总线和设备的分层。在使用时，开发者只需实例化对应的设备对象，将其挂载至对应总线，即可通过简单的读写接口访问设备，无需关心底层的SPI通信细节。

模型会根据设备的参数自动配置SPI通信参数，如时钟频率、SPI模式等，简化配置过程。同时模型也会自动控制设备的片选信号，开发者不再需要显式地控制CS管脚。

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
mr_device_ioctl(spi0_device, MR_DEVICE_CTRL_CONNECT, "spi1");
mr_device_ioctl(spi1_device, MR_DEVICE_CTRL_CONNECT, "spi1");

/* 以可读可写的方式打开SPI设备 */
mr_device_open(spi0_device, MR_DEVICE_OFLAG_RDWR);
mr_device_open(spi1_device, MR_DEVICE_OFLAG_RDWR);

/* 写入数据 */
char buffer0[] = "hello";
char buffer1[] = "world";
mr_device_write(spi0_device, -1, buffer0, sizeof(buffer0) - 1);
mr_device_write(spi1_device, -1, buffer1, sizeof(buffer1) - 1);

/* 读取数据 */
mr_device_read(spi0_device, -1, buffer0, sizeof(buffer0) - 1);
mr_device_read(spi1_device, -1, buffer1, sizeof(buffer1) - 1);

/* 关闭设备 */
mr_device_close(spi0_device);
mr_device_close(spi1_device);
```

 ----------

# 组件

组件是一种模块化的软件构建单元，专为嵌入式系统设计。组件设计充分考虑了嵌入式系统的资源限制，通过按需选择、自由组合的概念，极大提升了资源的利用率。

每个组件都承担特定的功能或任务，例如任务管理组件专注于对程序任务的管理，使得裸机与RTOS程序可高效移植。开发者可自主选择需要的组件，添加到项目中。

### etask组件

事件任务框架是一种用于处理异步事件和任务的编程模型。它提供了结构化的方式来管理和调度事件和任务，在并发环境中实现高效处理。
使用事件任务框架可以抽象和管理复杂的并发处理逻辑，提高代码的可维护性和可扩展性。使开发人员能够专注于事件处理逻辑，而无需关注底层的并发细节。
事件任务框架支持动态添加和移除事件任务，通过添加或替换事件任务来实现新增功能或修改业务逻辑，而无需修改其他代码。这种可插拔性使系统更灵活、能快速响应需求变化。

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

### 更多组件请查看 `module` 目录

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

# 更多使用教程与开发者文档请查看 `document` 目录

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。

 ----------

# 贡献代码

如果您对 **mr-library** 项目感兴趣，欢迎参与开发并成为代码贡献者。欢迎加入讨论群 **199915649(QQ)**。