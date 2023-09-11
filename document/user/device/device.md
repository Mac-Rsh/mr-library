# 设备框架

**mr-library** 将硬件设备抽象成设备对象，并提供了统一的设备操作接口，方便应用程序与硬件设备进行交互。通过将驱动层与设备层解耦，提高了代码的可靠性和可维护性。

## 操作流程图

![流程图](https://gitee.com/MacRsh/mr-library/raw/develop/document/resource/device_process.jpg)

## 设备模型

设备对象定义如下：

```c
struct mr_device
{
    struct mr_object object;                                        /* 设备对象基类 */

    mr_uint8_t type;                                                /* 设备类型 */
    mr_uint8_t sflags;                                              /* 设备支持的打开方式 */
    mr_uint8_t oflags;                                              /* 设备打开方式 */
    mr_uint8_t reserved;                                            /* 保留 */
    mr_size_t ref_count;                                            /* 设备被引用次数 */
    mr_err_t (*rx_cb)(mr_device_t device, void *args);              /* 设备接收回调函数 */
    mr_err_t (*tx_cb)(mr_device_t device, void *args);              /* 设备发送回调函数 */

    const struct mr_device_ops *ops;                                /* 设备操作方法 */
    void *data;                                                     /* 设备数据 */
};
```

### 设备类型

设备框架支持多种类型的设备，每种设备都由一个枚举值来表示。设备类型定义如下：

```c
enum mr_device_type
{
    Mr_Device_Type_None,                                            /* 无类型设备 */
    Mr_Device_Type_Pin,                                             /* GPIO设备 */
    Mr_Device_Type_SPIBUS,,                                         /* SPI总线设备 */
    Mr_Device_Type_SPI,                                             /* SPI设备 */
    Mr_Device_Type_I2CBUS,,                                         /* I2C总线设备 */
    Mr_Device_Type_I2C,                                             /* I2C设备 */
    Mr_Device_Type_Serial,                                          /* UART设备*/
    Mr_Device_Type_ADC,                                             /* ADC设备 */
    Mr_Device_Type_DAC,                                             /* DAC设备 */
    Mr_Device_Type_PWM,                                             /* PWM设备 */
    Mr_Device_Type_Timer,                                           /* TIMER设备 */
    Mr_Device_Type_Flash,                                           /* FLASH设备 */
    /* ... */
};
```

### 设备打开方式

设备只能以支持的打开方式打开，定义如下：

```c
MR_DEVICE_OFLAG_RDONLY                  0x10                        /* 只读打开 */
MR_DEVICE_OFLAG_WRONLY                  0x20                        /* 只写打开 */
MR_DEVICE_OFLAG_RDWR                    0x30                        /* 可读可写 */
MR_DEVICE_OFLAG_NONBLOCKING             0x40                        /* 非阻塞 */
```

非阻塞打开方式可与其他打开方式相或使用，例如：

```c
(MR_DEVICE_OFLAG_RDWR | MR_DEVICE_OFLAG_NONBLOCKING)                /* 非阻塞可读可写 */
```

### 设备操作方法

每种设备类型都有对应的设备操作方法，包括打开设备、关闭设备、控制设备、读取设备和写入设备等操作。设备操作方法定义如下：

```c
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*read)(mr_device_t device, mr_off_t pos, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
};
```

| 接口    | 描述                                                         |
|:------|:-----------------------------------------------------------|
| open  | 打开设备，同时完成设备配置。只有当设备首次被打开时，才会调用此方法来打开设备                     |
| close | 关闭设备。只有当设备被所有用户关闭时（设备引用次数为 0），才会调用此方法来关闭设备                 |
| ioctl | 控制设备。根据 cmd 命令控制设备                                         |
| read  | 从设备读取数据。pos 是设备读取位置（不同设备所表示的意义不同，请查看设备详细手册），size 是设备读取字节大小 |
| write | 向设备写入数据。pos 是设备写入位置（不同设备所表示的意义不同，请查看设备详细手册），size 是设备写入字节大小 |

## 操作设备

应用程序通过设备操作接口来操作硬件设备，具体如下：

| 接口              | 描述      |
|:----------------|:--------|
| mr_device_add   | 添加设备    |
| mr_device_find  | 查找设备    |
| mr_device_open  | 打开设备    |
| mr_device_close | 关闭设备    |
| mr_device_ioctl | 控制设备    |
| mr_device_read  | 从设备读取数据 |
| mr_device_write | 向设备写入数据 |

### 设备操作示例：

下面是一个以 SPI 设备为例的设备操作示例：

```c
/* 定义SPI设备 */
#define SPI_DEVICE0_CS_PIN              10
#define SPI_DEVICE1_CS_PIN              20
struct mr_spi_device spi_device0, spi_device1;

/* 添加SPI设备 */
mr_spi_device_add(&spi_device0, "spi10", SPI_DEVICE0_CS_PIN);
mr_spi_device_add(&spi_device1, "spi11", SPI_DEVICE1_CS_PIN);

/* 查找SPI设备 */
mr_device_t spi0_device = mr_device_find("spi10");
mr_device_t spi1_device = mr_device_find("spi11");

/* 挂载总线 */
mr_device_ioctl(spi0_device, MR_DEVICE_CTRL_CONNECT, "spi1");
mr_device_ioctl(spi1_device, MR_DEVICE_CTRL_CONNECT, "spi1");

/* 以可读可写的方式打开 */
mr_device_open(spi0_device, MR_DEVICE_OFLAG_RDWR);
mr_device_open(spi1_device, MR_DEVICE_OFLAG_RDWR);

/* 发送数据 */
char buffer0[] = "hello";
char buffer1[] = "world";
mr_device_write(spi0_device, -1, buffer0, sizeof(buffer0) - 1);
mr_device_write(spi1_device, -1, buffer1, sizeof(buffer1) - 1);

/* 读取数据 */
mr_device_read(spi0_device, -1, buffer0, sizeof(buffer0) - 1);
mr_device_read(spi1_device, -1, buffer1, sizeof(buffer1) - 1);

/* 关闭设备 */
mr_device_close(spi0_device);
mr_device_close(spi1_device);
```