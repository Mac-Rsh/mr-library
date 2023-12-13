# I2C设备

<!-- TOC -->
* [I2C设备](#i2c设备)
  * [注册I2C设备](#注册i2c设备)
  * [打开I2C设备](#打开i2c设备)
  * [关闭I2C设备](#关闭i2c设备)
  * [控制I2C设备](#控制i2c设备)
    * [设置/获取I2C设备配置](#设置获取i2c设备配置)
    * [设置/获取寄存器值](#设置获取寄存器值)
    * [设置/获取读缓冲区大小](#设置获取读缓冲区大小)
    * [清空读缓冲区](#清空读缓冲区)
    * [获取读缓冲区数据大小](#获取读缓冲区数据大小)
    * [设置/获取读回调函数](#设置获取读回调函数)
  * [读取I2C设备数据](#读取i2c设备数据)
  * [写入I2C设备数据](#写入i2c设备数据)
  * [使用示例：](#使用示例)
  * [软件I2C](#软件i2c)
    * [注册软件I2C总线](#注册软件i2c总线)
<!-- TOC -->

## 注册I2C设备

```c
int mr_i2c_dev_register(struct mr_i2c_dev *i2c_dev, const char *name, int addr, int addr_bits);
```

| 参数        | 描述         |
|-----------|------------|
| i2c_dev   | I2C设备结构体指针 |
| name      | 设备名称       |
| addr      | 设备地址       |
| addr_bits | 设备地址位数     |
| **返回值**   |            |
| `=0`      | 注册成功       |
| `<0`      | 错误码        |

- `name`：I2C设备要绑定到指定的I2C总线，命名需要加上总线名称，例如：`i2c1/dev-name`。
- `addr`：设备地址（最低位为读写位，请传入向左移位后的地址）。
- `addr_bits`：设备地址位数：
    - `MR_I2C_ADDR_BITS_7`：7位地址。
    - `MR_I2C_ADDR_BITS_10`：10位地址。

## 打开I2C设备

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

- `name`：I2C设备绑定在I2C总线上，需加上总线名称，例如：`i2cx/dev-name`，`i2c1/dev-name`。
- `oflags`：打开设备的标志，支持 `MR_OFLAG_RDONLY`、 `MR_OFLAG_WRONLY`、 `MR_OFLAG_RDWR`。

注：使用时应根据实际情况为不同的任务分别打开I2C设备，并使用适当的`oflags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭I2C设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

## 控制I2C设备

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
    - `MR_CTL_I2C_SET_CONFIG`： 设置I2C设备配置。
    - `MR_CTL_I2C_SET_REG`： 设置寄存器值。
    - `MR_CTL_I2C_SET_RD_BUFSZ`： 设置读缓冲区大小。
    - `MR_CTL_I2C_CLR_RD_BUF`： 清空读缓冲区。
    - `MR_CTL_I2C_SET_RD_CALL`：设置读回调函数。
    - `MR_CTL_I2C_GET_CONFIG`： 获取I2C设备配置。
    - `MR_CTL_I2C_GET_REG`： 获取寄存器值。
    - `MR_CTL_I2C_GET_RD_BUFSZ`： 获取读缓冲区大小。
    - `MR_CTL_I2C_GET_RD_DATASZ`： 获取读缓冲区数据大小。
    - `MR_CTL_I2C_GET_RD_CALL`：获取读回调函数。

### 设置/获取I2C设备配置

I2C设备配置：

- `baud_rate`：波特率。
- `host_slave`：主机/从机模式。
- `reg_bits`：寄存器位数。

```c
/* 设置默认配置 */
struct mr_i2c_config config = MR_I2C_CONFIG_DEFAULT;

/* 设置I2C设备配置 */
mr_dev_ioctl(ds, MR_CTL_I2C_SET_CONFIG, &config);
/* 获取I2C设备配置 */
mr_dev_ioctl(ds, MR_CTL_I2C_GET_CONFIG, &config);
```

注：

- 如未手动配置，默认配置为：
    - 波特率：`100000`
    - 主机/从机模式：`MR_I2C_HOST`
    - 寄存器位数：`MR_I2C_REG_BITS_8`
- 当I2C总线上有I2C设备被配置成从机模式后，其将持续占用I2C总线，此时其余I2C设备无法进行读写等操作，直至从机模式I2C设备被重新配置为主机模式。

### 设置/获取寄存器值

寄存器值将在读取和写入数据前被优先写入（范围：`0` ~ `INT32_MAX`）。

```c
/* 设置寄存器值 */
mr_dev_ioctl(ds, MR_CTL_I2C_SET_REG, mr_make_local(int, 0x12));

/* 获取寄存器值 */
uint8_t reg;
mr_dev_ioctl(ds, MR_CTL_I2C_GET_REG, &reg);
```

注：

- 寄存器值仅在主机模式下生效。
- 设为负数时不生效。
- 寄存器值不计入读写大小。

### 设置/获取读缓冲区大小

```c
size_t size = 256;

/* 设置读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_I2C_SET_RD_BUFSZ, &size);
/* 获取读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_I2C_GET_RD_BUFSZ, &size);
```

注：如未手动配置，将使用 `Kconfig`中配置的大小（默认为32Byte）。只有在从机模式下才使用读缓冲区。

### 清空读缓冲区

```c
mr_dev_ioctl(ds, MR_CTL_I2C_CLR_RD_BUF, MR_NULL);
```

### 获取读缓冲区数据大小

```c
size_t size = 0;

/* 获取读缓冲区数据大小 */
mr_dev_ioctl(ds, MR_CTL_I2C_GET_RD_DATASZ, &size);
```

### 设置/获取读回调函数

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
mr_dev_ioctl(ds, MR_CTL_I2C_SET_RD_CALL, &call);
/* 获取读回调函数 */
mr_dev_ioctl(ds, MR_CTL_I2C_GET_RD_CALL, &callback);
```

## 读取I2C设备数据

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
uint8_t buf[128];
/* 读取I2C设备数据 */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));
/* 是否读取成功 */
if (size < 0)
{
    
}
```

注：

- 主机模式下，将使用轮询方式同步读取数据。从机模式下，未设置读缓冲区将使用轮询方式同步读取数据，设置读缓冲区后将从读缓冲区读取指定数量的数据（返回实际读取的数据大小）。
- 当寄存器参数不为负数时，将在读取操作前插入寄存器值的写入操作。

## 写入I2C设备数据

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
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
/* 写入I2C设备数据 */
ssize_t size = mr_dev_write(ds, buf, sizeof(buf));
/* 是否写入成功 */
if (size < 0)
{
    
}
```

注：当寄存器参数不为负数时，将在写入操作前插入寄存器值的写入操作。

## 使用示例：

```c
#include "include/mr_lib.h"

/* 定义I2C设备 */
struct mr_i2c_dev host_dev;
struct mr_i2c_dev slave_dev;

/* 定义I2C设备描述符 */
int host_ds = -1;
int slave_ds = -1;

int i2c_init(void)
{
    int ret = MR_EOK;
    
    /* 注册I2C-HOST设备 */
    ret = mr_i2c_dev_register(&host_dev, "i2c1/host", 0x62, MR_I2C_ADDR_BITS_7);
    if (ret < 0)
    {
        mr_printf("host i2c device register failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    
    /* 注册I2C-SLAVE设备 */
    ret = mr_i2c_dev_register(&slave_dev, "i2c2/slave", 0x62, MR_I2C_ADDR_BITS_7);
    if (ret < 0)
    {
        mr_printf("slave i2c device register failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    
    /* 打开I2C-HOST设备 */
    host_ds = mr_dev_open("i2c1/host", MR_OFLAG_RDWR);
    if (host_ds < 0)
    {
        mr_printf("host i2c device open failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    /* 设置寄存器值 */
    mr_dev_ioctl(host_ds, MR_CTL_I2C_SET_REG, mr_make_local(int, 0x12));
    
    /* 打开I2C-SLAVE设备 */
    slave_ds = mr_dev_open("i2c2/slave", MR_OFLAG_RDWR);
    if (slave_ds < 0)
    {
        mr_printf("slave i2c device open failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    /* 设置为从机模式 */
    struct mr_i2c_config config = MR_I2C_CONFIG_DEFAULT;
    config.host_slave = MR_I2C_SLAVE;
    ret = mr_dev_ioctl(slave_ds, MR_CTL_I2C_SET_CONFIG, &config);
    if (ret < 0)
    {
        mr_printf("slave i2c device set config failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* 导出到自动初始化（APP级） */
MR_APP_EXPORT(i2c_init);

int main(void)
{
    /* 自动初始化（i2c_init函数将在此处自动调用） */
    mr_auto_init();
    
    /* 发送测试数据 */
    uint8_t wr_buf[] = {0x01, 0x02, 0x03, 0x04};
    mr_dev_write(host_ds, wr_buf, sizeof(wr_buf));
    
    /* 接收测试数据 */
    uint8_t rd_buf[128];
    ssize_t ret = mr_dev_read(slave_ds, rd_buf, sizeof(rd_buf));
    
    /* 比较寄存器值 */
    if (rd_buf[0] == 0x12)
    {
        /* 比较数据 */
        if (memcmp(wr_buf, (rd_buf + 1), sizeof(wr_buf)) == 0)
        {
            mr_printf("i2c test success\r\n");
            return 0;
        }
    }

    while (1)
    {
        
    }
}
```

将I2C1与I2C2相接，进行收发测试。I2C1作为主机，I2C2作为从机。I2C2将接收到的数据与发送的数据进行比较。
由于设置了寄存器值，故在写入数据前，先接收到了寄存器值，然后才是写入的数据。

## 软件I2C

注：软件I2C需使能PIN设备。

### 注册软件I2C总线

```c
int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus, const char *name, int scl_pin, int sda_pin);
```

| 参数           | 描述           |
|--------------|--------------|
| soft_i2c_bus | 软件I2C总线结构体指针 |
| name         | 总线名称         |
| scl_pin      | SCL引脚编号      |
| sda_pin      | SDA引脚编号      |
| **返回值**      |              |
| `=0`         | 注册成功         |
| `<0`         | 错误码          |

```c
/* 定义软件I2C总线SCL、SDA引脚编号 */
#define SCL_PIN_NUMBER                  0   
#define SDA_PIN_NUMBER                  1

/* 定义软件I2C总线 */
struct mr_soft_i2c_bus soft_i2c_bus;

/* 注册软件I2C总线 */
mr_soft_i2c_bus_register(&soft_i2c_bus, "i2c1", SCL_PIN_NUMBER, SDA_PIN_NUMBER);
```

注册完成后，软件I2C总线将模拟成硬件I2C。

注：软件I2C总线仅支持主机模式。
