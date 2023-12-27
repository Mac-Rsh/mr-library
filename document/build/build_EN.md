# MR Development Tools Python Library

[中文](build.md)

This Python library provides functions for building and configuring embedded projects using the MDK-ARM and Eclipse IDEs.

 ----------

<!-- TOC -->
* [MR Development Tools Python Library](#mr-development-tools-python-library)
  * [Classes](#classes)
    * [MDK5](#mdk5)
    * [Eclipse](#eclipse)
  * [Functions](#functions)
    * [build_mdk()](#buildmdk)
    * [build_eclipse()](#buildeclipse)
    * [menuconfig()](#menuconfig)
    * [show_logo()](#showlogo)
    * [show_license()](#showlicense)
  * [Classes](#classes-1)
  * [Command Line](#command-line)
  * [Usage Example](#usage-example)
<!-- TOC -->

 ----------

## Classes

### MDK5

Represents an MDK-ARM project file (.uvprojx).

**Methods**

- **add_include_path(path)** - Adds an include path to the project.

- **add_include_paths(paths)** - Adds multiple include paths to the project.

- **add_files_new_group(name, files)** - Adds files to a new group in the project.

- **add_path_files(path)** - Adds all files in a path to a group.

- **add_path_c_files(path)** - Adds C/C++ files in a path to a group.

- **use_gnu(enable)** - Enables or disables the GNU compiler.

- **save()** - Saves changes to the project file.

### Eclipse

Represents an Eclipse CDT project file (.cproject).

**Methods**

- **add_include_path(path)** - Adds an include path to the project.

- **use_auto_init()** - Enables automatic initialization section in linker script.

- **save()** - Saves changes to the project file.

## Functions

### build_mdk()

Builds an MDK project.

### build_eclipse()

Builds an Eclipse project.

### menuconfig()

Runs the Kconfig menuconfig tool.

### show_logo()

Displays the library logo.

### show_license()

Displays or explains the license.

## Classes

- **MR** - Represents the library root path and files.

- **log_print(level, text)** - Prints formatted log messages.

## Command Line

The library can be run from the command line with the following options:

- `-m, --menuconfig` - Runs menuconfig
- `-mdk, --mdk` - Builds with MDK
- `-ecl, --eclipse` - Builds with Eclipse
- `-lic, --license` - Shows license

## Usage Example

```python
mdk_proj_path = "D:/workspace"
include_path = "D:/workspace/include"
c_file_path = "D:/workspace/source"
# Instance MDK object
mdk_proj = MDK5(mdk_proj_path)
# Add contain path
mdk_proj.add_include_path(include_path)
# Add C file
mdk_proj.add_path_c_files(c_file_path)
# Use GNU
mdk_proj.use_gnu(True)
# Save
mdk_proj.save()
```
