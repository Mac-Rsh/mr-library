# I2C设备使用指南

----------

## 概述

I2C（Inter-Integrated Circuit）是一种串行外设接口，用于连接和控制嵌入式系统中的外部设备。
它是一种双向、同步的通信协议，通过两根线（时钟线和数据线）实现设备之间的数据传输。I2C常用于连接和控制各种外围设备，如传感器、存储器、显示器等，具有简单、可靠和灵活的特点。

----------

## 准备

1. 调用I2C设备初始化函数（如果实现了自动初始化,则无需调用）。
2. 使能 `mrconfig.h` 头文件中I2C宏开关。

----------

## 添加I2C设备

```c
mr_err_t mr_i2c_device_add(mr_i2c_device_t i2c_device, const char *name, mr_uint32_t address);
```

| 参数         | 描述       |
|:-----------|:---------|
| i2c_device | I2C设备    |
| name       | I2C设备名   |
| address    | I2C设备地址  |
| **返回**     |          |
| MR_ERR_OK  | 添加设备成功   |
| 错误码        | 添加设备失败   |

## 控制I2C设备

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | I2C设备   |
| cmd       | 控制命令    |
| args      | 控制参数    |
| **返回**    |         |
| MR_ERR_OK | 控制设备成功  |
| 错误码       | 控制设备失败  |

I2C设备支持以下命令：

```c
MR_DEVICE_CTRL_SET_CONFIG                                           /* 设置参数 */
MR_DEVICE_CTRL_GET_CONFIG                                           /* 获取参数 */
MR_DEVICE_CTRL_CONNECT                                              /* 连接总线 */
```

### 配置I2C设备

```c
struct mr_i2c_config
{
    mr_uint32_t baud_rate;                                          /* 波特率 */
    mr_uint32_t host_slave: 1;                                      /* 主从模式 */
    mr_uint32_t addr_bits: 2;                                       /* 地址位数 */
    mr_uint32_t pos_bits: 6;                                        /* 位置位数 */
}
```

- 波特率：I2C的通信速率，表示每秒传输的比特数，常见的有18M、36M、72M等。
- 主从模式：主机模式还是从机模式。

```c
MR_I2C_HOST                                                         /* 主机模式 */
MR_I2C_SLAVE                                                        /* 从机模式 */
```

- 地址位数：发送地址位数。

```c
MR_I2C_ADDR_BITS_7                                                  /* 7位地址 */
MR_I2C_ADDR_BITS_10                                                 /* 10位地址 */
```

- 位置位数：发送位置时的位置的位数。

```c
MR_I2C_POS_BITS_8                                                   /* 8位位置 */
MR_I2C_POS_BITS_16                                                  /* 16位位置 */
MR_I2C_POS_BITS_32                                                  /* 32位位置 */
```

### I2C设备配置，连接、断开总线

I2C设备添加后并不能立即进行读写操作，其读写操作依赖总线，所以当I2C设备未连接总线或从总线断开后，其不支持以任何形式打开。如果原先处于打开状态，其也会被强制关闭。

使用示例：

```c
/* 定义I2C设备 */
#define I2C_DEVICE0_ADDR                0x33
#define I2C_DEVICE1_ADDR                0x44
struct mr_i2c_device i2c_device0, i2c_device1;

/* 添加I2C设备 */
mr_i2c_device_add(&i2c_device0, "i2c10", I2C_DEVICE0_ADDR);
mr_i2c_device_add(&i2c_device1, "i2c11", I2C_DEVICE1_ADDR);

/* 查找I2C设备 */
mr_device_t i2c0_device = mr_device_find("i2c10");
mr_device_t i2c1_device = mr_device_find("i2c11");

/* 设置默认参数 */
struct mr_i2c_config i2c_config = MR_I2C_CONFIG_DEFAULT;
mr_device_ioctl(i2c0_device, MR_DEVICE_CTRL_SET_CONFIG, &i2c_config);
mr_device_ioctl(i2c1_device, MR_DEVICE_CTRL_SET_CONFIG, &i2c_config);

/* 连接总线 */
mr_device_ioctl(i2c0_device, MR_DEVICE_CTRL_CONNECT, "i2c1");
mr_device_ioctl(i2c1_device, MR_DEVICE_CTRL_CONNECT, "i2c1");

/* 断开总线 */
mr_device_ioctl(i2c0_device, MR_DEVICE_CTRL_CONNECT, MR_NULL);
mr_device_ioctl(i2c1_device, MR_DEVICE_CTRL_CONNECT, MR_NULL);
```

----------

## I2C设备读取数据

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数          | 描述     |
|:------------|:-------|
| device      | I2C设备  |
| pos         | 读取位置   |
| buffer      | 读取数据   |
| size        | 读取数据大小 |
| **返回**      |        |
| 实际读取的数据大小   |        |

- 读取位置：需要读取的（寄存器）地址，如读取0x23地址数据，则pos = 0x23，有效范围：0-2147483647。地址长度根据设置的pos_bits，如不使用时请赋值：-1。

使用示例：

```c
/* 查找I2C1设备（在此之前请先添加设备并连接总线） */
mr_device_t i2c_device = mr_device_find("i2c10");

/* 以可读可写的方式打开I2C设备 */
mr_device_open(i2c_device, MR_DEVICE_OFLAG_RDWR);

/* 读取数据 */
mr_uint8_t buffer[10];
mr_device_read(i2c_device, -1, buffer, sizeof(buffer));

/* 读取0x23地址数据*/
mr_device_read(i2c_device, 0x23, buffer, sizeof(buffer));
```

----------

## I2C设备写入数据

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数          | 描述     |
|:------------|:-------|
| device      | I2C设备  |
| pos         | 写入位置   |
| buffer      | 写入数据   |
| size        | 写入数据大小 |
| **返回**      |        |
| 实际写入的数据大小   |        |

- 写入位置：需要写入的（寄存器）地址，如写入0x23地址数据，则pos = 0x23，有效范围：0-2147483647。地址长度根据设置的pos_bits，如不使用时请赋值：-1。

使用示例：

```c
/* 查找I2C1设备（在此之前请先添加设备并连接总线） */
mr_device_t i2c_device = mr_device_find("i2c10");

/* 以可读可写的方式打开I2C设备 */
mr_device_open(i2c_device, MR_DEVICE_OFLAG_RDWR);

/* 写入数据 */
char buffer[] = "hello";
mr_device_write(i2c_device, -1, buffer, sizeof(buffer) - 1);

/* 向0x23地址写入数据*/
mr_device_write(i2c_device, 0x23, buffer, sizeof(buffer) - 1);
```