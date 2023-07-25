# PIN设备使用指南

----------

## 概述

GPIO（通用目的输入/输出）是一种可以通过软件进行配置和控制的硬件接口，广泛用于嵌入式系统中外设的连接和控制。

----------

## 准备工作

1. 引用 `mrdrv.h` 头文件以使用驱动部分。
2. 调用PIN设备初始化函数（如果实现了自动初始化,则无需调用）。
3. 使能 `mrconfig.h` 头文件中PIN宏开关。

----------

## 查找PIN设备

```c
mr_device_t mr_device_find(const char *name);
```

| 参数      | 描述     |
|:--------|:-------|
| name    | PIN设备名 |
| **返回**  |        |
| PIN句柄   | 查找设备成功 |
| MR_NULL | 查找设备失败 |

----------

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

PIN设备支持以下打开方式：

```c
MR_OPEN_RDONLY                                                      /* 只读 */
MR_OPEN_WRONLY                                                      /* 只写 */
MR_OPEN_RDWR                                                        /* 可读可写 */
```

----------

## 控制PIN设备

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
| 错误码       | 控制设备失败  |

PIN设备支持以下命令：

```c
MR_CTRL_CONFIG                                                      /* 配置命令 */
MR_CTRL_SET_RX_CB                                                   /* 设置接收（外部中断）回调函数 */
```

### 配置PIN设备IO

PIN控制参数原型如下：

```c
struct mr_pin_config
{
    mr_pos_t number;                                                /* IO编号 */
    mr_uint8_t mode;                                                /* 模式 */
};
```

- 编号：由底层驱动定义，推荐编号规则为编号 = port * 16 + pin，例如B13 = 1 * 16 + 13 = 29。
- 模式：支持对GPIO进行普通和中断模式配置。

```c
/* GPIO普通模式 */
MR_PIN_MODE_NONE                                                    /* 关闭IO */
MR_PIN_MODE_OUTPUT                                                  /* 推挽输出 */
MR_PIN_MODE_OUTPUT_OD                                               /* 开漏输出 */
MR_PIN_MODE_INPUT                                                   /* 浮空输入 */
MR_PIN_MODE_INPUT_DOWN                                              /* 下拉输入 */
MR_PIN_MODE_INPUT_UP                                                /* 上拉输入 */

/* GPIO中断模式 */
MR_PIN_MODE_IRQ_RISING                                              /* 上升沿触发 */
MR_PIN_MODE_IRQ_FALLING                                             /* 下降沿触发 */
MR_PIN_MODE_IRQ_EDGE                                                /* 边缘触发 */
MR_PIN_MODE_IRQ_LOW                                                 /* 低电平触发 */
MR_PIN_MODE_IRQ_HIGH                                                /* 高电平触发 */
```

使用示例：

```c
#define PIN_NUMBER                      29

/* 查找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为推挽输出模式 */
struct mr_pin_config pin_config;
pin_config.number = PIN_NUMBER;
pin_config.mode = MR_PIN_MODE_OUTPUT;
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);
```

### 配置PIN设备IO外部中断并绑定回调函数

- 回调函数：device为触发回调设备，args传入中断来源。

使用示例：

```c
#define PIN_NUMBER                      29

/* 定义回调函数 */
mr_err_t pin_device_cb(mr_device_t device, void *args)
{
    mr_pos_t number = *(mr_pos_t *)args;                            /* 获取中断源 */
    
    /* 判断中断源B13 */
    if (number == PIN_NUMBER)
    {
        /* Do something */
    }
}

/* 查找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为上升沿触发模式 */
struct mr_pin_config pin_config;
pin_config.number = PIN_NUMBER;
pin_config.mode = MR_PIN_MODE_IRQ_RISING;
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);

/* 绑定PIN函数回调函数 */
mr_device_ioctl(pin_device, MR_CTRL_SET_RX_CB, pin_device_cb);
```

----------

## 读取PIN设备IO电平

```c
mr_ssize_t mr_device_read(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| pos       | 读取位置    |
| buffer    | 读取数据    |
| size      | 读取数据大小  |
| **返回**    |         |
| 实际读取的数据大小 |         |

- 读取位置：指需要读取的IO编号，如读取B13，则pos = 29。

使用示例：

```c
#define PIN_NUMBER                      29

/* 查找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为浮空输入模式 */
struct mr_pin_config pin_config;
pin_config.number = PIN_NUMBER;
pin_config.mode = MR_PIN_MODE_INPUT;
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);

/* 获取B13电平 */
mr_level_t pin_level = MR_LOW;
mr_device_read(pin_device, PIN_NUMBER, &pin_level, sizeof(pin_level));
```

----------

## 写入PIN设备IO电平

```c
mr_ssize_t mr_device_write(mr_device_t device, mr_pos_t pos, const void *buffer, mr_size_t size);
```

| 参数        | 描述      |
|:----------|:--------|
| device    | PIN设备句柄 |
| pos       | 写入位置    |
| buffer    | 写入数据    |
| size      | 写入数据大小  |
| **返回**    |         |
| 实际写入的数据大小 |         |

- 写入位置：指需要写入的IO编号，如读取B13，则pos = 29。

使用示例：

```c
#define PIN_NUMBER                      29

/* 查找PIN设备 */
mr_device_t pin_device = mr_device_find("pin");

/* 以可读可写的方式打开PIN设备 */
mr_device_open(pin_device, MR_OPEN_RDWR);

/* 配置B13引脚为推挽输出模式 */
struct mr_pin_config pin_config;
pin_config.number = PIN_NUMBER;
pin_config.mode = MR_PIN_MODE_OUTPUT;
mr_device_ioctl(pin_device, MR_CTRL_CONFIG, &pin_config);

/* 设置B13为高电平 */
mr_level_t pin_level = MR_HIGH;
mr_device_write(pin_device, PIN_NUMBER, &pin_level, sizeof(pin_level));
```