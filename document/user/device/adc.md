# ADC设备使用指南

----------

## 概述

ADC（模数转换器）是一种可以将连续的模拟信号转换为离散的数字信号的设备。

----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用ADC设备初始化函数（如果实现了自动初始化,则无需调用）。
3. 使能 `mrconfig.h` 头文件中ADC宏开关。

----------

## 查找ADC设备

```c
mr_device_t mr_device_find(const char *name);  
```

| 参数      | 描述     |
|:--------|:-------|
| name    | ADC设备名 |
| **返回**  |        |
| ADC句柄   | 查找设备成功 |
| MR_NULL | 查找设备失败 |

----------

## 打开ADC设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | ADC设备句柄 |
| flags     | 打开方式    |
| **返回**    |         |
| MR_ERR_OK | 打开设备成功  |
| 错误码       | 打开设备失败  |  

ADC设备支持以下打开方式：

```c
MR_OPEN_RDONLY                                                      /* 只读 */
```

----------

## 控制ADC设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | ADC设备句柄 |
| cmd       | 控制命令    |
| args      | 控制参数    |
| **返回**    |         |
| MR_ERR_OK | 控制设备成功  |
| 错误码       | 控制设备失败  |

ADC设备支持以下命令：

```c
MR_CTRL_SET_CONFIG                                                  /* 设置参数 */
MR_CTRL_GET_CONFIG                                                  /* 获取参数 */
```

### 设置ADC设备通道

ADC控制参数原型如下：

```c
struct mr_adc_config
{
    union
    {
        struct
        {
            mr_pos_t channel0: 1;
            mr_pos_t channel1: 1;
            mr_pos_t channel2: 1;
            mr_pos_t channel3: 1;
            mr_pos_t channel4: 1;
            mr_pos_t channel5: 1;
            mr_pos_t channel6: 1;
            mr_pos_t channel7: 1;
            mr_pos_t channel8: 1;
            mr_pos_t channel9: 1;
            mr_pos_t channel10: 1;
            mr_pos_t channel11: 1;
            mr_pos_t channel12: 1;
            mr_pos_t channel13: 1;
            mr_pos_t channel14: 1;
            mr_pos_t channel15: 1;
            mr_pos_t channel16: 1;
            mr_pos_t channel17: 1;
            mr_pos_t channel18: 1;
            mr_pos_t channel19: 1;
            mr_pos_t channel20: 1;
            mr_pos_t channel21: 1;
            mr_pos_t channel22: 1;
            mr_pos_t channel23: 1;
            mr_pos_t channel24: 1;
            mr_pos_t channel25: 1;
            mr_pos_t channel26: 1;
            mr_pos_t channel27: 1;
            mr_pos_t channel28: 1;
            mr_pos_t channel29: 1;
            mr_pos_t channel30: 1;
            mr_pos_t channel31: 1;
        };
        mr_pos_t _channel_mask;
    };
};
```

- 通道：ADC支持的通道数，和芯片相关。

```c
MR_ADC_CHANNEL_DISABLE                  0                           /* 失能通道 */
MR_ADC_CHANNEL_ENABLE                   1                           /* 使能通道 */
```

使用示例：

```c
/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开 */
mr_device_open(adc_device, MR_OPEN_RDONLY);

/* 获取参数 */
struct mr_adc_config adc_config;
mr_device_ioctl(adc_device, MR_CTRL_GET_CONFIG, &adc_config);

/* 使能通道5 */
adc_config.channel5 = MR_ADC_CHANNEL_ENABLE;
mr_device_ioctl(adc_device, MR_CTRL_SET_CONFIG, &adc_config);
```

----------

## 读取ADC设备通道输入值

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | ADC设备句柄 |
| pos       | 读取位置    |
| buffer    | 读取数据地址  |
| size      | 读取数据大小  |
| **返回**    |         |
| 实际读取的数据大小 |         |

- 读取位置：需要读取数据的通道。
- 读取数据：ADC设备采集的输入值。

使用示例：

```c
#define ADC_CHANNEL                     5

/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开 */
mr_device_open(adc_device, MR_OPEN_RDONLY);

/* 使能通道5 */
struct mr_adc_config adc_config;
adc_config.channel5 = MR_ADC_CHANNEL_ENABLE;
mr_device_ioctl(adc_device, MR_CTRL_SET_CONFIG, &adc_config);

/* 读取通道5输入值 */
mr_uint32_t adc_value = 0;
mr_device_read(adc_device, ADC_CHANNEL, &adc_value, sizeof(adc_value));
```