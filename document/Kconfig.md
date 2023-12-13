# 如何为项目引入Kconfig配置系统

<!-- TOC -->
* [如何为项目引入Kconfig配置系统](#如何为项目引入kconfig配置系统)
* [环境搭建](#环境搭建)
  * [验证 python 环境](#验证-python-环境)
  * [安装Kconfig](#安装kconfig)
  * [C语言头文件生成脚本](#c语言头文件生成脚本)
* [应用](#应用)
  * [创建Kconfig](#创建kconfig)
<!-- TOC -->

项目中经常会有通过宏配置或选择参数：

```c
#ifdef CFG_ENABLE
#define CFG_ARGS    "test"
void printf_args(void)
{
    printf("args: %s\n", CFG_ARGS);
}
#endif
```

当项目规模较小，配置项和依赖关系相对简单时，我们可以通过手动修改源代码来实现配置功能。但是，随着项目规模的增长和配置复杂度的增加，这种方式也会出现一些问题。

随着代码体量和调用关系的增多，对配置者来说`很难完全熟悉`
代码结构及各模块之间的依赖关系。同时，如果配置项设置较为`随意且缺乏约束`，也很可能导致`错误配置`。这对项目质量和维护都带来一定难度。

对于规模较大的项目来说，需要一套专业的配置管理系统来解决这些问题。Kconfig正是为了应对这种需求而产生的。它通过配置脚本定义各配置选项及其依赖关系，并提供`图形化界面`
供配置者操作。同时还会检查配置`合理性`，帮助我们实现定制化构建的同时`防止错误配置`。

如果有接触过Linux内核或者嵌入式操作系统RTTHREAD等，相信对Kconfig系统都不陌生。Kconfig是Linux内核和许多其他系统广泛使用的一款配置管理工具。它可以让开发人员在编译时选择性地包含或排除某些功能模块。

Kconfig
使用配置脚本定义各种配置选项及其依赖关系。通过图形或字符型用户界面，开发者可以查看并设置各种配置。然后Kconfig会生成 `.config`
文件保存选择结果。在编译的时候，会根据 `.config` 文件里设置的符号自动包含或排除对应的源代码。

![Kconfig](picture/Kconfig/Kconfig.png)

# 环境搭建

## 验证 python 环境

Kconfig依赖于 ` python` ，如果没有 ` python` 环境请自行安装。

在命令行中使用所示命令验证：

```cmd
python --version
```

显示（正确显示版本即可）：

```cmd
Python 3.11.4
```

## 安装Kconfig

```cmd
python -m pip install windows-curses
python -m pip install kconfiglib
```

在命令行中使用所示命令验证：

```cmd
menuconfig -h
```

显示信息即安装成功。

## C语言头文件生成脚本

Kconfiglib 生成的是 `.config` 文件，并非C语言文件，需要使用脚本生成，`将以下代码复制到文件中并命名为kconfig.py`。

```python
import os
import re
import sys
import textwrap

from kconfiglib import Kconfig, split_expr, expr_value, expr_str, BOOL, TRISTATE, TRI_TO_STR, AND, OR

def generate_config(kconfig_file, config_in, config_out, header_out):
    print("Parsing " + kconfig_file)
    kconf = Kconfig(kconfig_file, warn_to_stderr=False,
                    suppress_traceback=True)

    # Load config files
    print(kconf.load_config(config_in))

    # Write merged config        
    print(kconf.write_config(config_out))

    # Write headers
    print(kconf.write_autoconf(header_out))

    with open(header_out, 'r+') as header_file:
        content = header_file.read()
        header_file.truncate(0)
        header_file.seek(0)

        # Remove CONFIG_ and MR_USING_XXX following number
        content = content.replace("#define CONFIG_", "#define ")

        # Add the micro
        header_file.write("#ifndef _CONFIG_H_\n")
        header_file.write("#define _CONFIG_H_\n\n")

        header_file.write("#ifdef __cplusplus\n")
        header_file.write("extern \"C\" {\n")
        header_file.write("#endif /* __cplusplus */\n\n")

        # Write back the original data
        header_file.write(content)

        # Add the micro
        header_file.write("\n#ifdef __cplusplus\n")
        header_file.write("}\n")
        header_file.write("#endif /* __cplusplus */\n\n")
        header_file.write("#endif /* _CONFIG_H_*/\n")


def main():
    kconfig_file = 'Kconfig'
    config_in = '.config'
    config_out = '.config'
    header_out = 'config.h'
    generate_config(kconfig_file, config_in, config_out, header_out)


if __name__ == "__main__":
    main()
```

修改`main`函数中`header_out `可修改生成的函数文件名。

此脚本去除了Kconfig默认添加的`CONFIG_`前缀，并为其加上了`_CONFIG_H_`
和`C`声明。

# 应用

## 创建Kconfig

新建一个名字为 `Kconfig` 的文件（注意没有后缀名），和上一步中中创建的 `kconfig.py` 的文件放在一起。

```Kconfig
mainmenu "Demo"

menu "Args configure"
config CFG_ARG1
    bool "Enable arg1"
    default n
    
config CFG_ARG2
	int "Arg2 number"
	default 8
	range 0 64

endmenu
```

在当前目录下调用命令行，输入：

```cmd
menuconfig
```

就可以看到我们刚刚写的Demo界面

![Kconfig1](picture/Kconfig/Kconfig1.png)

回车后就可以看到配置的2个参数：

![Kconfig2](picture/Kconfig/Kconfig2.png)

设置的范围也在输入时生效了：

![Kconfig3](picture/Kconfig/Kconfig3.png)

配置完成后按`Q`退出，`Y`保存配置。

在当前目录下调用命令行，输入：

```cmd
python kconfig.py
```

运行 `python` 脚本生成`.h`文件。

运行结果：

```cmd
Parsing Kconfig
Loaded configuration '.config'
Configuration saved to '.config'
Kconfig header saved to 'config.h'
```

显示生成`config.h`成功。打开这个文件：

![Kconfig4](picture/Kconfig/Kconfig4.png)
