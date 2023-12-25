# MR开发工具Python库

[English](build_EN.md)

此Python库提供了用于构建和配置嵌入式项目的MDK-ARM和Eclipse IDE功能。

 ----------

<!-- TOC -->
* [MR开发工具Python库](#mr开发工具python库)
  * [类](#类)
    * [MDK5](#mdk5)
    * [Eclipse](#eclipse)
  * [函数](#函数)
    * [build_mdk()](#buildmdk)
    * [build_eclipse()](#buildeclipse)
    * [menuconfig()](#menuconfig)
    * [show_logo()](#showlogo)
    * [show_license()](#showlicense)
  * [类](#类-1)
  * [命令行](#命令行)
  * [使用示例](#使用示例)
<!-- TOC -->

 ----------

## 类

### MDK5

表示MDK-ARM项目文件(.uvprojx)。

**方法**

- **add_include_path(path)** - 添加包含路径到项目中

- **add_include_paths(paths)** - 批量添加包含路径到项目中

- **add_files_new_group(name, files)** - 向项目中添加文件到新组中

- **add_path_files(path)** - 向组中添加路径中的所有文件

- **add_path_c_files(path)** - 向组中添加路径中的C/C++文件

- **use_gnu(enable)** - 启用或禁用GNU编译器

- **save()** - 保存项目文件更改

### Eclipse

表示Eclipse CDT项目文件(.cproject)。

**方法**

- **add_include_path(path)** - 添加包含路径到项目中

- **use_auto_init()** - 在链接脚本中启用自动初始化段

- **save()** - 保存项目文件更改

## 函数

### build_mdk()

构建MDK项目

### build_eclipse()

构建Eclipse项目

### menuconfig()

运行Kconfig菜单配置工具

### show_logo()

显示库logo

### show_license()

显示或说明许可证

## 类

- **MR** - 表示库根路径和文件

- **log_print(level, text)** - 打印格式化日志消息

## 命令行

可以使用以下选项从命令行运行库:

- `-m`、`--menuconfig` - 运行菜单配置
- `-mdk`、`--mdk` - 使用MDK构建
- `-ecl`、`--eclipse` - 使用Eclipse构建
- `-lic`、`--license` - 显示许可证

## 使用示例

```python
mdk_proj_path = "D:/workspace"
include_path = "D:/workspace/include"
c_file_path = "D:/workspace/source"
# 实例MDK对象
mdk_proj = MDK5(mdk_proj_path)
# 添加包含路径
mdk_proj.add_include_path(include_path)
# 添加C文件
mdk_proj.add_path_c_files(c_file_path)
# 使用GNU
mdk_proj.use_gnu(True)
# 保存
mdk_proj.save()
```
