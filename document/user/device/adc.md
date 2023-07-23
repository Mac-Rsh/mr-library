# ADC设备使用指南

----------

## 概述

ADC（模数转换器）是一种可以将连续的模拟信号转换为离散的数字信号的设备。

----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用ADC设备初始化函数（如果实现了自动初始化,则无需调用）。
3. 打开 `mrconfig.h` 头文件中ADC宏开关。

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
MR_CTRL_CONFIG                                                      /* 配置命令 */
```

### 配置ADC设备通道

ADC控制参数原型如下：

```c
struct mr_adc_config  
{  
    mr_pos_t channel;                                               /* 通道 */  
    mr_state_t state;                                               /* 状态 */
};
```

- 通道：ADC支持的通道数，和芯片相关。
- 状态：支持打开和关闭ADC通道。

```c
MR_ADC_STATE_DISABLE                    0                           /* 关闭通道 */
MR_ADC_STATE_ENABLE                     1                           /* 打开通道 */
```

使用示例：

```c
#define ADC_CHANNEL                     5

/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开ADC1设备 */
mr_device_open(adc_device, MR_OPEN_RDONLY);

/* 配置ADC1通道5并使能 */
struct mr_adc_config adc_config;
adc_config.channel = ADC_CHANNEL;
adc_config.state = MR_ADC_STATE_ENABLE;
mr_device_ioctl(adc_device, MR_CTRL_CONFIG, &adc_config);
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
- 读取数据：ADC设备采集的原始值。

使用示例：

```c
#define ADC_CHANNEL                     5

/* 查找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开ADC1设备 */
mr_device_open(adc_device, MR_OPEN_RDONLY);

/* 配置ADC1通道5并使能 */
struct mr_adc_config adc_config;
adc_config.channel = ADC_CHANNEL;
adc_config.state = MR_ADC_STATE_ENABLE;
mr_device_ioctl(adc_device, MR_CTRL_CONFIG, &adc_config);

/* 读取ADC1通道5数值 */
mr_uint32_t adc_value = 0;
mr_device_read(adc_device, ADC_CHANNEL, &adc_value, sizeof(adc_value));
```