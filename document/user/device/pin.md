# PIN使用指南

 ----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用pin设备注册函数（如果实现了自动初始化，则无需调用）。

 ----------

## 寻找PIN设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数      | 描述     |
|:--------|:-------|
| name    | PIN设备名 |
| **返回**  |        |
| PIN句柄   | 寻找设备成功 |
| MR_NULL | 寻找设备失败 |

## 打开PIN设备

```c
mr_err_t mr_device_open(mr_device_t device, mr_uint16_t flags);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| flags     | 打开方式    |
| **返回**    |         |
| MR_ERR_OK | 打开设备成功  |
| 错误码       | 打开设备失败  |

PIN设备支持以下打开方式:

```c
MR_OPEN_RDONLY                          /* 只读 */
MR_OPEN_WRONLY                          /* 只写 */
MR_OPEN_RDWR                            /* 可读可写 */
```

## 配置PIN模式

```c
mr_err_t mr_device_ioctl(mr_device_t device, int cmd, void *args);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| cmd       | 控制命令    |
| args      | 控制参数    |
| **返回**    |         |
| MR_ERR_OK | 控制设备成功  |
| MR_NULL   | 寻找设备失败  |

通过`ioctl`函数将控制参数配置到PIN设备。PIN控制参数原型如下:

```c
struct mr_pin_config
{
    mr_uint16_t number;                 /* 编号 */
    mr_uint8_t mode;                    /* 模式 */
};
```

**编号**:`mr-library`对GPIO进行编码，计算公式(端口号,引脚号) `->` ('A' - 端口号) * 16 + 引脚号，例如 B13 = 1 * 16 + 13 =
29。

**模式**:支持对GPIO进行普通和中断模式配置。

```c
/* GPIO 模式 */
#define
MR_PIN_MODE_NONE                /* 关闭IO */
#define
MR_PIN_MODE_OUTPUT              /* 推挽输出 */
#define
MR_PIN_MODE_OUTPUT_OD           /* 开漏输出 */
#define
MR_PIN_MODE_INPUT               /* 浮空输入 */
#define
MR_PIN_MODE_INPUT_DOWN          /* 下拉输入 */
#define
MR_PIN_MODE_INPUT_UP            /* 上拉输入 */

/* 中断模式 */
#define
MR_PIN_MODE_RISING              /* 上升沿触发 */
#define
MR_PIN_MODE_FALLING             /* 下降沿触发 */
#define
MR_PIN_MODE_EDGE                /* 边缘触发 */
#define
MR_PIN_MODE_LOW                 /* 低电平触发 */
#define
MR_PIN_MODE_HIGH                /* 高电平触发 */
```

使用示例如下所示:

```c
/* 寻找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为推挽输出模式 */
struct mr_pin_config pin_config = { 29, MR_PIN_MODE_OUTPUT };
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);
```

## 设置引脚电平

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| pos       | 写入位置    |
| buffer    | 写入数据    |
| size      | 写入数据大小  |
| **返回**    |         |
| 实际写入的数据大小 |         |

**写入位置**:指需要写入数据的位置，如写入B13，则pos = 29。

使用示例如下所示:

```c
/* 设置B13为高电平 */
mr_uint8_t pin_level = 1;
mr_device_write(pin_device, 29, &pin_level, sizeof(pin_level));
```

## 获取引脚电平

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_off_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| pos       | 读取位置    |
| buffer    | 读取数据    |
| size      | 读取数据大小  |
| **返回**    |         |
| 实际读取的数据大小 |         |

**读取位置**:指需要读取数据的位置，如读取B13，则pos = 29。

使用示例如下所示:

```c
/* 读取B13电平 */
mr_uint8_t pin_level = 0;
mr_device_read(pin_device, 29, &pin_level, sizeof(pin_level));
```

## 绑定引脚回调函数

```c
mr_err_t (*rx_cb)(mr_device_t device, void *args); 
```

**回调函数**:device为触发回调设备，args传入中断来源。

使用示例如下所示:

```c
/* 定义回调函数 */
mr_err_t pin_device_cb(mr_device_t device, void *args)
{
mr_uint32_t *line = args;           /* 获取中断源 */

/* 判断中断源是line-13 */
if (*line == 13)
{
/* do something */
}
}

/* 绑定PIN函数回调函数 */
mr_device_ioctl(pin_device, MR_CTRL_SET_RX_CB, pin_device_cb);
```