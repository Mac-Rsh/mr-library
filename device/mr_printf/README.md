`mr-library` `mr-printf`
# 简介
**mr-printf** 模块为 **mr-library** 项目下的可裁剪模块，以C语言编写，可快速移植到各种平台（主要以嵌入式mcu为主）。
**mr-printf** 模块用以替代 **libc** 中 **printf**， 可在较小资源占用的同时支持绝大部分 **printf** 功能，于此同时还支持对单独功能模块的裁剪以减少用户不需要功能的资源占用。


----------


# **mr-printf** 特点
* 资源占用低。
- 简单易用。
- 通用性强，不同平台仅需适配一个接口即可使用。
* 松解耦，模块化。


----------


# 代码目录
**mr-printf** 源代码目录结构如下图所示：
|名称|描述|
|:---|:---|
|mr_printf.c|源代码|
|mr_printf.h|头文件|
|mr_printf_cfg.h|配置文件|


----------


# 需适配接口
|函数|描述|
|:---|:---|
|void mr_putc(char data)|输出 char 字符（函数原形为weak类型，故仅需在工程中定义即可）|


----------


# 可使用接口
|函数|描述|
|:---|:---|
|int mr_printf(char *fmt, ...)|与libc中的 printf 功能相同，返回值为输出的字符个数|


----------


# 配置选项
可通过修改配置文件( mr_printf_cfg.h )实现功能修改
|配置|描述|
|:---|:---|
|USING_MR_PRINTF|使用 mr_printf 输出功能|
|USING_MR_PRINTF_DEC|使用 mr_printf 十进制输出功能|
|USING_MR_PRINTF_HEX|使用 mr_printf 十六进制输出功能|
|USING_MR_PRINTF_OCT|使用 mr_printf 八进制输出功能|
|USING_MR_PRINTF_UNSIGNED|使用 mr_printf 无符号整型输出功能 |
|USING_MR_PRINTF_CHAR|使用 mr_printf 字符输出功能 |
|USING_MR_PRINTF_STRING|使用 mr_printf 字符串输出功能 |
|USING_MR_PRINTF_FLOAT|使用 mr_printf 浮点输出功能 |


----------


# 使用流程
1. 复制 `模块（mr_printf）` 文件夹到您的工程文件。
2. 按需修改 `配置文件（mr_printf_cfg.h）`的配置.
3. 查看 `源代码（mr_printf.c）` 文件中的 user specification，按说明适配接口。
4. 将 `头文件（mr_printf.h）` 引用到您的工程。
5. 开始愉快的使用。



----------



# 贡献代码
如果您在使用 **mr-printf** 模块中遇到了 bug 或是 您有自己的想法，欢迎您提交 pr 或者联系我(email)：macrsh@outlook.com

## 感谢各位对本仓库的贡献！
