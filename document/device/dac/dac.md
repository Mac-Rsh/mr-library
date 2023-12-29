# DAC设备

[English](dac_EN.md)

<!-- TOC -->
* [DAC设备](#dac设备)
  * [打开DAC设备](#打开dac设备)
  * [关闭DAC设备](#关闭dac设备)
  * [控制DAC设备](#控制dac设备)
    * [设置/获取通道编号](#设置获取通道编号)
    * [设置/获取通道状态](#设置获取通道状态)
  * [写入DAC设备通道值](#写入dac设备通道值)
  * [使用示例：](#使用示例)
<!-- TOC -->

## 打开DAC设备

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

- `name`：DAC设备名称一般为：`dacx`、`dac1`、`dac2`。
- `oflags`：打开设备的标志，支持 `MR_OFLAG_WRONLY`。

注：使用时应根据实际情况为不同的任务分别打开DAC设备，并使用适当的`oflags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭DAC设备

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

## 控制DAC设备

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
    - `MR_CTL_DAC_SET_CHANNEL`：设置通道编号。
    - `MR_CTL_DAC_SET_CHANNEL_STATE`：设置通道状态。
    - `MR_CTL_DAC_GET_CHANNEL`：获取通道编号。
    - `MR_CTL_DAC_GET_CHANNEL_STATE`：获取通道状态。

### 设置/获取通道编号

通道编号范围：`0` ~ `31`。

```c
/* 定义通道编号 */
#define CHANNEL_NUMBER                  5

/* 设置通道编号 */
mr_dev_ioctl(ds, MR_CTL_DAC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* 获取通道编号 */
int number;
mr_dev_ioctl(ds, MR_CTL_DAC_GET_CHANNEL, &number);
```

### 设置/获取通道状态

通道状态：

- `MR_DAC_STATE_DISABLE`：禁用通道。
- `MR_DAC_STATE_ENABLE`：启用通道。

```c
/* 设置通道状态 */
mr_dev_ioctl(ds, MR_CTL_DAC_SET_CHANNEL_STATE, MR_MAKE_LOCAL(int, MR_DAC_STATE_ENABLE));

/* 获取通道状态 */
int state;
mr_dev_ioctl(ds, MR_CTL_DAC_GET_CHANNEL_STATE, &state);
```

## 写入DAC设备通道值

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t size);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 写入数据缓冲区 |
| size    | 写入数据大小  |
| **返回值** |         |
| `>=0`   | 写入数据大小  |
| `<0`    | 错误码     |

```c
/* 写入通道值 */
uint32_t data = 2048;
int ret = mr_dev_write(ds, &data, sizeof(data));
/* 是否写入成功 */
if (ret != sizeof(data))
{
    return ret;
}
```

注：写入数据为DAC原始数据。单次写入最小单位为`uint32_t`，即4个字节。

## 使用示例：

```c
#include "include/mr_lib.h"

/* 定义通道编号 */
#define CHANNEL_NUMBER                  5

/* 定义DAC设备描述符 */
int dac_ds = -1;

int dac_init(void)
{
    int ret = MR_EOK;

    /* 初始化DAC */
    dac_ds = mr_dev_open("dac1", MR_OFLAG_RDONLY);
    if (dac_ds < 0)
    {
        mr_printf("DAC1 open failed: %s\r\n", mr_strerror(dac_ds));
        return dac_ds;
    }
    /* 打印DAC描述符 */
    mr_printf("DAC1 desc: %d\r\n", dac_ds);
    /* 设置到通道5 */
    mr_dev_ioctl(dac_ds, MR_CTL_DAC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
    /* 设置通道使能 */
    ret = mr_dev_ioctl(dac_ds, MR_CTL_DAC_SET_CHANNEL_STATE, MR_MAKE_LOCAL(int, MR_DAC_STATE_ENABLE));
    if (ret < 0)
    {
        mr_printf("Channel5 enable failed: %s\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* 导出到自动初始化（APP级） */
MR_INIT_APP_EXPORT(dac_init);

/* 定义DAC数据最大值 */
#define DAC_DATA_MAX                    4095

int main(void)
{
    /* 自动初始化（dac_init函数将在此处自动调用） */
    mr_auto_init();
    
    while(1)
    {
        uint32_t data = 0;
        for (data = 0; data < DAC_DATA_MAX; data += 500)
        {
            int ret = mr_dev_write(dac_ds, &data, sizeof(data));
            if (ret != sizeof(data))
            {
                mr_printf("Write failed: %s\r\n", mr_strerror(ret));
            }
            mr_printf("DAC value: %d\r\n", data);
            mr_delay_ms(500);
        }
    }
}
```

DAC1通道5使能，间隔500毫秒输出一次DAC值并打印（输出值递增，步进500，直到达到最大值4095）。
