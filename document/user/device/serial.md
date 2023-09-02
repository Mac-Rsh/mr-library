# SERIAL设备使用指南

----------

## 概述

UART（Universal Asynchronous Receiver/Transmitter）即通用异步收发传输器,是一种串行通信协议,常用于串口通信。

----------

## 调用关系

![调用关系](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/serial_device.png)

----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用SERIAL设备初始化函数（如果实现了自动初始化,则无需调用）。
3. 使能 `mrconfig.h` 头文件中SERIAL宏开关。

----------

## 查找SERIAL设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数       | 描述        |
|:---------|:----------|
| name     | SERIAL设备名 |
| **返回**   |           |
| SERIAL句柄 | 查找设备成功    |
| MR_NULL  | 查找设备失败    |

----------

## 打开SERIAL设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
```

| 参数        | 描述         |
|:----------|:-----------|
| device    | SERIAL设备句柄 |
| flags     | 打开方式       |
| **返回**    |            |
| MR_ERR_OK | 打开设备成功     |
| 错误码       | 打开设备失败     |

SERIAL设备支持以下打开方式：

```c
MR_OPEN_RDONLY                                                      /* 只读 */
MR_OPEN_WRONLY                                                      /* 只写 */
MR_OPEN_RDWR                                                        /* 可读可写 */
MR_OPEN_NONBLOCKING                                                 /* 非阻塞 */
```

当打开方式|MR_OPEN_NONBLOCKING，对串口设备写入数据时将检测发送缓冲区空间是否为0，如果非0，数据将被压入发送缓冲区后通过中断发送。

| 缓冲区 | 打开方式 | 接收方式 | 发送方式 |
|:----|:-----|:-----|:-----|
| 不使用 | 阻塞   | 轮询   | 轮询   |
| 不使用 | 非阻塞  | 轮询   | 轮询   |
| 使用  | 阻塞   | 中断   | 轮询   |
| 使用  | 非阻塞  | 中断   | 中断   |

----------

## 控制SERIAL设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述         |
|:----------|:-----------|
| device    | SERIAL设备句柄 |
| cmd       | 控制命令       |
| args      | 控制参数       |
| **返回**    |            |
| MR_ERR_OK | 控制设备成功     |
| 错误码       | 控制设备失败     |

SERIAL设备支持以下命令：

```c
MR_CTRL_SET_CONFIG                                                  /* 设置参数 */
MR_CTRL_GET_CONFIG                                                  /* 获取参数 */
MR_CTRL_SET_RX_CB                                                   /* 设置接收（接收中断）回调函数 */
MR_CTRL_SET_TX_CB                                                   /* 设置发送（发送完成中断）回调函数 */     
MR_CTRL_SET_RX_BUFSZ                                                /* 设置接收缓冲区大小 */
MR_CTRL_SET_TX_BUFSZ                                                /* 设置发送缓冲区大小 */
```

### 配置SERIAL设备

```c
struct mr_serial_config
{
    mr_uint32_t baud_rate;                                          /* 波特率 */
    mr_uint8_t data_bits: 2;                                        /* 数据位 */
    mr_uint8_t stop_bits: 2;                                        /* 停止位 */
    mr_uint8_t parity: 2;                                           /* 奇偶校验 */
    mr_uint8_t bit_order: 1;                                        /* 高低位 */
    mr_uint8_t invert: 1;                                           /* 模式 */
};
```

- 波特率：串口的通信速率，表示每秒传输的比特数，常见的有2400bps、4800bps、9600bps等。
- 数据位：表示每个字符所占的比特数。

```c
MR_SERIAL_DATA_BITS_8                                               /* 数据位8 */
MR_SERIAL_DATA_BITS_9                                               /* 数据位9 */
```

- 停止位：表示每字节数据后面的停止信号位数。

```c
MR_SERIAL_STOP_BITS_1                                               /* 停止位1 */
MR_SERIAL_STOP_BITS_1_5                                             /* 停止位1.5 */
MR_SERIAL_STOP_BITS_2                                               /* 停止位2 */
```

- 奇偶校验：串口通信中的错误检测方式。

```c
MR_SERIAL_PARITY_NONE                                               /* 不校验 */
MR_SERIAL_PARITY_EVEN                                               /* 偶校验 */
MR_SERIAL_PARITY_ODD                                                /* 奇校验 */
```

- 高低位：数据高位在前还是低位在前。

```c
MR_SERIAL_BIT_ORDER_LSB                                             /* 低位在前 */
MR_SERIAL_BIT_ORDER_MSB                                             /* 高位在前 */
```

- 模式：串口通信的电平逻辑。

```c
MR_SERIAL_NRZ_NORMAL                                                /* 不翻转 */
MR_SERIAL_NRZ_INVERTED                                              /* 电平翻转 */
```

使用示例：

```c
/* 查找SERIAL1设备 */    
mr_device_t serial_device = mr_device_find("uart1");

/* 设置默认参数 */
struct mr_serial_config serial_config = MR_SERIAL_CONFIG_DEFAULT;
mr_device_ioctl(serial_device, MR_CTRL_SET_CONFIG, &serial_config);

/* 获取参数 */
mr_device_ioctl(serial_device, MR_CTRL_GET_CONFIG, &serial_config);
```

注：如未手动修改SERIAL设备参数，则默认使用默认参数。

### 设置SERIAL设备接收（发送完成）回调函数

- 回调函数：device为触发回调设备，args传入缓冲区数据长度。

使用示例：

```c
/* 定义回调函数 */
mr_err_t serial_device_rx_cb(mr_device_t device, void *args)
{
    mr_size_t length = *(mr_size_t *)args;                          /* 获取接收缓冲区数据长度 */
    
    /* Do something */
}

mr_err_t serial_device_tx_cb(mr_device_t device, void *args)
{
    mr_size_t length = *(mr_size_t *)args;                          /* 获取发送缓冲区数据长度 */
    
    /* Do something */
}

/* 查找SERIAL1设备 */    
mr_device_t serial_device = mr_device_find("uart1");

/* 设置接收（发送完成）回调函数 */
mr_device_ioctl(serial_device, MR_CTRL_SET_RX_CB, serial_device_rx_cb);
mr_device_ioctl(serial_device, MR_CTRL_SET_TX_CB, serial_device_tx_cb);
```

### 设置SERIAL设备接收（发送）缓冲区大小

使用示例：

```c
/* 查找SERIAL1设备 */    
mr_device_t serial_device = mr_device_find("uart1");

/* 设置接收（发送）缓冲区大小 */
mr_size_t bufsz = 64;
mr_device_ioctl(serial_device, MR_CTRL_SET_RX_BUFSZ, &bufsz);
mr_device_ioctl(serial_device, MR_CTRL_SET_TX_BUFSZ, &bufsz);
```

----------

## SERIAL设备读取数据

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述           |
|:----------|:-------------|
| device    | SERIAL设备句柄   |
| pos       | 读取位置         |
| buffer    | 读取数据         |
| size      | 读取数据大小       |
| **返回**    |              |
| 实际读取的数据大小 |              |

使用示例：

```c
/* 查找SERIAL1设备 */    
mr_device_t serial_device = mr_device_find("uart1");

/* 以可读可写方式打开 */
mr_device_open(serial_device, MR_OPEN_RDWR);

/* 设置默认参数 */
struct mr_serial_config serial_config = MR_SERIAL_CONFIG_DEFAULT;
mr_device_ioctl(serial_device, MR_CTRL_SET_CONFIG, &serial_config);

/* 读取数据 */
char buffer[5] = {0};
mr_device_write(serial_device, 0, buffer, sizeof(buffer));
```

----------

## SERIAL设备写入数据

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述         |
|:----------|:-----------|
| device    | SERIAL设备句柄 |
| pos       | 写入位置       |
| buffer    | 写入数据       |
| size      | 写入数据大小     |
| **返回**    |            |
| 实际写入的数据大小 |            |

使用示例：

```c
/* 查找SERIAL1设备 */    
mr_device_t serial_device = mr_device_find("uart1");

/* 以非阻塞可读可写方式打开 */
mr_device_open(serial_device, MR_OPEN_RDWR | MR_OPEN_NONBLOCKING);

/* 设置默认参数 */
struct mr_serial_config serial_config = MR_SERIAL_CONFIG_DEFAULT;
mr_device_ioctl(serial_device, MR_CTRL_SET_CONFIG, &serial_config);

/* 写入数据 */
char buffer[] = "hello";
mr_device_write(serial_device, 0, buffer, sizeof(buffer) - 1);
```