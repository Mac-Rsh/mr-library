`mr-library`
# **设备框架**

**device** 设备框架对驱动设备进行了抽象。应用层通过使用设备框架对底层设备进行交互。底层驱动仅需适配对应接口，即可实现对应功能，且底层驱动变化对应用层无影响，降低了应用程序移植成本，提高了程序可靠性。

#### **设备结构体**

```
struct mr_device
{
    /* Device object */
    struct mr_object object;                                    /* 内核对象 */

    /* Device properties */
    enum mr_device_type type;                                   /* 设备类型 */
    mr_uint16_t support_flag;                                   /* 设备支持标志 */
    mr_uint16_t open_flag;                                      /* 设备打开标志 */
    mr_uint8_t ref_count;                                       /* 设备被引用次数 */

    /* Device operations and callbacks */
    struct mr_device_ops *ops;                                  /* 设备操作方法 */
    mr_err_t (*rx_callback)(mr_device_t device, void *args);    /* 设备接收回调函数 */
    mr_err_t (*tx_callback)(mr_device_t device, void *args);    /* 设备发送回调函数 */

    /* Hardware private data */
    void *private_data;                                         /* 设备私有数据 */
};
```

#### **设备类型**

```
enum mr_device_type
{
    MR_DEVICE_TYPE_PIN,
    MR_DEVICE_TYPE_SPI_BUS,
    MR_DEVICE_TYPE_SPI,
    MR_DEVICE_TYPE_I2C,
    MR_DEVICE_TYPE_UART,
    MR_DEVICE_TYPE_ADC,
    MR_DEVICE_TYPE_DAC,

    MR_DEVICE_TYPE_PWM,
    MR_DEVICE_TYPE_TIMER,
    MR_DEVICE_TYPE_WDT,
    MR_DEVICE_TYPE_FLASH,
    MR_DEVICE_TYPE_SDRAM,
    /* ... */
};
```

#### **设备支持标志**

#### **设备打开标志**

#### **设备操作方法**
```
struct mr_device_ops
{
    mr_err_t (*open)(mr_device_t device);
    mr_err_t (*close)(mr_device_t device);
    mr_err_t (*ioctl)(mr_device_t device, int cmd, void *args);
    mr_size_t (*read)(mr_device_t device, mr_off_t pos, void *buf, mr_size_t count);
    mr_size_t (*write)(mr_device_t device, mr_off_t pos, const void *buf, mr_size_t count);
};
```

`open` 初始化设备，使设备能够完成后续操作。该函数在设备整个生命周期内只会被调用一次。
`close` 关闭设备，解除设备的资源占用。该函数在设备整个生命周期内只会被调用一次。
`ioctl` 控制设备，响应设备的各种控制命令。
`read` 从设备读取数据。
`write` 向设备写入数据。

## **设备对外暴露接口**

#### **将设备注册到内核**

```
mr_err_t mr_device_register(mr_device_t device, const char *name);
```

|参数|描述|
|:--|:--|
|device|设备句柄|
|name|设备名称|
|**返回**|— —|
|对象句柄|寻找成功|
|MR_NULL|寻找失败|

注：设备名称不可以重复，建议使用 xxx1、xxx2 方式为设备命名。如果设备是挂载在某个设备下的，建议使用 xxx10、xxx11 方式为设备命名（xxx10 即为 xxx1 设备下的 0号设备）。

#### **打开设备**

```
mr_device_t mr_device_open(char *name, mr_uint16_t flags);
```

|参数|描述|
|:--|:--|
|name|设备名称|
|flags|打开设备的方式|
|**返回**|— —|
|设备句柄|打开成功|
|MR_NULL|打开失败|

#### **关闭设备**

```
mr_err_t mr_device_close(mr_device_t device);
```

|参数|描述|
|:--|:--|
|device|设备句柄|
|**返回**|— —|
|MR_ERR_OK|关闭成功|
|Other|关闭失败，返回错误码|

#### **控制设备**

```
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

|参数|描述|
|:--|:--|
|device|设备句柄|
|cmd|控制命令|
|args|控制参数|
|**返回**|— —|
|MR_ERR_OK|关闭成功|
|Other|关闭失败，返回错误码|

#### **从设备读取数据**

```
mr_size_t mr_device_read(mr_device_t device, mr_off_t pos, void *buf, mr_size_t count);
```

|参数|描述|
|:--|:--|
|device|设备句柄|
|pos|偏移值|
|buf|内存缓冲区，读取到的数据将保存到缓冲区中|
|count|读取数据数量|
|**返回**|— —|
|实际读取数据数量|读取成功|
|0|读取失败|

注：pos 偏移量请参考实际设备确定，例如 pin 设备的 pos 即为 相对 0 号 io 的偏移值。如不需要偏移请传入 0 。

#### **向设备写入数据**

```
mr_size_t mr_device_write(mr_device_t device, mr_off_t pos, void *buf, mr_size_t count);
```

|参数|描述|
|:--|:--|
|device|设备句柄|
|pos|偏移值|
|buf|内存缓冲区，缓冲区数据将被写入到设备|
|count|写入数据数量|
|**返回**|— —|
|实际写入数据数量|写入成功|
|0|写入失败|

注：pos 偏移量请参考实际设备确定，例如 pin 设备的 pos 即为 相对 0 号 io 的偏移值。如不需要偏移请传入 0 。

## **设备框架使用示例**

下面的代码为访问 PIN 设备示例。

 1. 通过 `mr_device_open()` 打开设备，获得设备句柄。
 2. 通过 `mr_device_ioctl()` 控制 GPIO 初始化。
 3. 通过 `mr_device_write()` 向 GPIO 写入电平数据。
 4. 通过 `mr_device_read()` 从 GPIO 读取电平数据。
 5. 通过 `mr_device_close()` 关闭 PIN 设备。

```
/* 定义高低电平（内核中已存在）*/
#define MR_HIGH     1
#define MR_LOW      0

/* 计算 GPIO C13 的标号 */
#define GPIO_NUM    ('C' - 'A') * 16 + 13;

/* 定义一个设备指针 */
mr_device_t device;

int main(void)
{
    /* 使用可读可写的方式打开 pin 设备 */
    device = mr_device_open("pin", MR_OPEN_RDWR);

    /* 使用设置参数命令，将 GPIO C13 配置成推挽输出模式 */
    mr_uint8_t pin_num = GPIO_NUM;
    mr_device_ioctl(device, MR_CMD_SET_PARAM | MR_PIN_OUTPUT, &pin_num);
    
    /* 向 GPIO C13 写入 高电平 */
    mr_uint8_t value = MR_HIGH;
    mr_device_write(device, pin_num, &value, 1);
    
    /* 使用设置参数命令，将 GPIO C13 配置成输入模式 */
    mr_device_ioctl(device, MR_CMD_SET_PARAM | MR_PIN_INPUT, &pin_num);
    
    /* 从 GPIO C13 读取电平 */
    mr_device_read(device, pin_num, &value, 1);
    
    /* 关闭 pin 设备 */
    mr_device_close(device);
}
```