# ST系列 BSP使用教程

由于CUBEMX和ST-HAL库的使用极为便利，同时为了保持原有的开发流程，降低使用成本，ST系列配置仍将使用原先的开发流程。

## 1. 使用CUBEMX配置所需要的外设，仅需使能无需具体配置。
## 2. 生成KEIL工程后将 mr-library 导入至工程（参考 document/user/usage_guide.md）。
## 3. 配置 driver/mrboard.h 中宏，裁剪不需要使用或者不存在的外设。

注：keil开发环境下，无需做自动初始化适配，可直接使用。

----------

视频教程：

[下载链接](https://gitee.com/MacRsh/mr-library/blob/master/bsp/st/document/build.mkv)

 ----------

## ADC外设

由于ST系列通道对应关系不统一，需在CUBEMX中勾选需要使用ADC通道。即可通过ADC设备操作。

![ADC](https://gitee.com/MacRsh/mr-library/raw/master/bsp/st/document/use_adc.png)

## GPIO外设

无需在CUBEMX配置，即可直接通过PIN设备操作。

注：禁止在CUBEMX中勾选任何EXTI中断（会出现中断函数重复定义）。

## 串口外设

选择需要使用的串口设备为异步模式，即可直接通过SERIAL设备操作。

![UART](https://gitee.com/MacRsh/mr-library/raw/master/bsp/st/document/use_uart.png)

注：禁止在CUBEMX中勾选任何串口中断（会出现中断函数重复定义）。

## SPI外设

选择需要使用的SPI设备为主机/从机模式，即可直接通过SPI设备操作。

![SPI](https://gitee.com/MacRsh/mr-library/raw/master/bsp/st/document/use_spi.png)

注：禁止在CUBEMX中勾选任何SPI中断（会出现中断函数重复定义）。

 ----------

## 导入完成后的keil文件结构

![KEIL](https://gitee.com/MacRsh/mr-library/raw/master/bsp/st/document/keil.png)

 ----------

## mrboard.h 板级配置

1. 引用芯片头文件，在 bsp head file位置删除 #warning 替换成对应芯片头文件。

```c
/**
 * @include bsp head file
 *
 * like this: #include "stm32f1xx.h"
 */
#include "stm32f4xx.h"                                              /* 此处以F4系列为例 */
```

2. 配置引脚数量和外设，裁剪宏配置。

假设芯片为48脚，3串口设备（2，3，4串口），修改引脚数量宏，删除不存在的串口宏。

```c
/**
 * @def Bsp pin number
 */
#define MR_BSP_PIN_NUMBER               48

/**
 * @def Bsp uart
 */
#define MR_BSP_UART_2
#define MR_BSP_UART_3
#define MR_BSP_UART_4
```