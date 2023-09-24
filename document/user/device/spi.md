# SPI设备使用指南

----------

## 概述

SPI（Serial Peripheral Interface）是一种串行外设接口，用于连接和控制嵌入式系统中的外部设备。
是一种全双工、同步的通信协议，通过时钟和数据线实现高速数据传输。SPI常用于存储器、传感器、显示器等外围设备的通信，具有简单、可靠和高速传输的特点。

----------

## 准备

1. 调用SPI设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中SPI宏开关。

----------

## 调用关系

![调用关系](https://gitee.com/MacRsh/mr-library/raw/master/document/resource/spi_device.png)

----------

## 准备

1. 调用SPI设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中SPI宏开关。

----------

## 添加SPI设备

```c
mr_err_t mr_spi_device_add(mr_spi_device_t spi_device, const char *name, mr_off_t cs_number);
```

| 参数         | 描述          |
|:-----------|:------------|
| spi_device | SPI设备       |
| name       | SPI设备名      |
| cs_number  | SPI设备片选引脚编号 |
| **返回**     |             |
| MR_ERR_OK  | 添加设备成功      |
| 错误码        | 添加设备失败      |

## 控制SPI设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | SPI设备   |
| cmd       | 控制命令    |
| args      | 控制参数    |
| **返回**    |         |
| MR_ERR_OK | 控制设备成功  |
| 错误码       | 控制设备失败  |

SPI设备支持以下命令：

```c
MR_DEVICE_CTRL_SET_CONFIG                                           /* 设置参数 */
MR_DEVICE_CTRL_GET_CONFIG                                           /* 获取参数 */
MR_DEVICE_CTRL_CONNECT                                              /* 连接总线 */
MR_DEVICE_CTRL_SET_RX_CB                                            /* 设置接收（接收中断）回调函数 */
MR_DEVICE_CTRL_SET_RX_BUFSZ                                         /* 设置接收缓冲区大小 */
MR_DEVICE_CTRL_SPI_TRANSFER                                         /* 同步传输 */
```

### 配置SPI设备

```c
struct mr_spi_config
{
    mr_uint32_t baud_rate;                                          /* 波特率 */
    mr_uint32_t host_slave: 1;                                      /* 主从模式 */
    mr_uint32_t mode: 2;                                            /* 极性、相位模式 */
    mr_uint32_t data_bits: 6;                                       /* 数据位数 */
    mr_uint32_t bit_order: 1;                                       /* 高低位 */
    mr_uint32_t cs_active: 2;                                       /* 片选激活电平 */
    mr_uint32_t pos_bits: 6;                                        /* 位置位数 */
}
```

- 波特率：SPI的通信速率，表示每秒传输的比特数，常见的有18M、36M、72M等。
- 主从模式：主机模式还是从机模式。

```c
MR_SPI_HOST                                                         /* 主机模式 */
MR_SPI_SLAVE                                                        /* 从机模式 */
```

- 极性、相位模式：时钟的极性和相位。

```c
MR_SPI_MODE_0                                                       /* CPOL = 0, CPHA = 0 */
MR_SPI_MODE_1                                                       /* CPOL = 0, CPHA = 1 */
MR_SPI_MODE_2                                                       /* CPOL = 1, CPHA = 0 */
MR_SPI_MODE_3                                                       /* CPOL = 1, CPHA = 1 */
```

- 数据位：表示每次能发送的位数。

```c
MR_SPI_DATA_BITS_8                                                  /* 8位数据 */
MR_SPI_DATA_BITS_16                                                 /* 16位数据 */
MR_SPI_DATA_BITS_32                                                 /* 32位数据 */
```

- 高低位：数据高位在前还是低位在前。

```c
MR_SPI_BIT_ORDER_MSB                                                /* 高位在前 */
MR_SPI_BIT_ORDER_LSB                                                /* 低位在前 */
```

- 片选激活电平：片选引脚有效的电平。

```c
MR_SPI_CS_ACTIVE_LOW                                                /* 低电平有效 */
MR_SPI_CS_ACTIVE_HIGH                                               /* 高电平有效 */
MR_SPI_CS_ACTIVE_HARDWARE                                           /* 硬件控制 */
```

- 位置位数：发送位置时的位置的位数。

```c
MR_SPI_POS_BITS_8                                                   /* 8位位置 */
MR_SPI_POS_BITS_16                                                  /* 16位位置 */
MR_SPI_POS_BITS_32                                                  /* 32位位置 */
```

### SPI设备配置，连接、断开总线

SPI设备添加后并不能立即进行读写操作，其读写操作依赖总线。SPI从机设备挂载总线后其余设备将无法进行读写操作，直至从机设备断开连接。

使用示例：

```c
/* 定义SPI设备 */
#define SPI_DEVICE0_CS_NUMBER           10
#define SPI_DEVICE1_CS_NUMBER           20
struct mr_spi_device spi_device0, spi_device1;

/* 添加SPI设备 */
mr_spi_device_add(&spi_device0, "spi10", SPI_DEVICE0_CS_NUMBER);
mr_spi_device_add(&spi_device1, "spi11", SPI_DEVICE1_CS_NUMBER);

/* 查找SPI设备 */
mr_device_t spi0_device = mr_device_find("spi10");
mr_device_t spi1_device = mr_device_find("spi11");

/* 设置默认参数 */
struct mr_spi_config spi_config = MR_SPI_CONFIG_DEFAULT;
mr_device_ioctl(spi0_device, MR_DEVICE_CTRL_SET_CONFIG, &spi_config);
mr_device_ioctl(spi1_device, MR_DEVICE_CTRL_SET_CONFIG, &spi_config);

/* 连接总线 */
mr_device_ioctl(spi0_device, MR_DEVICE_CTRL_CONNECT, "spi1");
mr_device_ioctl(spi1_device, MR_DEVICE_CTRL_CONNECT, "spi1");

/* 断开总线 */
mr_device_ioctl(spi0_device, MR_DEVICE_CTRL_CONNECT, MR_NULL);
mr_device_ioctl(spi1_device, MR_DEVICE_CTRL_CONNECT, MR_NULL);
```

### 设置SPI设备从机模式接收回调函数

- 回调函数：device为触发回调设备，args传入缓冲区数据长度。

使用示例：

```c
/* 定义回调函数 */
mr_err_t spi_device_rx_cb(mr_device_t device, void *args)
{
    mr_size_t length = *(mr_size_t *)args;                          /* 获取接收缓冲区数据长度 */
    
    /* Do something */
}

/* 查找SPI1设备 */    
mr_device_t spi_device = mr_device_find("spi10");

/* 设置接收回调函数 */
mr_device_ioctl(spi_device, MR_DEVICE_CTRL_SET_RX_CB, spi_device_rx_cb);
```

## 设置SPI设备接收缓冲区大小

使用示例：

```c
/* 查找SPI1设备 */
mr_device_t spi_device = mr_device_find("spi10");

/* 设置接收缓冲区大小 */
mr_size_t bufsz = 64;
mr_device_ioctl(spi_device, MR_DEVICE_CTRL_SET_RX_BUFSZ, &bufsz);
```

### SPI设备同步传输

```c
struct mr_spi_transfer
{
    void *write_buffer;                                             /* 写入数据 */
    void *read_buffer;                                              /* 读取数据 */

    mr_size_t size;                                                 /* 传输大小 */
};
```

使用示例：

```c
/* 查找SPI1设备（在此之前请先添加设备并连接总线） */
mr_device_t spi_device = mr_device_find("spi10");

/* 以可读可写的方式打开SPI设备 */
mr_device_open(spi_device, MR_DEVICE_OFLAG_RDWR);

/* 传输数据 */
mr_uint8_t buffer_w[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
mr_uint8_t buffer_r[10];
struct mr_spi_transfer spi_transfer;
spi_transfer.write_buffer = buffer_w;
spi_transfer.read_buffer = buffer_r;
spi_transfer.size = sizeof(buffer_w);
mr_device_ioctl(spi_device, MR_DEVICE_CTRL_SPI_TRANSFER, &spi_transfer);
```

----------

## SPI设备读取数据

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数          | 描述     |
|:------------|:-------|
| device      | SPI设备  |
| pos         | 读取位置   |
| buffer      | 读取数据   |
| size        | 读取数据大小 |
| **返回**      |        |
| 实际读取的数据大小   |        |

- 读取位置：需要读取的（寄存器）地址，如读取0x23地址数据，则pos = 0x23，有效范围：0-2147483647。地址长度根据设置的pos_bits，如不使用时请赋值：-1。

使用示例：

```c
/* 查找SPI1设备（在此之前请先添加设备并连接总线） */
mr_device_t spi_device = mr_device_find("spi10");

/* 以可读可写的方式打开SPI设备 */
mr_device_open(spi_device, MR_DEVICE_OFLAG_RDWR);

/* 读取数据 */
mr_uint8_t buffer[10];
mr_device_read(spi_device, -1, buffer, sizeof(buffer));

/* 读取0x23地址数据*/
mr_device_read(spi_device, 0x23, buffer, sizeof(buffer));
```

----------

## SPI设备写入数据

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数          | 描述     |
|:------------|:-------|
| device      | SPI设备  |
| pos         | 写入位置   |
| buffer      | 写入数据   |
| size        | 写入数据大小 |
| **返回**      |        |
| 实际写入的数据大小   |        |

- 写入位置：需要写入的（寄存器）地址，如写入0x23地址数据，则pos = 0x23，有效范围：0-2147483647。地址长度根据设置的pos_bits，如不使用时请赋值：-1。

使用示例：

```c
/* 查找SPI1设备（在此之前请先添加设备并连接总线） */
mr_device_t spi_device = mr_device_find("spi10");

/* 以可读可写的方式打开SPI设备 */
mr_device_open(spi_device, MR_DEVICE_OFLAG_RDWR);

/* 写入数据 */
char buffer[] = "hello";
mr_device_write(spi_device, -1, buffer, sizeof(buffer) - 1);

/* 向0x23地址写入数据*/
mr_device_write(spi_device, 0x23, buffer, sizeof(buffer) - 1);
```