# Automated Build Tool

[中文](tool)

<!-- TOC -->
* [Automated Build Tool](#automated-build-tool)
  * [Build Process](#build-process)
  * [Supported Commands](#supported-commands)
<!-- TOC -->

The MR framework provides an automated build system. The script-driven build tool can automatically and efficiently
complete the introduction and configuration according to different IDEs, greatly reducing the learning cost of using the
framework and the difficulty of integration with different environments.

## Build Process

1. Run `python tool.py` in the command line to automatically install the required environment for the tool.

2. Run `python tool.py -m` to complete the configuration of the MR framework in a graphical interface. After the
   configuration is completed, automatically generate the `mr_config.h` header file.

3. Run `python tool.py -b` automated build projects.

The automated build tool will complete:

- Automatic import of MR framework files
- Automatic addition of MR framework header file paths
- Automatically enable GNU support for the IDE compilation toolchain

## Supported Commands

- `-b`：Build projects for the development environment (MDK5, Eclipse support)
- `-m`: Launches the graphical configuration interface
- `lic`: Displays license information

The build script greatly simplifies the configuration and integration process, allowing developers to quickly start
development without needing to understand the internal workings of the project.