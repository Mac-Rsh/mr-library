# ST配置教程

[English](README_EN.md)

注：

- 此`BSP`仅支持`HAL`库，暂不支持标准库或`LL`库需求。
- 此`BSP`需与`CubeMx`结合使用。

## 创建工程

使用`CubeMx`创建对应芯片的工程。使能需要使用的功能（仅需要配置管脚映射关系，不需要设置具体参数值）。

注：禁止使能中断，否则会报重复定义错误（`MR`框架已接管中断）。

## 将项目导入工程（2.添加驱动）

复制`bsp/st/driver`和`stm32xxx/driver`文件至`driver`。

## 移植驱动

参考上一步中`stm32xxx/driver`路径下`Kconfig`和`mr_board.h`，修改参数，完成移植与裁剪。

## 继续按仓库目录下`README`，添加 mr-library
