﻿`mr-library` `mr-auto-init`
# 简介
**mr-auto-init** 模块为 **mr-library** 项目下的可裁剪模块，以C语言编写，可快速移植到各种平台（主要以嵌入式mcu为主）。
**mr-auto-init** 模块通过隐式调用初始化函数，使代码更简介更模块化。


----------
# **mr-auto-init** 特点
* 资源占用低。
- 实用高效。
* 无需过多适配。

----------

# 代码目录
**mr-auto-init** 源代码目录结构如下图所示：
|名称|描述|
|:---|:---|
|mr_auto_init.c|源代码|
|mr_auto_init.h|头文件|


----------


# 需适配接口
|函数|描述|
|:---|:---|
|无|无|



----------


# 可使用接口
|函数|描述|
|:---|:---|
|INIT_BOARD_EXPORT()|将函数导出到板级初始化|
|INIT_DEVICE_EXPORT()|将函数导出到设备初始化|
|INIT_ENV_EXPORT()|将函数导出到环境初始化|
|INIT_APP_EXPORT()|将函数导出到应用初始化|
|mr_auto_init()|初始化所有导出的函数|



----------


# 初始化顺序

 1. 板级初始化：`INIT_BOARD_EXPORT`
 2. 设备初始化：`INIT_DEVICE_EXPORT`
 3. 环境初始化：`INIT_ENV_EXPORT`
 4. 应用初始化：`INIT_APP_EXPORT`
 
----------

# 使用流程
1. 复制 `模块(mr_auto_init)` 文件夹到您的工程文件。
2. 调用 `宏函数(INIT_BOARD_EXPORT / INIT_DEVICE_EXPORT / INIT_ENV_EXPORT / INIT_APP_EXPORT)` 导出初始化函数。
3. 将 `头文件(mr_auto_init.h)` 引用到您的工程。
4. 在mian函数中调用 `初始化函数(mr_auto_init)`，将自动按初始化顺序调用导出的初始化函数。
5. 开始愉快的使用。


----------


# 使用示例
```
/* -------------------- 模拟需要导出的初始化函数 -------------------- */

int board_init(void)
{
  printf("auto-init: board\r\n");
  return 0;
}
INIT_BOARD_EXPORT(board_init);

int device_init(void)
{
  printf("auto-init: device\r\n");
  return 0;
}
INIT_DEVICE_EXPORT(device_init);

int env_init(void)
{
  printf("auto-init: env\r\n");
  return 0;
}
INIT_ENV_EXPORT(env_init);

int app_init(void)
{
  printf("auto-init: app\r\n");
  return 0;
}
INIT_APP_EXPORT(app_init);

/* -------------------- 使用 -------------------- */

int main(void)
{
    /* 初始化 */
    mr_auto_init();
    
    /* 用户代码 */
    ...
}

```


----------

# 注意事项
当您使用中出现功能无法实现并且您的编译器为 `GCC` 时，请您手动在 `link.ld` / `SECTIONS` / `.text` 中添加以下代码。
```
/* section information for initial. */
. = ALIGN(4);
_mr_init_ = .;
KEEP(*(SORT(.mri_fn*)))
_mr_init__end = .;
```

----------


# 贡献代码
如果您在使用 **mr-auto-init** 模块中遇到了 bug 或是 您有自己的想法，欢迎您提交 pr 或者联系我(email)：macrsh@outlook.com

## 感谢各位对本仓库的贡献！