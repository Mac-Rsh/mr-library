# MSH组件

[English](msh_EN.md)

<!-- TOC -->
* [MSH组件](#msh组件)
  * [支持的按键](#支持的按键)
  * [命令格式](#命令格式)
  * [内置命令](#内置命令)
  * [自定义命令](#自定义命令)
  * [参数访问](#参数访问)
  * [使用示例：](#使用示例)
  * [如何使用](#如何使用)
<!-- TOC -->

`msh` 是 `mr-library`的命令行组件，`msh`是一个命令行解释器，提供用户交互的界面，用户通过在命令行中输入命令和参数，`msh`
负责解释和执行用户输入的命令。

## 支持的按键

| 按键    | 描述              |
|:------|:----------------|
| 退格键   | 删除当前光标前的字符      |
| 删除键   | 删除当前光标后的字符      |
| 左右方向键 | 向左向右移动光标        |
| Tab键  | 根据光标所在的位置自动补全命令 |
| 回车键   | 运行命令            |

## 命令格式

所有命令均以空格分隔，格式为：

`命令 [参数1] [参数2] ... [参数n]`

## 内置命令

`msh`内置了一些基础命令如`help`、`clear`等。

启动后使用`help`命令可以查看所有内置命令帮助。

```c
msh> help
lsdev    - List all devices.
logo     - Show the logo.
clear    - Clear the screen.
help     - Show help information.
```

## 自定义命令

通过内置的宏将命令导出。

```c
MR_MSH_CMD_EXPORT(name, fn, help);
```

| 参数   | 描述     |
|:-----|:-------|
| name | 命令名    |
| fn   | 命令处理函数 |
| help | 命令帮助描述 |

命令处理函数：

```c
int cmd_fn(int argc, void *argv)
{
    
}
```

| 参数   | 描述   |
|:-----|:-----|
| argc | 参数数量 |
| argv | 参数表  |

注：命令处理函数中最后需要输出`\r\n`换行，否则提示词将紧跟在当前行。

## 参数访问

```c
char *p = MR_MSH_GET_ARG(index);
```

## 使用示例：

```c
int hello_fn(int argc, void *argv)
{
    for (size_t i = 0; i < argc; i++)
    {
        mr_msh_printf("hello %s\r\n", MR_MSH_GET_ARG(i));
    }
    return MR_EOK;
}
MR_MSH_CMD_EXPORT(hello, hello_fn, "Hello every one");
```

运行`hello`命令：

```c
msh> hello 1 2 3 4
hello 1
hello 2
hello 3
hello 4
msh>
```

## 如何使用

```c
int main(void)
{
    /* 自动初始化 */
    mr_auto_init();
    
    while(1)
    {
        /* msh处理 */
        mr_msh_handle();
    }   
}
```
