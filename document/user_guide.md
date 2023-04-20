# 使用指南

 ----------

## 准备工作

在一切开始前，请先从 [gitee仓库](https://gitee.com/MacRsh/mr-library.git) 下载源代码。

<br>源代码包含文件描述如下表所示:

| 名称         | 描述      |
|:-----------|:--------|
| mr_library | 库文件     |
| -bsp       | 板级支持包   |
| -config    | 配置文件    |
| -device    | 设备抽象层文件 |
| -document  | 文档      |
| -driver    | 驱动文件    |
| -include   | 库头文件    |
| -src       | 库源文件    |

选取工程所需文件:

| 名称         | 描述      |
|:-----------|:--------|
| mr_library | 库文件     |
| -config    | 配置文件    |
| -device    | 设备抽象层文件 |
| -driver    | 驱动文件    |
| -include   | 库头文件    |
| -src       | 库源文件    |

## 配置工程

1. 从 `bsp` 目录中选取对应芯片驱动文件放入 `driver` 中，同时配置 `config` 目录下 `board.c` 、 `board.h` 文件（或在 `bsp`
   目录中寻找芯片对应文件）。
2. 添加 `mr_library` 文件到工程，添加文件夹路径。
3. 引入头文件 `mr_lib.h` 即可使用内核部分，若在 `mr_config.h` 中启用了 `Device` 下任意设备，则需引入头文件 `mr_drv.h`
   即可使用设备驱动部分。




