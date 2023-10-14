# DAC设备使用指南

----------

## 概述

DAC（数模转换器）是一种可以将离散数字信号转换为连续模拟信号的设备。

----------

## 准备

1. 调用DAC设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中DAC宏开关。

----------

## 调用关系

![调用关系](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/dac_device.png)

----------

## 查找DAC设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数      | 描述     |
|:--------|:-------|
| name    | DAC设备名 |
| **返回**  |        |
| DAC     | 查找设备成功 |
| MR_NULL | 查找设备失败 |

----------

## 打开DAC设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint8_t oflags);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | DAC设备  |
| oflags    | 打开方式   |
| **返回**    |        |
| MR_ERR_OK | 打开设备成功 |
| 错误码       | 打开设备失败 |  

DAC设备支持以下打开方式：

```c
MR_DEVICE_OFLAG_WRONLY                                              /* 只写 */
```

----------

## 控制DAC设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | DAC设备  |
| cmd       | 控制命令   |
| args      | 控制参数   |
| **返回**    |        |
| MR_ERR_OK | 控制设备成功 |
| 错误码       | 控制设备失败 |

DAC设备支持以下命令：

```c
MR_DEVICE_CTRL_SET_CONFIG                                                  /* 设置参数 */
MR_DEVICE_CTRL_GET_CONFIG                                                  /* 获取参数 */
```

### 设置DAC设备通道

DAC控制参数原型如下：

```c
struct mr_dac_config
{
    struct mr_device_channel channel;
};
```

- 通道：DAC支持的通道数,和芯片相关。

```c
MR_DISABLE                              0                           /* 失能通道 */
MR_ENABLE                               1                           /* 使能通道 */
```

使用示例：

```c
/* 查找DAC1设备 */
mr_device_t dac_device = mr_device_find("dac1");

/* 以只写方式打开 */
mr_device_open(dac_device, MR_DEVICE_OFLAG_WRONLY);

/* 获取参数 */
struct mr_dac_config dac_config;
mr_device_ioctl(dac_device, MR_DEVICE_CTRL_GET_CONFIG, &dac_config);

/* 使能通道1 */
dac_config.channel.ch1 = MR_ENABLE;
mr_device_ioctl(dac_device, MR_DEVICE_CTRL_SET_CONFIG, &dac_config);
```

----------

## 写入DAC设备通道输出值

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | DAC设备  |
| pos       | 写入位置   |
| buffer    | 写入数据   |
| size      | 写入数据大小 |
| **返回**    |        |
| 实际写入的数据大小 |        |  

- 写入位置：需要写入数据的通道，有效范围：0-31。
- 写入数据：DAC设备输出值，类型为：uint32。

使用示例：

```c
#define DAC_CHANNEL                     1

/* 查找DAC1设备 */
mr_device_t dac_device = mr_device_find("dac1");

/* 以只写方式打开 */
mr_device_open(dac_device, MR_DEVICE_OFLAG_WRONLY);

/* 使能通道1 */
dac_config.channel.ch1 = MR_ENABLE;
mr_device_ioctl(dac_device, MR_DEVICE_CTRL_SET_CONFIG, &dac_config);

/* 写入通道1输出值 */
mr_uint32_t dac_value = 1200;
mr_device_write(dac_device, DAC_CHANNEL, &dac_value, sizeof(dac_value));
```