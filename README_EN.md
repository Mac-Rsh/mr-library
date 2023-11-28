# MR frame

 ----------

<!-- TOC -->
* [MR frame](#mr-frame)
* [Brief introduction](#brief-introduction)
* [Key characteristic](#key-characteristic)
* [Major component](#major-component)
* [Standardized device interface](#standardized-device-interface)
* [Configuration tool](#configuration-tool)
* [Directory structure](#directory-structure)
<!-- TOC -->

 ----------

# Brief introduction

The `MR` framework is a lightweight framework designed specifically for embedded systems. It fully considers the
resource and performance requirements of embedded systems.

By providing standardized device management interfaces, it greatly simplifies the development of embedded applications
and helps developers quickly build embedded applications.

The framework provides developers with standardized `open`, `close`, `ioctl`, `read`, `write` and other
interfaces. It decouples the applications from the low-level hardware drivers. The applications do not need to know the
implementation details of the drivers.

When the hardware changes, the applications can seamlessly migrate to the new hardware by only adapting the low-level
drivers. This greatly improves the reusability of software and its extensibility to new hardware.

 ----------

![](E:\Workspace\Clion\mr-library\mr-library\document\picture\README.png)

 ----------

# Key characteristic

- Standardized device access interfaces
- Decoupled application and driver development
- Simplified lower-level driver and application development
- Lightweight and easy to use with low resource usage
- Modular design with loose coupling between components for independent development and extremely low hardware migration
  costs
- Supported in bare-metal and operating system environments

# Major component

- Device framework: Provides standardized device access interfaces
- Memory management: Dynamic memory management
- Tools: Common data structures like lists, queues, balanced trees etc.
- Various functional components

 ----------

# Standardized device interface

All operations of the device can be implemented through the following interfaces:

| interface            | describe                    |
|:---------------------|:----------------------------|
| mr_dev_register      | Registered device           |
| mr_dev_open          | Open device                 |
| mr_dev_close         | Close device                |
| mr_dev_ioctl         | Control device              |
| mr_dev_read          | Read data from the device   |
| mr_dev_write         | Writes data to the device   |
| mr_dev_isr           | Device interrupt control    |
| mr_dev_get_full_name | Obtain the full device name |

 ----------

# Configuration tool

`MR` provides `Kconfig` visual configuration tool that developers can configure without deep knowledge of the source
code.

`Kconfig` will automatically generate the configuration options interface based on the configuration file. Developers
can select the functional components that need to be enabled and set relevant parameters through simple operations.

![](E:\Workspace\Clion\mr-library\mr-library\document\picture\README_Kconfig.png)

![](E:\Workspace\Clion\mr-library\mr-library\document\picture\README_Kconfig1.png)

By modifying parameters, you can quickly tailor the required functions. After the configuration is complete,
the `Python` script automatically generates the configuration file.

 ----------

# Directory structure

| name       | describe                       |
|:-----------|:-------------------------------|
| bsp        | Board support package          |
| components | Components                     |
| device     | Device file                    |
| document   | Document                       |
| driver     | Driver file                    |
| include    | Library header file            |
| source     | Library source file            |
| Kconfig    | Configuration files            |
| kconfig.py | Automatic configuration script |
| LICENSE    | Open-source license            |

 ----------