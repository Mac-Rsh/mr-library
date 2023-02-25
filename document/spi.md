`mr-library`
# **SPI设备**

**spi设备** 可以分为 2 种，SPI 总线和 SPI 设备。SPI 设备挂载在 SPI 总线上，每个 SPI 设备都可以获取 SPI 总线，然后开始传输数据。

#### **SPI 总线结构体**

```
struct mr_spi_bus
{
    struct mr_device parent;

    struct mr_spi_bus_ops *ops;
    volatile mr_uint8_t lock;
    struct mr_spi_device *owner;
};
```

#### **SPI 设备结构体**

```
struct mr_spi_device
{
    struct mr_device parent;

    struct mr_spi_config config;
    struct mr_spi_bus *bus;
};
```

通过结构体我们可以看到 SPI 总线包含一个 SPI 设备指针，用来获取当前占用总线的 SPI 设备。SPI 设备也拥有一个SPI 总线指针，用来指示设备挂载在哪个总线。

#### **SPI设备配置结构体**

```
struct mr_spi_config
{
    mr_uint32_t baud_rate;      /* 速率 */
    mr_uint8_t master_slave;    /* 主从模式 */
    mr_uint8_t mode;            /* SPI模式 */
    mr_uint8_t bit_order;       /* 高低位优先 */
    mr_uint8_t data_bits;       /* 数据位数 */
    mr_uint8_t cs_active;       /* cs有效电平 */
};
```

## **SPI设备**
根据设备框架结构，SPI 总线在使用前应已注册到内核中，而我们的 SPI 设备还暂未注册到内核，设备框架还无法使用，因此我们在使用前需先向内核注册一个 SPI设备。

#### **向内核注册 SPI 设备**

```
mr_err_t mr_spi_device_register(mr_spi_device_t spi_device,
                                const char *name,
                                mr_uint16_t support_flag,
                                void *cs_data);
```

|参数|描述|
|:--|:--|
|spi_device|SPI 设备句柄|
|name|SPI 设备名称|
|support_flag|SPI 设备支持标志|
|cs_data|SPI 设备 cs 引脚标号|
|**返回**|— —|
|MR_ERR_OK|关闭成功|
|Other|关闭失败，返回错误码|

注：在使用前需先创建一个 spi_device 实例，并且不能是局部变量也不允许被释放。SPI 设备被注册到内核后，会自动将其配置成默认配置，如下：

```
#define MR_SPI_CONFIG_DEFAULT                                    \
{                                                                \
  .baud_rate = 3000000,                /* 3M bits/s */           \
  .master_slave = MR_SPI_MASTER,       /* spi master */          \
  .mode = MR_SPI_MODE_0,               /* CPOL=0,CPHA=0 */       \
  .bit_order = MR_SPI_BIT_ORDER_MSB,   /* MSB first sent */      \
  .data_bits = MR_SPI_DATA_BITS_8,     /* 8 data_bits */         \
  .cs_active = MR_SPI_CS_ACTIVE_LOW,   /* CS low level active */ \
}
```

## **SPI 设备使用示例**

 1. 通过 `mr_spi_device_register()` 将 SPI 设备注册到内核。
 2. 通过 `mr_device_open()` 打开设备，获得设备句柄。
 3. 通过 `mr_device_ioctl()` 将 SPI 设备挂载到 SPI 总线上。
 4. 通过 `mr_device_ioctl()` 配置 SPI 设备参数。
 5. 通过 `mr_device_write()` 向 SPI 设备写入数据。
 6. 通过 `mr_device_read()` 从 SPI 设备 读取数据。
 7. 通过 `mr_device_ioctl()` 控制 SPI 设备全双工传输。
 8. 通过 `mr_device_close()` 关闭 SPI 设备。
 
```
/* 计算 GPIO C13 的标号 */
#define CS_NUM    ('C' - 'A') * 16 + 13;

/* 定义一个 SPI 设备 */
struct mr_spi_device spi_device;
mr_base_t spi_device_cs_num = CS_NUM;

/* 定义一个设备指针 */
mr_device_t device;

int main(void)
{
    /* 将 SPI 设备注册到内核 */
    mr_spi_device_register(&spi_device, "spi10", MR_OPEN_RDWR, &spi_device_cs_num);

    /* 使用可读可写的方式打开 spi10 设备 */
    device = mr_device_open("spi10", MR_OPEN_RDWR);
    
    /* 使用挂载命令，将 spi10 设备 挂载到 spi1 总线上 */
    mr_device_ioctl(device, MR_CMD_ATTACH, "spi1");
   
    /* 使用设置参数命令，修改 spi10 设备速率 */
    mr_uint32_t baud_rate = 10000000;
    mr_device_ioctl(device, MR_CMD_SET_PARAM | MR_SPI_BAUD_RATE, &baud_rate);
   
    /* 数据缓冲区 */
    mr_uint8_t buf[7] = {1,2,3,4,5,6,7};
   
    /* 向 spi10 设备写入 buf 数据 */
    mr_device_write(device, 0, buf, 7);
   
    /* 从 spi10 设备读取数据保存到 buf */
    mr_device_read(device, 0, buf, 7);
    
    /* 创建一个邮件，同时使用传输命令，控制 SPI 设备全双工传输 */
    struct mr_device_msg device_msg = {.send_buf = buf, .recv_buf = buf, .count = 7};
    mr_device_ioctl(device, MR_CMD_TRANSFER, &device_msg);
   
    /* 关闭 pin 设备 */
    mr_device_close(device);
}
```

## **SPI 设备控制命令补充说明**

SPI 设备支持以下命令：

|命令|描述|传入参数|
|:--|:--|:--|
|MR_CMD_ATTACH|将 SPI 设备挂载到 SPI 总线| SPI 总线名，如："spi1"|
|MR_CMD_CONFIG|将 SPI 设备配置修改为传入配置|传入 SPI 配置结构体|
|MR_CMD_SET_PARAM|修改 SPI 设备配置|参下|

当使用 `MR_CMD_SET_PARAM` 命令配置参数时，有以下可选命令：

|命令|描述|传入参数|
|:--|:--|:--|
|MR_SPI_BAUD_RATE|配置波特率|波特率，如：10000000|
|MR_SPI_MASTER|配置为主机|MR_NULL|
|MR_SPI_SLAVE|配置为从机|MR_NULL|
|MR_SPI_BIT_ORDER_LSB|配置为低位优先发送|MR_NULL|
|MR_SPI_BIT_ORDER_MSB|配置为高位优先发送|MR_NULL|
|MR_SPI_MODE_0|配置为 SPI 模式 0|MR_NULL|
|MR_SPI_MODE_1|配置为 SPI 模式 1|MR_NULL|
|MR_SPI_MODE_2|配置为 SPI 模式 2|MR_NULL|
|MR_SPI_MODE_3|配置为 SPI 模式 3|MR_NULL|
|MR_SPI_DATA_BITS_8|配置为 8bit 数据模式|MR_NULL|
|MR_SPI_DATA_BITS_16|配置为 16bit 数据模式|MR_NULL|
|MR_SPI_CS_ACTIVE_LOW|配置为 CS 低电平有效|MR_NULL|
|MR_SPI_CS_ACTIVE_HIGH|配置为 CS 高电平有效|MR_NULL|
