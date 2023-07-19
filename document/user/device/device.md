# 设备框架

**mr-library** 将硬件设备抽象成设备对象，并提供了统一的设备操作接口，方便应用程序与硬件设备进行交互。通过将驱动层与设备层解耦，提高了代码的可靠性和可维护性。

## 设备模型

设备对象定义如下:

```c
struct mr_device
{
    struct mr_object object;                                        /* 设备对象基类 */

    enum mr_device_type type;                                       /* 设备类型 */
    mr_uint16_t support_flag;                                       /* 设备支持的打开方式 */
    mr_uint16_t open_flag;                                          /* 设备打开方式 */
    mr_size_t ref_count;                                            /* 设备被引用次数 */
    void *data;                                                     /* 设备数据 */

    mr_err_t (*rx_cb)(mr_device_t device, void *args);              /* 设备接收回调函数 */
    mr_err_t (*tx_cb)(mr_device_t device, void *args);              /* 设备发送回调函数 */

    const struct mr_device_ops *ops;                                /* 设备操作方法 */
};
```

### 设备类型

设备框架支持多种类型的设备，每种设备都由一个枚举值来表示。设备类型定义如下：

```c
enum mr_device_type
{
    MR_DEVICE_TYPE_NONE,                                            /* 无类型设备 */
    MR_DEVICE_TYPE_PIN,                                             /* GPIO设备 */
    MR_DEVICE_TYPE_SPI_BUS,                                         /* SPI总线设备 */
    MR_DEVICE_TYPE_SPI,                                             /* SPI设备 */
    MR_DEVICE_TYPE_I2C_BUS,                                         /* I2C总线设备 */
    MR_DEVICE_TYPE_I2C,                                             /* I2C设备 */
    MR_DEVICE_TYPE_SERIAL,                                          /* UART设备*/
    MR_DEVICE_TYPE_ADC,                                             /* ADC设备 */
    MR_DEVICE_TYPE_DAC,                                             /* DAC设备 */
    MR_DEVICE_TYPE_PWM,                                             /* PWM设备 */
    MR_DEVICE_TYPE_TIMER,                                           /* TIMER设备 */
    MR_DEVICE_TYPE_FLASH,                                           /* FLASH设备 */
    /* ... */
};
```

### 设备打开方式

设备只能以支持的打开方式打开，定义如下：

```c
#define MR_OPEN_RDONLY                  0x1000                      /* 只读打开 */
#define MR_OPEN_WRONLY                  0x2000                      /* 只写打开 */
#define MR_OPEN_RDWR                    0x3000                      /* 可读可写 */
```

### 设备操作方法

每种设备类型都有对应的设备操作方法，包括打开设备、关闭设备、控制设备、读取设备和写入设备等操作。设备操作方法定义如下：

```c
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_ssize_t (*sda_read)(mr_device_t device, mr_pos_t pos, void *buffer, mr_size_t size);
    mr_ssize_t (*write)(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
};
```

| 接口    | 描述                                                          |
|:------|:------------------------------------------------------------|
| open  | 打开设备，同时完成设备配置。只有当设备首次被打开时，才会调用此方法来打开设备。                     |
| close | 关闭设备。只有当设备被所有用户关闭时（设备引用次数为 0），才会调用此方法来关闭设备。                 |
| ioctl | 控制设备。根据 cmd 命令控制设备                                          |
| sda_read  | 从设备读取数据。pos 是设备读取位置（不同设备所表示的意义不同，请查看设备详细手册），size 是设备读取字节大小。 |
| write | 向设备写入数据。pos 是设备写入位置（不同设备所表示的意义不同，请查看设备详细手册），size 是设备写入字节大小。 |

## 访问设备

应用程序通过设备操作接口来访问硬件设备，具体如下：

| 接口              | 描述          |
|:----------------|:------------|
| mr_device_add   | 将设备添加到内核容器中 |
| mr_device_find  | 在内核容器中查找设备  |
| mr_device_open  | 打开设备        |
| mr_device_close | 关闭设备        |
| mr_device_ioctl | 控制设备        |
| mr_device_read  | 从设备读取数据     |
| mr_device_write | 向设备写入数据     |

### 设备操作示例

下面是一个以 PIN 设备为例的设备操作示例：

```c
/* 寻找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为推挽输出模式 */
struct mr_pin_config pin_config = { 29, MR_PIN_MODE_OUTPUT };
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);

/* 设置B13为高电平 */
mr_uint8_t pin_level = 1;
mr_device_write(pin_device, 29, &pin_level, sizeof(pin_level));

/* 获取B13电平 */
mr_device_read(pin_device, 29, &pin_level, sizeof(pin_level));
```