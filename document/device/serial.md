# SERIAL设备

<!-- TOC -->
* [SERIAL设备](#serial设备)
  * [打开SERIAL设备](#打开serial设备)
  * [关闭SERIAL设备](#关闭serial设备)
  * [控制SERIAL设备](#控制serial设备)
    * [设置/获取SERIAL设备配置](#设置获取serial设备配置)
    * [设置/获取读/写缓冲区大小](#设置获取读写缓冲区大小)
    * [清空读/写缓冲区](#清空读写缓冲区)
    * [获取读/写缓冲区数据大小](#获取读写缓冲区数据大小)
    * [设置/获取读/写回调函数](#设置获取读写回调函数)
  * [读取SERIAL设备数据](#读取serial设备数据)
  * [写入SERIAL设备数据](#写入serial设备数据)
  * [使用示例](#使用示例)
<!-- TOC -->

## 打开SERIAL设备

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

- `name`：SERIAL设备名称一般为：`serialx`，例如：`serial1`、`serial2`、`serial3`。
- `oflags`：打开设备的标志，支持 `MR_OFLAG_RDONLY`、 `MR_OFLAG_WRONLY`、 `MR_OFLAG_RDWR`、 `MR_OFLAG_NONBLOCK`。

注：使用时应根据实际情况为不同的任务分别打开SERIAL设备，并使用适当的`oflags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭SERIAL设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

## 控制SERIAL设备

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
    - `MR_CTL_SERIAL_SET_CONFIG`：设置SERIAL设备配置。
    - `MR_CTL_SERIAL_SET_RD_BUFSZ`：设置读缓冲区大小。
    - `MR_CTL_SERIAL_SET_WR_BUFSZ`：设置写缓冲区大小。
    - `MR_CTL_SERIAL_CLR_RD_BUF`：清空读缓冲区。
    - `MR_CTL_SERIAL_CLR_WR_BUF`：清空写缓冲区。
    - `MR_CTL_SERIAL_SET_RD_CALL`：设置读回调函数。
    - `MR_CTL_SERIAL_SET_WR_CALL`：设置写回调函数。
    - `MR_CTL_SERIAL_GET_CONFIG`：获取SERIAL设备配置。
    - `MR_CTL_SERIAL_GET_RD_BUFSZ`：获取读缓冲区大小。
    - `MR_CTL_SERIAL_GET_WR_BUFSZ`：获取写缓冲区大小。
    - `MR_CTL_SERIAL_GET_RD_DATASZ`：获取读缓冲区数据大小。
    - `MR_CTL_SERIAL_GET_WR_DATASZ`：获取写缓冲区数据大小。
    - `MR_CTL_SERIAL_GET_RD_CALL`：获取读回调函数。
    - `MR_CTL_SERIAL_GET_WR_CALL`：获取写回调函数。

### 设置/获取SERIAL设备配置

SERIAL设备配置：

- `baud_rate`：波特率。
- `data_bits`：数据位数。
- `stop_bits`：停止位数。
- `parity`：校验位。
- `bit_order`：数据传输顺序。
- `invert`：极性反转。

```c
/* 设置默认配置 */
struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;

/* 设置SERIAL设备配置 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_SET_CONFIG, &config);
/* 获取SERIAL设备配置 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_CONFIG, &config);
```

注：如未手动配置，默认配置为：

- 波特率：`115200`
- 数据位数：`MR_SERIAL_DATA_BITS_8`
- 停止位数：`MR_SERIAL_STOP_BITS_1`
- 校验位：`MR_SERIAL_PARITY_NONE`
- 数据传输顺序：`MR_SERIAL_BIT_ORDER_LSB`
- 极性反转：`MR_SERIAL_POLARITY_NORMAL`

### 设置/获取读/写缓冲区大小

```c
size_t size = 256;

/* 设置读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_SET_RD_BUFSZ, &size);
/* 获取读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_RD_BUFSZ, &size);

/* 设置写缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_SET_WR_BUFSZ, &size);
/* 获取写缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_WR_BUFSZ, &size);
```

注：如未手动配置，将使用 `Kconfig`中配置的大小（默认为32Byte）。

### 清空读/写缓冲区

```c
mr_dev_ioctl(ds, MR_CTL_SERIAL_CLR_RD_BUF, MR_NULL);
mr_dev_ioctl(ds, MR_CTL_SERIAL_CLR_WR_BUF, MR_NULL);
```

### 获取读/写缓冲区数据大小

```c
size_t size = 0;

/* 获取读缓冲区数据大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_RD_DATASZ, &size);

/* 获取写缓冲区数据大小 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_WR_DATASZ, &size);
```

### 设置/获取读/写回调函数

```c
/* 定义回调函数 */
int call(int desc, void *args)
{
  /* 获取缓冲区数据大小 */
  ssize_t data_size = *(ssize_t *)args;
  
  /* 处理中断 */
  
  return MR_EOK;
}
int (*callback)(int, void *args);

/* 设置读回调函数 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_SET_RD_CALL, &call);
/* 获取读回调函数 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_RD_CALL, &callback);

/* 设置写回调函数 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_SET_WR_CALL, &call);
/* 获取写回调函数 */
mr_dev_ioctl(ds, MR_CTL_SERIAL_GET_WR_CALL, &callback);
```

## 读取SERIAL设备数据

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
char buf[128];
/* 读取SERIAL设备数据 */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));
/* 是否读取成功 */
if (size < 0)
{
    
}
```

注：当未设置读缓冲区时将使用轮询方式同步读取数据（无法保证完整接收数据）。当设置读缓冲区后将从读缓冲区读取指定数量的数据（返回实际读取的数据大小）。

## 写入SERIAL设备数据

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
char buf[] = {"hello world"};
/* 写入SERIAL设备数据 */
ssize_t size = mr_dev_write(ds, buf, sizeof(buf));
/* 是否写入成功 */
if (size < 0)
{
    
}
```

注：当未设置写缓冲区且未使用 `MR_OFLAG_NONBLOCK` 打开时将使用轮询方式同步写入数据。
当设置写缓冲区后会将数据写入写缓冲区（返回实际写入的数据大小），通过中断或DMA异步发送数据，发送完成后会触发写回调函数。
当有数据在异步发送时，写入锁将自动上锁，此时无法同步写入，直至异步发送完成。

## 使用示例

```c
#include "include/mr_lib.h"

/* 定义串口设备描述符 */
int serial_ds = -1;

int serial_init(void)
{
    /* 初始化串口 */
    serial_ds = mr_dev_open("serial1", MR_OFLAG_RDWR); 
    if (serial_ds < 0)
    {
        mr_printf("serial open failed: %s\r\n", mr_strerror(serial_ds));
        return serial_ds;
    }
    /* 设置串口配置 */
    struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;
    int ret = mr_dev_ioctl(serial_ds, MR_CTL_SERIAL_SET_CONFIG, &config);
    if (ret < 0)
    {
        mr_printf("serial set config failed: %s\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* 导出到自动初始化（APP级） */
MR_APP_EXPORT(serial_init);

int main(void)
{
    /* 自动初始化（serial_init函数将在此处自动调用） */
    mr_auto_init();

    while (1)
    {
        /* 回环测试 */
        char buf[64];
        ssize_t ret = mr_dev_read(serial_ds, buf, sizeof(buf));
        mr_dev_write(serial_ds, buf, ret);
    }
}
```

电脑连接串口1，在串口软件中进行回环测试，发送数据后会在串口软件中显示接收到的数据。