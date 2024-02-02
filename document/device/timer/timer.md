# TIMER设备

[English](timer_EN.md)

<!-- TOC -->
* [TIMER设备](#timer设备)
  * [打开TIMER设备](#打开timer设备)
  * [关闭TIMER设备](#关闭timer设备)
  * [控制TIMER设备](#控制timer设备)
    * [设置/获取TIMER设备配置](#设置获取timer设备配置)
    * [设置/获取超时回调函数](#设置获取超时回调函数)
  * [读取TIMER设备运行时间](#读取timer设备运行时间)
  * [写入TIMER设备定时时间](#写入timer设备定时时间)
  * [使用示例](#使用示例)
<!-- TOC -->

## 打开TIMER设备

```c
int mr_dev_open(const char *path, int flags);
```

| 参数      | 描述      |
|---------|---------|
| path    | 设备路径    |
| flags   | 打开设备的标志 |
| **返回值** |         |
| `>=0`   | 设备描述符   |
| `<0`    | 错误码     |

- `path`：TIMER设备路径一般为：`timerx`，例如：`timer1`、`timer2`、`timer3`。
- `flags`：打开设备的标志，支持 `MR_O_RDONLY`、 `MR_O_WRONLY`、 `MR_O_RDWR`。

注：使用时应根据实际情况为不同的任务分别打开TIMER设备，并使用适当的`flags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭TIMER设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

## 控制TIMER设备

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
    - `MR_IOC_TIMER_SET_MODE`：设置TIMER设备模式。
    - `MR_IOC_TIMER_SET_TIMEOUT_CALL`：设置超时回调函数。
    - `MR_IOC_TIMER_GET_MODE`：获取TIMER设备模式。
    - `MR_IOC_TIMER_GET_TIMEOUT_CALL`：获取超时回调函数。

### 设置/获取TIMER设备配置

TIMER设备配置：

- `mode`：周期或单次模式。

```c
/* 定义TIMER设备模式 */
#define TIMER_MODE                      MR_TIMER_MODE_ONESHOT

/* 设置TIMER设备模式 */
mr_dev_ioctl(ds, MR_IOC_TIMER_SET_MODE, MR_MAKE_LOCAL(int, TIMER_MODE));

/* 获取TIMER设备模式 */
int mode;
mr_dev_ioctl(ds, MR_IOC_TIMER_GET_MODE, &mode);
```

不依赖TIMER接口：

```c
/* 定义TIMER设备模式 */
#define TIMER_MODE                      1

/* 设置TIMER设备模式 */
mr_dev_ioctl(ds, MR_IOC_SCFG, MR_MAKE_LOCAL(int, TIMER_MODE));

/* 获取TIMER设备模式 */
int mode;
mr_dev_ioctl(ds, MR_IOC_GCFG, &mode);
```

注：如未手动配置，默认配置为：

- 模式：`MR_TIMER_MODE_PERIOD`

### 设置/获取超时回调函数

```c
/* 定义回调函数 */
void fn(int desc, void *args)
{
    /* 处理中断 */
}
void (*callback)(int desc, void *args);
    
/* 设置超时回调函数 */
mr_dev_ioctl(ds, MR_IOC_TIMER_SET_TIMEOUT_CALL, &fn);
/* 获取超时回调函数 */
mr_dev_ioctl(ds, MR_IOC_TIMER_GET_TIMEOUT_CALL, &callback);
```

不依赖TIMER接口：

```c
/* 定义回调函数 */
void fn(int desc, void *args)
{
    /* 处理中断 */
}
void (*callback)(int desc, void *args);
    
/* 设置超时回调函数 */
mr_dev_ioctl(ds, MR_IOC_SRCB, &fn);
/* 获取超时回调函数 */
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback);
```

## 读取TIMER设备运行时间

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 读取数据缓冲区 |
| count   | 读取数据大小  |
| **返回值** |         |
| `>=0`   | 读取数据大小  |
| `<0`    | 错误码     |

```c
uint32_t time;
/* 读取TIMER设备数据 */
ssize_t size = mr_dev_read(ds, &time, sizeof(time));
/* 是否读取成功 */
if (size < 0)
{
    
}
```

注：

- 时间单位为微妙。单次读取最小单位为`uint32_t`，即4个字节。
- 运行时间指定时器启动后运行的时间，超时后重置。

## 写入TIMER设备定时时间

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 写入数据缓冲区 |
| count   | 写入数据大小  |
| **返回值** |         |
| `>=0`   | 写入数据大小  |
| `<0`    | 错误码     |

注：

- 时间单位为微妙。单次写入最小单位为`uint32_t`，即4个字节。
- 当有一次写入多个单位数据时，仅实际生效最后一个有效数据（如果最后一个有效数据为`0`将关闭定时器）。

## 使用示例

```c
#include "include/mr_lib.h"

void timeout_call(int desc, void *args)
{
    mr_printf("Timeout\r\n");
}

/* 定时时间 */
#define TIMEOUT                         500000

void timer_init(void)
{
    /* 初始化TIMER */
    int timer_ds = mr_dev_open("timer1", MR_O_RDWR);
    if (timer_ds < 0)
    {
        mr_printf("timer open failed: %s\r\n", mr_strerror(timer_ds));
        return;
    }
    /* 打印TIMER描述符 */
    mr_printf("TIMER desc: %d\r\n", timer_ds);
    /* 设置超时回调函数 */
    mr_dev_ioctl(timer_ds, MR_IOC_TIMER_SET_TIMEOUT_CALL, timeout_call);
    /* 设置定时时间 */
    uint32_t timeout = TIMEOUT;
    int ret = mr_dev_write(timer_ds, &timeout, sizeof(timeout));
    if(ret < 0)
    {
        mr_printf("timer write failed: %s\r\n", mr_strerror(ret));
    }
}
/* 导出到自动初始化（APP级） */
MR_INIT_APP_EXPORT(timer_init);

int main(void)
{
    /* 自动初始化（timer_init函数将在此处自动调用） */
    mr_auto_init();

    while(1)
    {
        
    }
}
```

定时器以500ms为周期，每500ms打印一次`Timeout`。
