# PIN开发手册

 ----------

## 准备工作

1. 从[仓库](https://gitee.com/MacRsh/mr-library.git)获取最新的代码。
2. 整理文件仅保留必要的文件：

   | 名称         | 描述   |
       |:-----------|:-----|
   | mr_library |      |
   | -device    | 设备文件 |
   | -driver    | 驱动文件 |
   | -include   | 库头文件 |
   | -src       | 库源文件 |
3. 在`mrboard.h`中添加芯片头文件，并为适配的芯片添加支持的宏：BSP_GPIO_x。
4. 在`driver`目录中新建名为`drv_gpio.c`和`drv_gpio.h`，并添加版权信息：
    ```c
    /*
     * Copyright (c) 2023, mr-library Development Team
     *
     * SPDX-License-Identifier: Apache-2.0
     *
     * Change Logs:
     * Date           Author       Notes
     * 20xx-xx-xx     name         first version
     */
    ```
5. 启用`include/mrconfig.h`中的`MR_CONF_DEVICE`和`MR_CONF_PIN`。
6. 在`drv_gpio.h`中引用`device/pin/pin.h`（请按此格式引用路径）。
7. 在`drv_gpio.c`和`drv_gpio.h`中添加宏开关:
    ```c
    #if (MR_CONF_PIN == MR_CONF_ENABLE)
    ```

 ----------

## 注册PIN设备

以CH32为例，将前缀修改为适配的型号即可，如果您有关于硬件的信息需要引入，请在`mr_pin_device_add`函数`data`
参数中传入（注意使用模板时请去除中文注释）：

```c
/* 定义pin驱动 */
static struct mr_pin pin_device;

/* 注册pin设备 */
mr_err_t ch32_gpio_init(void)
{
mr_err_t ret = MR_ERR_OK;
static struct mr_pin_ops ops =
{
ch32_pin_configure,
ch32_pin_write,
ch32_pin_read,
};

ret = mr_pin_device_add(&pin_device, "pin", &ops, MR_NULL);
MR_ASSERT(ret == MR_ERR_OK);

return MR_ERR_OK;
}
/* 导出到驱动自动初始化 */
AUTO_INIT_DRIVER_EXPORT(ch32_gpio_init);
```

在`drv_gpio.h`中声明此函数（方便未实现自动初始化时调用）。

## 适配PIN设备驱动

PIN设备接口原型:

```c
struct mr_pin_ops
{
    mr_err_t (*configure)(mr_pin_t pin, struct mr_pin_config *config);
    void (*write)(mr_pin_t pin, mr_uint16_t number, mr_uint8_t value);
    mr_uint8_t (*read)(mr_pin_t pin, mr_uint16_t number);
};
```

- configure:GPIO模式配置。
- write:写入GPIO电平。
- read:读取GPIO电平。

#### configure

```c
static mr_err_t ch32_pin_configure(mr_pin_t pin, struct mr_pin_config *config)
```

mr_pin_config原型：

```c
struct mr_pin_config
{
    mr_uint16_t number;
    mr_uint8_t mode;
};
```

- number:GPIO引脚编号，计算公式(端口号,引脚号) `->` ('A' - 端口号) * 16 + 引脚号，例如 B13 = 1 * 16 + 13 = 29。
- mode:GPIO模式如下
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
    ````
  注意模式配置时，请定义一个数组，用于存储外部中断源是哪个GPIO，-1代表未被占用。
    ```c
    static mr_int32_t mask[16] = {-1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1,
                                  -1};
    ```

#### write

```c
static void ch32_pin_write(mr_pin_t pin, mr_uint16_t number, mr_uint8_t value)
```

- number:GPIO引脚编号。
- value:GPIO电平。

#### read

```c
static mr_uint8_t ch32_pin_read(mr_pin_t pin, mr_uint16_t number)
```

- number:GPIO引脚编号。
- return：GPIO电平。

#### 中断回调函数

以CH32为例，在中断中调用以下函数完成框架对中断的接管：

```c
void mr_pin_device_isr(mr_pin_t pin, mr_int32_t number)
```

- pin:PIN设备。
- number:触发中断的GPIO引脚编号。

```c
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        mr_pin_device_isr(&pin_device, mask[0]);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
```

## 发起PR，感谢您做出的贡献。