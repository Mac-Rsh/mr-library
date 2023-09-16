# ADC设备使用指南

----------

## 概述

ADC（模数转换器）是一种可以将连续的模拟信号转换为离散的数字信号的设备。

----------

## 准备

1. 调用ADC设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中ADC宏开关。

----------

## 调用关系

![调用关系](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/adc_device.png)

----------

## 查找ADC设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数      | 描述     |
|:--------|:-------|
| name    | ADC设备名 |
| **返回**  |        |
| ADC设备   | 查找设备成功 |
| MR_NULL | 查找设备失败 |

----------

## 打开ADC设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint8_t oflags);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | ADC设备  |
| oflags    | 打开方式   |
| **返回**    |        |
| MR_ERR_OK | 打开设备成功 |
| 错误码       | 打开设备失败 |  

ADC设备支持以下打开方式：

```c
MR_DEVICE_OFLAG_RDONLY                                                        /* 只读 */
```

----------

## 控制ADC设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | ADC设备  |
| cmd       | 控制命令   |
| args      | 控制参数   |
| **返回**    |        |
| MR_ERR_OK | 控制设备成功 |
| 错误码       | 控制设备失败 |

ADC设备支持以下命令：

```c
MR_DEVICE_CTRL_SET_CONFIG                                                  /* 设置参数 */
MR_DEVICE_CTRL_GET_CONFIG                                                  /* 获取参数 */
```

### 设置ADC设备通道

ADC控制参数原型如下：

```c
struct mr_adc_config
{
    struct mr_device_channel channel;
};
```

- 通道：ADC支持的通道数，和芯片相关。

```c
MR_DISABLE                              0                           /* 失能通道 */
MR_ENABLE                               1                           /* 使能通道 */
```

使用示例：

```c
/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开 */
mr_device_open(adc_device, MR_DEVICE_OFLAG_RDONLY);

/* 获取参数 */
struct mr_adc_config adc_config;
mr_device_ioctl(adc_device, MR_DEVICE_CTRL_GET_CONFIG, &adc_config);

/* 使能通道5 */
adc_config.channel.ch5 = MR_ENABLE;
mr_device_ioctl(adc_device, MR_DEVICE_CTRL_SET_CONFIG, &adc_config);
```

----------

## 读取ADC设备通道输入值

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
```

| 参数        | 描述     |
|:----------|:-------|
| device    | ADC设备  |
| pos       | 读取位置   |
| buffer    | 读取数据地址 |
| size      | 读取数据大小 |
| **返回**    |        |
| 实际读取的数据大小 |        |

- 读取位置：需要读取数据的通道，有效范围：0-31。
- 读取数据：ADC设备采集的输入值，类型为：uint32。

使用示例：

```c
#define ADC_CHANNEL                     5

/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开 */
mr_device_open(adc_device, MR_DEVICE_OFLAG_RDONLY);

/* 使能通道5 */
struct mr_adc_config adc_config;
adc_config.channel.ch5 = MR_ENABLE;
mr_device_ioctl(adc_device, MR_DEVICE_CTRL_SET_CONFIG, &adc_config);

/* 读取通道5输入值 */
mr_uint32_t adc_value = 0;
mr_device_read(adc_device, ADC_CHANNEL, &adc_value, sizeof(adc_value));
```