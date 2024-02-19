# PIN设备

[English](pin_EN.md)

<!-- TOC -->
* [PIN设备](#pin设备)
  * [打开PIN设备](#打开pin设备)
  * [关闭PIN设备](#关闭pin设备)
  * [控制PIN设备](#控制pin设备)
    * [设置/获取引脚编号](#设置获取引脚编号)
      * [引脚编号](#引脚编号)
    * [设置引脚模式](#设置引脚模式)
      * [引脚模式](#引脚模式)
    * [设置/获取外部中断回调函数](#设置获取外部中断回调函数)
  * [读取PIN设备引脚电平](#读取pin设备引脚电平)
  * [写入PIN设备引脚电平](#写入pin设备引脚电平)
  * [使用示例：](#使用示例)
<!-- TOC -->

## 打开PIN设备

```c
int mr_dev_open(const char *path, int flags);
```

| 参数      | 描述      |
|---------|---------|
| path    | 设备路径    |
| flags   | 打开设备的标志 |
| **返回值** |         |
| `>=0`   | 设备描述符   |
| `<0`    | 错误码     |

- `path`：PIN设备路径一般为：`pin`。
- `flags`：打开设备的标志，支持 `MR_O_RDONLY`、 `MR_O_WRONLY`、 `MR_O_RDWR`。

注：使用时应根据实际情况为不同的任务分别打开PIN设备，并使用适当的`flags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭PIN设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

注：关闭设备时并不会将之前的配置恢复到默认状态，需要用户自行根据实际情况进行恢复。

## 控制PIN设备

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
    - `MR_IOC_PIN_SET_NUMBER`：设置引脚编号。
    - `MR_IOC_PIN_SET_MODE`：设置引脚模式。
    - `MR_IOC_PIN_SET_EXTI_CALL`：设置外部中断回调函数。
    - `MR_IOC_PIN_GET_NUMBER`：获取引脚编号。
    - `MR_IOC_PIN_GET_MODE`：获取引脚模式。
    - `MR_IOC_PIN_GET_EXTI_CALL`：获取外部中断回调函数。

### 设置/获取引脚编号

#### 引脚编号

不同 `MCU` 的 `GPIO` 数量、功能、命名规则等往往不同，所以 `MR` 使用数字的方式来定义 `GPIO` 引脚，以便在不同 `MCU` 上使用统一的接口。

默认计算公式为：`Number = Port * 16 + Pin`，其中 `Port` 为 `GPIO` 端口编号，`Pin` 为 `GPIO` 引脚编号。
例如 `PC13` 对应的 `Port` 为 `C`，`Pin` 为 `13`，则 `Number = (C - A * 16) + 13 = 32 + 13 = 45`。

注：此规则并不适用于所有 `MCU`，对于特殊的需求需要查看底层驱动设定。

```c
/* 定义引脚编号 */
#define PIN_NUMBER                      45

/* 设置引脚编号 */
mr_dev_ioctl(ds, MR_IOC_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, PIN_NUMBER));

/* 获取引脚编号 */
int number;
mr_dev_ioctl(ds, MR_IOC_PIN_GET_NUMBER, &number);
```

不依赖PIN接口：

```c
/* 定义引脚编号 */
#define PIN_NUMBER                      45

/* 设置引脚编号 */
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, PIN_NUMBER));

/* 获取引脚编号 */
int number;
mr_dev_ioctl(ds, MR_IOC_GPOS, &number);
```

### 设置/获取引脚模式

#### 引脚模式

`MR` 支持6种普通模式：

- `MR_PIN_MODE_NONE`：无模式，引脚恢复默认状态。
- `MR_PIN_MODE_OUTPUT`：输出模式，引脚为推挽输出模式。
- `MR_PIN_MODE_OUTPUT_OD`：输出模式，引脚为开漏输出模式。
- `MR_PIN_MODE_INPUT`：输入模式，引脚为浮空输入模式。
- `MR_PIN_MODE_INPUT_DOWN`：输入模式，引脚为下拉输入模式。
- `MR_PIN_MODE_INPUT_UP`：输入模式，引脚为上拉输入模式。

以及5种外部中断模式：

- `MR_PIN_MODE_IRQ_RISING`：外部中断模式，上升沿触发中断。
- `MR_PIN_MODE_IRQ_FALLING`：外部中断模式，下降沿触发中断。
- `MR_PIN_MODE_IRQ_EDGE`：外部中断模式，上升沿和下降沿触发中断。
- `MR_PIN_MODE_IRQ_LOW`：外部中断模式，低电平触发中断。
- `MR_PIN_MODE_IRQ_HIGH`：外部中断模式，高电平触发中断。

```c
/* 定义引脚模式 */
#define PIN_MODE                        MR_PIN_MODE_OUTPUT

/* 设置引脚模式 */
mr_dev_ioctl(ds, MR_IOC_PIN_SET_MODE, MR_MAKE_LOCAL(int, PIN_MODE));

/* 获取引脚模式 */
int mode;
mr_dev_ioctl(ds, MR_IOC_PIN_GET_MODE, &mode);
```

不依赖PIN接口：

```c
/* 定义引脚模式 */
#define PIN_MODE                        1

/* 设置引脚模式 */
mr_dev_ioctl(ds, MR_IOC_SCFG, MR_MAKE_LOCAL(int, PIN_MODE));

/* 获取引脚模式 */
int mode;
mr_dev_ioctl(ds, MR_IOC_GCFG, &mode);
```

### 设置/获取外部中断回调函数

```c
#define PIN_NUMBER                      45
/* 定义外部中断回调函数 */
void fn(int desc, void *args)
{
    /* 获取引脚编号 */
    int number = *(int *)args;
    if (number == PIN_NUMBER)
    {
        /* 处理外部中断事件 */
    }
}

/* 设置外部中断回调函数 */
mr_dev_ioctl(ds, MR_IOC_PIN_SET_EXTI_CALL, fn);

/* 获取外部中断回调函数 */
void (*callback)(int desc, void *args);
mr_dev_ioctl(ds, MR_IOC_PIN_GET_EXTI_CALL, &callback);
```

不依赖PIN接口：

```c
#define PIN_NUMBER                      45
/* 定义外部中断回调函数 */
void fn(int desc, void *args)
{
    /* 获取引脚编号 */
    int number = *(int *)args;
    if (number == PIN_NUMBER)
    {
        /* 处理外部中断事件 */
    }
}

/* 设置外部中断回调函数 */
mr_dev_ioctl(ds, MR_IOC_SRCB, fn);

/* 获取外部中断回调函数 */
void (*callback)(int desc, void *args);
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback);
```

注：

- 只要有引脚触发外部中断，即会调用所有回调函数，请一定检查是否引脚是否正确。

## 读取PIN设备引脚电平

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 读取数据缓冲区 |
| count   | 读取数据大小  |
| **返回值** |         |
| `>=0`   | 读取数据大小  |
| `<0`    | 错误码     |

```c
/* 读取引脚电平 */
uint8_t pin_level;
int ret = mr_dev_read(ds, &pin_level, sizeof(pin_level));
/* 是否读取成功 */
if (ret != sizeof(pin_level))
{
    return ret;
}
```

## 写入PIN设备引脚电平

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
```

| 参数      | 描述      |
|---------|---------|
| desc    | 设备描述符   |
| buf     | 写入数据缓冲区 |
| count   | 写入数据大小  |
| **返回值** |         |
| `>=0`   | 写入数据大小  |
| `<0`    | 错误码     |

- 电平：`0`：低电平；`1`：高电平。

```c
/* 写入引脚电平 */
uint8_t pin_level = 1;
int ret = mr_dev_write(ds, &pin_level, sizeof(pin_level));
/* 是否写入成功 */
if (ret != sizeof(pin_level))
{
    return ret;
}
```

## 使用示例：

```c
#include "include/mr_lib.h"

/* 定义LED引脚(PC13)和KEY引脚(PA0) */
#define LED_PIN_NUMBER                  45
#define KEY_PIN_NUMBER                  0

/* 定义LED和KEY描述符 */
int led_ds = -1;
int key_ds = -1;

void key_call(int desc, void *args)
{
    int number = *((int *)args);

    if (number == KEY_PIN_NUMBER)
    {
        /* 打印回调函数描述符 */
        mr_printf("KEY callback\r\n");
    }
}

void led_key_init(void)
{
    int ret = MR_EOK;

    /* 初始化LED */
    led_ds = mr_dev_open("pin", MR_O_WRONLY);
    if (led_ds < 0)
    {
        mr_printf("led open failed: %s\r\n", mr_strerror(led_ds));
        return;
    }
    /* 打印LED描述符 */
    mr_printf("LED desc: %d\r\n", led_ds);
    /* 设置到LED引脚 */
    mr_dev_ioctl(led_ds, MR_IOC_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, LED_PIN_NUMBER));
    /* 设置LED引脚为推挽输出模式 */
    ret = mr_dev_ioctl(led_ds, MR_IOC_PIN_SET_MODE, MR_MAKE_LOCAL(int, MR_PIN_MODE_OUTPUT));
    if (ret < 0)
    {
        mr_printf("led set mode failed: %s\r\n", mr_strerror(ret));
        return;
    }

    /* 初始化KEY */
    key_ds = mr_dev_open("pin", MR_O_RDONLY);
    if (key_ds < 0)
    {
        mr_printf("key open failed: %s\r\n", mr_strerror(key_ds));
        return;
    }
    /* 打印KEY描述符 */
    mr_printf("KEY desc: %d\r\n", key_ds);
    /* 设置到KEY引脚 */
    mr_dev_ioctl(key_ds, MR_IOC_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, KEY_PIN_NUMBER));
    /* 设置KEY外部中断回调函数 */
    mr_dev_ioctl(key_ds, MR_IOC_PIN_SET_EXTI_CALL, key_call);
    /* 设置KEY引脚为外部中断（下降沿）模式 */
    ret = mr_dev_ioctl(key_ds, MR_IOC_PIN_SET_MODE, MR_MAKE_LOCAL(int, MR_PIN_MODE_IRQ_FALLING));
    if (ret < 0)
    {
        mr_printf("key set mode failed: %s\r\n", mr_strerror(ret));
    }
}
/* 导出到自动初始化（APP级） */
MR_INIT_APP_EXPORT(led_key_init);

int main(void)
{
    /* 自动初始化（led_key_init函数将在此处自动调用） */
    mr_auto_init();

    while(1)
    {
        /* 翻转LED引脚电平 */
        uint8_t level = 0;
        mr_dev_read(led_ds, &level, sizeof(level));
        level = !level;
        mr_dev_write(led_ds, &level, sizeof(level));
        mr_delay_ms(500);
    }
}
```

按下KEY后，串口打印 "KEY callback"，LED每500ms翻转一次。
