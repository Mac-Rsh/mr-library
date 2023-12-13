# SPI设备

<!-- TOC -->
* [SPI设备](#spi设备)
  * [注册SPI设备](#注册spi设备)
  * [打开SPI设备](#打开spi设备)
  * [关闭SPI设备](#关闭spi设备)
  * [控制SPI设备](#控制spi设备)
    * [设置/获取SPI设备配置](#设置获取spi设备配置)
    * [设置/获取寄存器值](#设置获取寄存器值)
    * [设置/获取读缓冲区大小](#设置获取读缓冲区大小)
    * [清空读缓冲区](#清空读缓冲区)
    * [获取读缓冲区数据大小](#获取读缓冲区数据大小)
    * [设置/获取读回调函数](#设置获取读回调函数)
    * [全双工传输](#全双工传输)
  * [读取SPI设备数据](#读取spi设备数据)
  * [写入SPI设备数据](#写入spi设备数据)
  * [使用示例：](#使用示例)
<!-- TOC -->

注：SPI-CS控制需使能PIN设备。

## 注册SPI设备

```c
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *name, int cs_pin, int cs_active);
```

| 参数        | 描述         |
|-----------|------------|
| spi_dev   | SPI设备结构体指针 |
| name      | 设备名称       |
| cs_pin    | 片选引脚编号     |
| cs_active | 片选使能状态     |
| **返回值**   |            |
| `=0`      | 注册成功       |
| `<0`      | 错误码        |

- `name`：SPI设备要绑定到指定的SPI总线，命名需要加上总线名称，例如：`spi1/dev-name`。
- `cs_pin`：片选引脚编号（参考PIN设备文档）。
- `cs_active`：片选使能状态：
    - `MR_SPI_CS_ACTIVE_LOW`：低电平使能。
    - `MR_SPI_CS_ACTIVE_HIGH`：高电平使能。
    - `MR_SPI_CS_ACTIVE_NONE`：不使能（忽略`cs_pin`参数）。

## 打开SPI设备

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

- `name`：SPI设备绑定在SPI总线上，需加上总线名称，例如：`spix/dev-name`，`spi1/dev-name`。
- `oflags`：打开设备的标志，支持 `MR_OFLAG_RDONLY`、 `MR_OFLAG_WRONLY`、 `MR_OFLAG_RDWR`。

注：使用时应根据实际情况为不同的任务分别打开SPI设备，并使用适当的`oflags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭SPI设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

## 控制SPI设备

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
    - `MR_CTL_SPI_SET_CONFIG`：设置SPI设备配置。
    - `MR_CTL_SPI_SET_REG`：设置寄存器值。
    - `MR_CTL_SPI_SET_RD_BUFSZ`：设置读缓冲区大小。
    - `MR_CTL_SPI_CLR_RD_BUF`：清空读缓冲区。
    - `MR_CTL_SPI_SET_RD_CALL`：设置读回调函数。
    - `MR_CTL_SPI_TRANSFER`：全双工传输。
    - `MR_CTL_SPI_GET_CONFIG`：获取SPI设备配置。
    - `MR_CTL_SPI_GET_REG`：获取寄存器值。
    - `MR_CTL_SPI_GET_RD_BUFSZ`：获取读缓冲区大小。
    - `MR_CTL_SPI_GET_RD_DATASZ`：获取读缓冲区数据大小。
    - `MR_CTL_SPI_GET_RD_CALL`：获取读回调函数。

### 设置/获取SPI设备配置

SPI设备配置：

- `baud_rate`：波特率。
- `host_slave`：主机/从机模式。
- `mode`：模式。
- `data_bits`：数据位数。
- `bit_order`：数据传输顺序。
- `reg_bits`：寄存器位数。

```c
/* 设置默认配置 */
struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;

/* 设置SPI设备配置 */
mr_dev_ioctl(ds, MR_CTL_SPI_SET_CONFIG, &config);
/* 获取SPI设备配置 */
mr_dev_ioctl(ds, MR_CTL_SPI_GET_CONFIG, &config);
```

注：

- 如未手动配置，默认配置为：
    - 波特率：`3000000`
    - 主机/从机模式：`MR_SPI_HOST`
    - 模式：`MR_SPI_MODE_0`
    - 数据位数：`MR_SPI_DATA_BITS_8`
    - 数据传输顺序：`MR_SPI_BIT_ORDER_MSB`
    - 寄存器位数：`MR_SPI_REG_BITS_8`
- 当SPI总线上有SPI设备被配置成从机模式后，其将持续占用SPI总线，此时其余SPI设备无法进行读写等操作，直至从机模式SPI设备被重新配置为主机模式。

### 设置/获取寄存器值

寄存器值将在读取和写入数据前被优先写入（范围：`0` ~ `INT32_MAX`）。

```c
/* 设置寄存器值 */
mr_dev_ioctl(ds, MR_CTL_SPI_SET_REG, mr_make_local(int, 0x12));

/* 获取寄存器值 */
uint8_t reg;
mr_dev_ioctl(ds, MR_CTL_SPI_GET_REG, &reg);
```

注：

- 寄存器值仅在主机模式下生效。
- 设为负数时不生效。
- 寄存器值不计入读写大小。

### 设置/获取读缓冲区大小

```c
size_t size = 256;

/* 设置读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SPI_SET_RD_BUFSZ, &size);
/* 获取读缓冲区大小 */
mr_dev_ioctl(ds, MR_CTL_SPI_GET_RD_BUFSZ, &size);
```

注：如未手动配置，将使用 `Kconfig`中配置的大小（默认为32Byte）。只有在从机模式下才使用读缓冲区。

### 清空读缓冲区

```c
mr_dev_ioctl(ds, MR_CTL_SPI_CLR_RD_BUF, MR_NULL);
```

### 获取读缓冲区数据大小

```c
size_t size = 0;

/* 获取读缓冲区数据大小 */
mr_dev_ioctl(ds, MR_CTL_SPI_GET_RD_DATASZ, &size);
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
mr_dev_ioctl(ds, MR_CTL_SPI_SET_RD_CALL, &call);
/* 获取读回调函数 */
mr_dev_ioctl(ds, MR_CTL_SPI_GET_RD_CALL, &callback);
```

### 全双工传输

```c
/* 定义传输结构体 */
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
struct mr_spi_transfer transfer =
{
  .rd_buf = buf,
  .wr_buf = buf,
  .size = sizeof(buf),
};

/* 全双工传输 */
ssize_t size = mr_dev_ioctl(ds, MR_CTL_SPI_TRANSFER, &transfer);
/* 是否传输成功 */
if (size < 0)
{

}
```

## 读取SPI设备数据

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
/* 读取SPI设备数据 */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));
/* 是否读取成功 */
if (size < 0)
{
    
}
```

注：

- 主机模式下，将使用轮询方式同步读取数据。从机模式下，未设置读缓冲区将使用轮询方式同步读取数据，设置读缓冲区后将从读缓冲区读取指定数量的数据（返回实际读取的数据大小）。
- 当寄存器参数不为负数时，将在读取操作前插入寄存器值的写入操作。

## 写入SPI设备数据

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
/* 写入SPI设备数据 */
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

/* 定义SPI设备 */
struct mr_spi_dev host_dev;
struct mr_spi_dev slave_dev;

/* 定义SPI设备描述符 */
int host_ds = -1;
int slave_ds = -1;

int spi_init(void)
{
    int ret = MR_EOK;
    
    /* 注册SPI-HOST设备 */
    ret = mr_spi_dev_register(&host_dev, "spi1/host", 0, MR_SPI_CS_ACTIVE_LOW);
    if (ret < 0)
    {
        mr_printf("host spi device register failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    
    /* 注册SPI-SLAVE设备 */
    ret = mr_spi_dev_register(&slave_dev, "spi2/slave", 1, MR_SPI_CS_ACTIVE_LOW);
    if (ret < 0)
    {
        mr_printf("slave spi device register failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    
    /* 打开SPI-HOST设备 */
    host_ds = mr_dev_open("spi1/host", MR_OFLAG_RDWR);
    if (host_ds < 0)
    {
        mr_printf("host spi device open failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    /* 设置寄存器值 */
    mr_dev_ioctl(host_ds, MR_CTL_SPI_SET_REG, mr_make_local(int, 0x12));
    
    /* 打开SPI-SLAVE设备 */
    slave_ds = mr_dev_open("spi2/slave", MR_OFLAG_RDWR);
    if (slave_ds < 0)
    {
        mr_printf("slave spi device open failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    /* 设置为从机模式 */
    struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;
    config.host_slave = MR_SPI_SLAVE;
    ret = mr_dev_ioctl(slave_ds, MR_CTL_SPI_SET_CONFIG, &config);
    if (ret < 0)
    {
        mr_printf("slave spi device set config failed: %d\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* 导出到自动初始化（APP级） */
MR_APP_EXPORT(spi_init);

int main(void)
{
    /* 自动初始化（spi_init函数将在此处自动调用） */
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
            mr_printf("spi test success\r\n");
            return 0;
        }
    }

    while (1)
    {
        
    }
}
```

将SPI1与SPI2相接，引脚0与1相接（参考PIN设备文档），进行收发测试，SPI1作为主机，SPI2作为从机。SPI2将接收到的数据与发送的数据进行比较。
由于设置了寄存器值，故在写入数据前，先接收到了寄存器值，然后才是写入的数据。
