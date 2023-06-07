# 软件I2C文档

## I2C原型

```c
/* I2C总线 */
struct i2c_bus
{
    struct i2c_device *owner;
    uint8_t lock;

    void *data;

    const struct i2c_bus_ops *ops;
};
```

- owner:总线拥有设备。
- lock：总线锁。
- data:总线数据。
- ops:操作方法。

```c
/* I2C设备 */
struct i2c_device
{
    uint8_t addr;

struct i2c_bus *bus;
};
```

- addr:设备地址。

## I2C总线方法原型（需要适配的接口）

```c
struct i2c_bus_ops
{
    void (*clk_ctrl)(i2c_bus_t i2c_bus, uint8_t state);
    void (*sda_ctrl)(i2c_bus_t i2c_bus, uint8_t state);
    uint8_t (*read)(i2c_bus_t i2c_bus);
};
```

- clk_ctrl：控制时钟线。
- sda_ctrl：控制数据线。
- read：读取数据线。

### 初始化I2C总线

```c
void i2c_bus_init(i2c_bus_t bus, const struct i2c_bus_ops *ops, void *data);
```

| 参数   | 描述     |
|:-----|:-------|
| bus  | 总线句柄   |
| ops  | 总线方法   |
| data | 总线数据   |

### 初始化I2C设备

```c
void i2c_device_init(i2c_device_t device, uint8_t addr, i2c_bus_t bus);
```

| 参数     | 描述     |
|:-------|:-------|
| device | 设备句柄   |
| addr   | 设备地址   |
| bus    | 总线句柄   |

### 向I2C设备寄存器写入数据

```c
void i2c_device_write_reg(i2c_device_t device, uint8_t reg, uint8_t data);
```

| 参数     | 描述     |
|:-------|:-------|
| device | 设备句柄   |
| reg    | 寄存器地址  |
| data   | 要写入的数据 |

### 从I2C设备寄存器读取数据

```c
void i2c_device_write_reg(i2c_device_t device, uint8_t reg, uint8_t data);
```

| 参数      | 描述      |
|:--------|:--------|
| device  | 设备句柄    |
| reg     | 寄存器地址   |
| **返回**  |         |
| data    | 读取到的数据  |

### 向I2C设备传输数据（先写入再读取）

```c
void i2c_device_transfer(i2c_device_t device,
                         const uint8_t *write_buffer,
                         size_t write_size,
                         uint8_t *read_buffer,
                         size_t read_size);
```

| 参数              | 描述     |
|:----------------|:-------|
| device          | 设备句柄   |
| write_buffer    | 要写入的数据 |
| write_size      | 写入数据大小 |
| read_buffer     | 要读取的数据 |
| read_size       | 读取数据大小 |

## 使用示例：

```c

/* 定义I2C总线和设备 */
struct i2c_bus i2c_bus;
struct i2c_device i2c_device1;
struct i2c_device i2c_device2;

/* 适配I2C总线接口 */
void clk_ctrl(i2c_bus_t i2c_bus, uint8_t state)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, state);
}
void sda_ctrl(i2c_bus_t i2c_bus, uint8_t state)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, state);
}

uint8_t read(i2c_bus_t i2c_bus)
{
    return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);
}

/* 定义I2C总线方法 */
struct i2c_bus_ops i2c_ops =
{
    clk_ctrl,
    sda_ctrl,
    read
};

int main(void)
{
    /* 初始化I2C总线 */
    i2c_bus_init(&i2c_bus, &i2c_ops, NULL);
    
    /* 初始化I2C设备 */
    i2c_device_init(&i2c_device1, 0x34, &i2c_bus);
    i2c_device_init(&i2c_device2, 0x57, &i2c_bus);

    /* I2C设备写入数据 */
    i2c_device_write_reg(&i2c_device1, 0x12, 0xAA);
    i2c_device_write_reg(&i2c_device2, 0x12, 0xAA);
    
    /* I2C设备读取数据 */
    uint8_t data1 = i2c_device_read_reg(&i2c_device1, 0x12);
    uint8_t data2 = i2c_device_read_reg(&i2c_device2, 0x12);
    
    uint8 write_buffer[2] = {0x10, 0x11};
    uint8 read_buffer[2] = {0, 0};

    /* 向I2C设备传输数据 */
    i2c_device_transfer(&i2c_device1, write_buffer, sizeof(write_buffer), read_buffer, sizeof(read_buffer));
    i2c_device_transfer(&i2c_device2, write_buffer, sizeof(write_buffer), read_buffer, sizeof(read_buffer));
}
```

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。

 ----------

# 贡献代码

如果您对 **mr-library** 项目感兴趣，欢迎参与开发并成为代码贡献者。欢迎加入讨论群 **199915649(QQ)** 分享您的观点。