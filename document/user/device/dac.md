# DAC设备使用指南

----------

## 概述

DAC（数模转换器）是一种可以将离散数字信号转换为连续模拟信号的设备。

 ----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用DAC设备初始化函数（如果实现了自动初始化,则无需调用）。
3. 打开 `mrconfig.h` 头文件中DAC宏开关。

 ----------

## 查找DAC设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数      | 描述     |
|:--------|:-------|
| name    | DAC设备名 |
| **返回**  |        |
| DAC句柄   | 查找设备成功 |
| MR_NULL | 查找设备失败 |

 ----------

## 打开DAC设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | DAC设备句柄 |
| flags     | 打开方式    |
| **返回**    |         |
| MR_ERR_OK | 打开设备成功  |
| 错误码       | 打开设备失败  |  

DAC设备支持以下打开方式：

```c
MR_OPEN_WRONLY                                              /* 只写 */
```

 ----------

## 控制DAC设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | DAC设备句柄 |
| cmd       | 控制命令    |
| args      | 控制参数    |
| **返回**    |         |
| MR_ERR_OK | 控制设备成功  |
| 错误码       | 控制设备失败  |

DAC设备支持以下命令：

```c
MR_CTRL_CONFIG                                                      /* 配置命令 */
```

### 配置DAC设备通道

DAC控制参数原型如下：

```c
struct mr_dac_config  
{  
    mr_pos_t channel;                                               /* 通道 */  
    mr_state_t state;                                               /* 状态 */  
};
```

- 通道：DAC支持的通道数,和芯片相关。
- 状态：支持打开和关闭DAC通道。

```c
#define MR_DAC_STATE_DISABLE            0                           /* 关闭通道 */
#define MR_DAC_STATE_ENABLE             1                           /* 打开通道 */
```

使用示例：

```c
#define DAC_CHANNEL                     3

/* 寻找DAC1设备 */
mr_device_t dac_device = mr_device_find("dac1");

/* 以只写方式打开DAC1设备 */
mr_device_open(dac_device, MR_OPEN_WRONLY);

/* 配置DAC1通道3并使能 */
struct mr_dac_config dac_config;
dac_config.channel = DAC_CHANNEL;
dac_config.state = MR_ADC_STATE_ENABLE;
mr_device_ioctl(dac_device, MR_CTRL_CONFIG, &dac_config);
```

 ----------

## 写入DAC设备通道输出值

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | DAC设备句柄 |
| pos       | 写入位置    |
| buffer    | 写入数据    |
| size      | 写入数据大小  |
| **返回**    |         |
| 实际写入的数据大小 |         |  

- 写入位置：需要写入数据的通道。
- 写入数据：DAC设备输出值。

使用示例：

```c
#define DAC_CHANNEL                     3

/* 寻找DAC1设备 */
mr_device_t dac_device = mr_device_find("dac1");

/* 以只写方式打开DAC1设备 */
mr_device_open(dac_device, MR_OPEN_WRONLY);

/* 配置DAC1通道3并使能 */
struct mr_dac_config dac_config;
dac_config.channel = DAC_CHANNEL;
dac_config.state = MR_ADC_STATE_ENABLE;
mr_device_ioctl(dac_device, MR_CTRL_CONFIG, &dac_config);

/* 写入DAC1通道3输出值为1200 */
mr_uint32_t dac_value = 1200;
mr_device_write(dac_device, DAC_CHANNEL, &dac_value, sizeof(dac_value));
```