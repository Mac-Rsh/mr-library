# ADC设备

<!-- TOC -->
* [ADC设备](#adc设备)
  * [打开ADC设备](#打开adc设备)
  * [关闭ADC设备](#关闭adc设备)
  * [控制ADC设备](#控制adc设备)
    * [设置/获取通道编号](#设置获取通道编号)
    * [设置/获取通道状态](#设置获取通道状态)
  * [读取ADC设备通道值](#读取adc设备通道值)
  * [使用示例：](#使用示例)
<!-- TOC -->

## 打开ADC设备

```c
int mr_dev_open(const char *name, int oflags);
```

| 参数      | 描述      |
|---------|---------|
| name    | 设备名称    |
| oflags  | 打开设备的标志 |
| **返回值** |         |
| `>=0`   | 设备描述符   |
| `<0`    | 错误码     |

- `name`：ADC设备名称一般为：`adcx`、`adc1`、`adc2`。
- `oflags`：打开设备的标志，支持 `MR_OFLAG_RDONLY`。

注：使用时应根据实际情况为不同的任务分别打开ADC设备，并使用适当的`oflags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭ADC设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

注：关闭设备时所有的通道都将被自动恢复到默认状态，重新打开后需要重新配置通道。

## 控制ADC设备

```c
int mr_dev_ioctl(int desc, int cmd, void *args);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| cmd     | 命令码   |
| args    | 命令参数  |
| **返回值** |       |
| `=0`    | 设置成功  |
| `<0`    | 错误码   |

- `cmd`：命令码，支持以下命令：
    - `MR_CTL_ADC_SET_CHANNEL`：设置通道编号。
    - `MR_CTL_ADC_SET_CHANNEL_STATE`：设置通道状态。
    - `MR_CTL_ADC_GET_CHANNEL`：获取通道编号。
    - `MR_CTL_ADC_GET_CHANNEL_STATE`：获取通道状态。

### 设置/获取通道编号

通道编号范围：`0` ~ `31`。

```c
/* 定义通道编号 */
#define CHANNEL_NUMBER                  5

/* 设置通道编号 */
mr_dev_ioctl(ds, MR_CTL_ADC_SET_CHANNEL, mr_make_local(int, CHANNEL_NUMBER));

/* 获取通道编号 */
int number;
mr_dev_ioctl(ds, MR_CTL_ADC_GET_CHANNEL, &number);
```

### 设置/获取通道状态

通道状态：

- `MR_ADC_STATE_DISABLE`：禁用通道。
- `MR_ADC_STATE_ENABLE`：启用通道。

```c
/* 设置通道状态 */
mr_dev_ioctl(ds, MR_CTL_ADC_SET_CHANNEL_STATE, mr_make_local(int, MR_ADC_STATE_ENABLE));

/* 获取通道状态 */
int state;
mr_dev_ioctl(ds, MR_CTL_ADC_GET_CHANNEL_STATE, &state);
```

## 读取ADC设备通道值

```c
ssize_t mr_dev_read(int desc, void *buf, size_t size);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 读取数据缓冲区 |
| size    | 读取数据大小  |
| **返回值** |         |
| `>=0`   | 读取数据大小  |
| `<0`    | 错误码     |

```c
/* 读取通道值 */
uint32_t data;
int ret = mr_dev_read(ds, &data, sizeof(data));
/* 是否读取成功 */
if (ret != sizeof(data))
{
    return ret;
}
```

注：读取数据为ADC原始数据。单次读取最小单位为`uint32_t`，即4个字节。

## 使用示例：

```c
#include "include/mr_lib.h"

/* 定义通道编号 */
#define CHANNEL_NUMBER                  5

/* 定义ADC设备描述符 */
int adc_ds = -1;

int adc_init(void)
{
    int ret = MR_EOK;

    /* 初始化ADC */
    int adc_ds = mr_dev_open("adc1", MR_OFLAG_RDONLY);
    if (adc_ds < 0)
    {
        mr_printf("ADC1 open failed: %s\r\n", mr_strerror(adc_ds));
        return adc_ds;
    }
    /* 打印ADC描述符 */
    mr_printf("ADC1 desc: %d\r\n", adc_ds);
    /* 设置到通道5 */
    mr_dev_ioctl(adc_ds, MR_CTL_ADC_SET_CHANNEL, mr_make_local(int, CHANNEL_NUMBER));
    /* 设置通道使能 */
    ret = mr_dev_ioctl(adc_ds, MR_CTL_ADC_SET_CHANNEL_STATE, mr_make_local(int, MR_ADC_STATE_ENABLE));
    if (ret < 0)
    {
        mr_printf("Channel5 enable failed: %s\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* 导出到自动初始化（APP级） */
MR_APP_EXPORT(adc_init);

int main(void)
{
    /* 自动初始化（adc_init函数将在此处自动调用） */
    mr_auto_init();

    while(1)
    {
        uint32_t data;
        int ret = mr_dev_read(adc_ds, &data, sizeof(data));
        if (ret != sizeof(data))
        {
            mr_printf("Read failed: %s\r\n", mr_strerror(ret));
        }
        mr_printf("ADC value: %d\r\n", data);
        mr_delay_ms(1000);
    }
}
```

ADC1通道5使能，间隔一秒读取一次ADC值并打印。
