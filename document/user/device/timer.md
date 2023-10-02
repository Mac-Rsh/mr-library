# TIMER设备使用指南

----------

## 概述

定时器（Timer）是一种用于测量时间间隔、产生定时事件的重要外设。定时器通过提供精确的时间基准信号和计数功能，为系统提供了时间相关的功能和事件触发机制。

----------

## 准备

1. 调用TIMER设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中TIMER宏开关。

----------

## 查找TIMER设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数        | 描述           |
|:----------|:-------------|
| name      | TIMER设备名     | 
| **返回**    |              |
| TIMER     | 查找设备成功       |
| MR_NULL   | 查找设备失败       |

----------

## 打开TIMER设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint8_t oflags);
```

| 参数          | 描述         |
|:------------|:-----------|
| device      | TIMER设备    |
| oflags      | 打开方式       |
| **返回**      |            |
| MR_ERR_OK   | 打开设备成功     |
| 错误码         | 打开设备失败     |

TIMER设备支持以下打开方式：

```c
MR_DEVICE_OFLAG_RDONLY                                              /* 只读 */
MR_DEVICE_OFLAG_WRONLY                                              /* 只写 */
MR_DEVICE_OFLAG_RDWR                                                /* 可读可写 */
```

----------

## 控制TIMER设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述          |
|:----------|:------------|
| device    | TIMER设备     |
| cmd       | 控制命令        |
| args      | 控制参数        |
| **返回**    |             |
| MR_ERR_OK | 控制设备成功      |
| 错误码       | 控制设备失败      |


TIMER设备支持以下命令：

```c
MR_DEVICE_CTRL_SET_CONFIG                                           /* 设置参数 */
MR_DEVICE_CTRL_GET_CONFIG                                           /* 获取参数 */
MR_DEVICE_CTRL_SET_RX_CB                                            /* 设置接收（接收中断）回调函数 */
```

### 配置TIMER设备

```c
struct mr_timer_config
{
    mr_uint32_t mode;                                               /* 模式 */
};
```

- 模式：单次模式或者周期模式。

```c
MR_TIMER_MODE_PERIOD                                                 /* 周期模式 */
MR_TIMER_MODE_ONE_SHOT                                               /* 单次模式 */
```

使用示例：

```c
/* 查找TIMER1设备 */    
mr_device_t timer_device = mr_device_find("timer1");

/* 设置默认参数 */
struct mr_timer_config timer_config = MR_TIMER_CONFIG_DEFAULT;
mr_device_ioctl(timer_device, MR_DEVICE_CTRL_SET_CONFIG, &timer_config);

/* 获取参数 */
mr_device_ioctl(timer_device, MR_DEVICE_CTRL_GET_CONFIG, &timer_config);
```

### 设置TIMER设备超时回调函数

使用示例：

```c
/* 定义回调函数 */
mr_err_t timer_device_rx_cb(mr_device_t device, void *args)
{
    /* Do something */
}

/* 查找TIMER1设备 */
mr_device_t timer_device = mr_device_find("timer1");

/* 设置超时回调函数 */
mr_device_ioctl(serial_device, MR_DEVICE_CTRL_SET_RX_CB, timer_device_rx_cb);
```

----------

## TIMER设备读取已运行时间

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数          | 描述            |
|:------------|:--------------|
| device      | TIMER设备       |
| pos         | 读取位置          |
| buffer      | 读取数据          |
| size        | 读取数据大小        |
| **返回**      |               |
| 实际读取的数据大小   |               |

- 读取数据：TIMER设备已运行时间（单位us），类型为：uint32。

已运行时间指从定时启动后运行的时间，当定时超时且未再次启动，已运行时间将为0.

使用示例：

```c
/* 查找TIMER1设备 */
mr_device_t timer_device = mr_device_find("timer1");

/* 以可读可写方式打开 */
mr_device_open(timer_device, MR_DEVICE_OFLAG_RDWR);

/* 读取数据 */
mr_uint32_t buffer = 0;
mr_device_read(serial_device, 0, &buffer, sizeof(buffer));
```

----------

## TIMER设备写入定时时间

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述         |
|:----------|:-----------|
| device    | TIMER设备    |
| pos       | 写入位置       |
| buffer    | 写入数据       |
| size      | 写入数据大小     |
| **返回**    |            |
| 实际写入的数据大小 |            |

- 写入数据：TIMER设备定时时间（单位us），定时范围1us-4294.967295s,类型为：uint32。写入多个数据时将取最后一个有效数据。

使用示例：

```c
/* 查找TIMER1设备 */    
mr_device_t timer_device = mr_device_find("timer1");

/* 以可读可写方式打开 */
mr_device_open(timer_device, MR_DEVICE_OFLAG_RDWR);

/* 设置默认参数 */
struct mr_timer_config timer_config = MR_TIMER_CONFIG_DEFAULT;
mr_device_ioctl(timer_device, MR_DEVICE_CTRL_SET_CONFIG, &timer_config);

/* 写入数据（1ms） */
mr_uint32_t buffer = 1000;
mr_device_write(timer_device, 0, &buffer, sizeof(buffer));
```