# PWM设备

[English](pwm_EN.md)

<!-- TOC -->
* [PWM设备](#pwm设备)
  * [打开PWM设备](#打开pwm设备)
  * [关闭PWM设备](#关闭pwm设备)
  * [控制PWM设备](#控制pwm设备)
    * [设置/获取通道编号](#设置获取通道编号)
    * [设置/获取通道配置](#设置获取通道配置)
    * [设置/获取频率](#设置获取频率)
  * [读取PWM通道占空比](#读取pwm通道占空比)
  * [写入PWM通道占空比](#写入pwm通道占空比)
  * [使用示例：](#使用示例)
<!-- TOC -->

## 打开PWM设备

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

- `name`：PWM设备路径一般为：`pwmx`、`pwm1`、`pwm2`。
- `flags`：打开设备的标志，支持 `MR_O_RDONLY`、 `MR_O_WRONLY`、 `MR_O_RDWR`。

注：使用时应根据实际情况为不同的任务分别打开PWM设备，并使用适当的`flags`进行管理和权限控制，以确保它们不会相互影响。

## 关闭PWM设备

```c
int mr_dev_close(int desc);
```

| 参数      | 描述    |
|---------|-------|
| desc    | 设备描述符 |
| **返回值** |       |
| `=0`    | 关闭成功  |
| `<0`    | 错误码   |

注：关闭设备时所有的通道都将被自动恢复到默认配置，重新打开后需要重新配置通道（可关闭此功能）。

## 控制PWM设备

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
    - `MR_IOC_PWM_SET_CHANNEL`：设置通道编号。
    - `MR_IOC_PWM_SET_CHANNEL_CONFIG`：设置通道配置。
    - `MR_IOC_PWM_SET_FREQ`：设置频率。
    - `MR_IOC_PWM_GET_CHANNEL`：获取通道编号。
    - `MR_IOC_PWM_GET_CHANNEL_CONFIG`：获取通道配置。
    - `MR_IOC_PWM_GET_FREQ`：获取频率。

### 设置/获取通道编号

通道编号范围：`0` ~ `31`。

```c
/* 定义通道编号 */
#define CHANNEL_NUMBER                  1

/* 设置通道编号 */
mr_dev_ioctl(ds, MR_IOC_PWM_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* 获取通道编号 */
int number;
mr_dev_ioctl(ds, MR_IOC_PWM_GET_CHANNEL, &number);
```

不依赖PWM接口：

```c
/* 定义通道编号 */
#define CHANNEL_NUMBER                  1

/* 设置通道编号 */
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* 获取通道编号 */
int number;
mr_dev_ioctl(ds, MR_IOC_GPOS, &number);
```

### 设置/获取通道配置

通道配置：

- `MR_DISABLE`：禁用通道。
- `MR_ENABLE`：启用通道。

```c
struct mr_pwm_config config = {MR_ENABLE, MR_PWM_POLARITY_NORMAL};

/* 设置通道配置 */
mr_dev_ioctl(ds, MR_IOC_PWM_SET_CHANNEL_CONFIG, &config);
/* 获取通道配置 */
mr_dev_ioctl(ds, MR_IOC_PWM_GET_CHANNEL_CONFIG, &config);
```

不依赖PWM接口：

```c
int config[] = {MR_ENABLE, 0};

/* 设置通道配置 */
mr_dev_ioctl(ds, MR_IOC_SCFG, &config);
/* 获取通道配置 */
mr_dev_ioctl(ds, MR_IOC_GCFG, &config);
```

### 设置/获取频率

```c
/* 定义频率 */
#define PWM_FREQ                        1000

/* 设置频率 */
mr_dev_ioctl(ds, MR_IOC_PWM_SET_FREQ, MR_MAKE_LOCAL(uint32_t, PWM_FREQ));

/* 获取频率 */
uint32_t freq;
mr_dev_ioctl(ds, MR_IOC_PWM_GET_FREQ, &freq);
```

不依赖PWM接口：

```c
/* 定义频率 */
#define PWM_FREQ                        1000

/* 设置频率 */
mr_dev_ioctl(ds, (0x01), MR_MAKE_LOCAL(uint32_t, PWM_FREQ));

/* 获取频率 */
uint32_t freq;
mr_dev_ioctl(ds, (-(0x01)), &freq);
```

## 读取PWM通道占空比

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
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
/* 读取占空比 */
uint32_t duty;
int ret = mr_dev_read(ds, &duty, sizeof(duty));
/* 是否读取成功 */
if (ret != sizeof(duty))
{
    return ret;
}
```

注：读取数据为PWM占空比，范围：`0` ~ `1000000`。单次读取最小单位为`uint32_t`，即4个字节。

## 写入PWM通道占空比

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
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
/* 写入占空比 */
uint32_t duty = 500000;
int ret = mr_dev_write(ds, &duty, sizeof(duty));
/* 是否写入成功 */
if (ret != sizeof(duty))
{
    return ret;
}
```

注：写入数据为PWM占空比，范围：`0` ~ `1000000`。单次写入最小单位为`uint32_t`，即4个字节。

## 使用示例：

```c
#include "include/mr_lib.h"

/* 定义通道编号和频率 */
#define CHANNEL_NUMBER                  1
#define FREQ                            1000

/* 定义PWM设备描述符 */
int pwm_ds = -1;

void pwm_init(void)
{
    int ret = MR_EOK;

    /* 初始化PWM */
    pwm_ds = mr_dev_open("pwm1", MR_O_RDWR);
    if (pwm_ds < 0)
    {
        mr_printf("PWM1 open failed: %s\r\n", mr_strerror(pwm_ds));
        return;
    }
    /* 打印PWM描述符 */
    mr_printf("PWM1 desc: %d\r\n", pwm_ds);
    /* 设置到通道1 */
    mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
    /* 设置通道使能 */
    ret = mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(struct mr_pwm_config, MR_ENABLE, MR_PWM_POLARITY_NORMAL));
    if (ret < 0)
    {
        mr_printf("Channel%d enable failed: %s\r\n", CHANNEL_NUMBER, mr_strerror(ret));
        return;
    }
    ret = mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_FREQ, MR_MAKE_LOCAL(uint32_t, FREQ));
    if (ret < 0)
    {
        mr_printf("Freq configure failed: %s\r\n", mr_strerror(ret));
    }
}
/* 导出到自动初始化（APP级） */
MR_INIT_APP_EXPORT(pwm_init);

int main(void)
{
    /* 自动初始化（pwm_init函数将在此处自动调用） */
    mr_auto_init();

    while(1)
    {
        /* 写入占空比 */
        uint32_t duty = 500000;
        int ret = mr_dev_write(pwm_ds, &duty, sizeof(duty));
        /* 是否写入成功 */
        if (ret != sizeof(duty))
        {
            mr_printf("Write failed: %s\r\n", mr_strerror(ret));
            return ret;
        }
        mr_delay_ms(1000);
    }
}
```

PWM频率设置为1000Hz，通道1输出50%占空比。
