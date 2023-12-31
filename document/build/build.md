# 自动构建工具

[English](build_EN.md)

<!-- TOC -->
* [自动构建工具](#自动构建工具)
  * [构建流程](#构建流程)
  * [支持的命令](#支持的命令)
<!-- TOC -->

`MR` 框架提供自动化构建系统，通过脚本驱动的构建工具可以根据不同`IDE`自动、高效地完成引入与配置。大大降低了框架使用的学习成本以及与不同环境的集成难度。

## 构建流程

1. 在命令行中运行`python build.py`，自动完成工具所需环境的安装。
2. 运行`python build.py -m`，在图形化界面中完成`MR` 框架的配置。配置完成后，会自动生成`mr_config.h`头文件。
3. 运行`python build.py -ide`，根据`IDE`类型(`-mdk`或`-ecl`)，完成自动化配置。

自动构建工具将完成：

- `MR` 框架文件的自动导入。
- `MR` 框架头文件路径的自动添加。
- 自动使能IDE编译工具链的`GNU`支持。

## 支持的命令

- `m`：启动图形用户界面来配置框架
- `lic`：显示框架的许可证信息
- `mdk`：为Keil开发环境构建项目
- `ecl`：为Eclipse开发环境构建项目设置

构建脚本大大简化了配置和集成流程，开发者无需了解工程内部，就可以快速启动开发。
