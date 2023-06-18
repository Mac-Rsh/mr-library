# ADC使用指南
----------

## 准备工作

1. 引用 mrdrv.h 头文件以使用驱动部分。
2. 调用adc设备注册函数(如果实现了自动初始化,则无需调用)。
3. 打开 mrconfig.h 头文件中ADC宏开关。

 ----------

## 寻找ADC设备

```c
mr_device_t mr_device_find(const char *name);  
```

| 参数      | 描述     |
|:--------|:-------|
| name    | ADC设备名 |
| **返回**  |        |
| ADC句柄   | 寻找设备成功 |
| MR_NULL | 寻找设备失败 |

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

ADC设备支持以下打开方式:

```c
MR_OPEN_RDONLY                          /* 只读 */
```

## 配置ADC通道

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

通过`ioctl`函数将控制参数配置到ADC设备。ADC控制参数原型如下:

```c
struct mr_adc_config  
{  
    mr_uint16_t channel;                /* 通道 */  
    mr_uint8_t state;                   /* 状态 */
};
```

**通道**:ADC支持的通道数,和芯片相关。

**状态**:支持打开和关闭ADC通道。

```c
#define MR_ADC_STATE_DISABLE 0          /* 关闭通道 */
#define MR_ADC_STATE_ENABLE 1           /* 打开通道 */
```

使用示例如下所示:

```c
/* 寻找ADC1设备 */    
mr_device_t adc_device = mr_device_find("adc1");

/* 以只读方式打开ADC1设备 */
mr_device_open(adc_device, MR_OPEN_RDONLY);

/* 配置ADC1通道5并使能 */
struct mr_adc_config adc_config = {5, MR_ADC_STATE_ENABLE};
mr_device_ioctl(adc_device, MR_CTRL_CONFIG, &adc_config);
```

## 获取ADC通道值

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | ADC设备句柄 |
| pos       | 读取位置    |
| buffer    | 读取数据    |
| size      | 读取数据大小  |
| **返回**    |         |
| 实际读取的数据大小 |         |

**读取位置**:指需要读取数据的通道。

使用示例如下所示:

```c
/* 获取ADC1通道5值 */
mr_uint32_t adc_value = 0;
mr_device_read(adc_device, 5, &adc_value, sizeof(adc_value));
```