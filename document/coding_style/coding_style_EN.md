# Format and Style

[中文](coding_style.md)

<!-- TOC -->
* [Format and Style](#format-and-style)
  * [Indentation and Line Breaks](#indentation-and-line-breaks)
  * [Spaces and Empty Lines](#spaces-and-empty-lines)
  * [Header Files](#header-files)
* [Naming Conventions](#naming-conventions)
  * [Types](#types)
  * [Functions and Variables](#functions-and-variables)
  * [File Names](#file-names)
  * [Macros](#macros)
* [Comment Conventions](#comment-conventions)
<!-- TOC -->

## Indentation and Line Breaks

1. Only use 4 spaces for code indentation, tabs are forbidden.
2. Keep each line of code within 120 characters, break lines and indent the next line by 4 spaces if exceeded.
3. Curly braces `{` and `}` should each be on their own line.
   For example:

   ````c
   if (condition)  
   {
       do_something();
   }

   void func(void)  
   {
       ...
   }
   ```

## Spaces and Empty Lines

1. Add a space on each side of operators, without spaces for pre/post increment/decrement.
   For example:

   ````c
   x = 1;
   a + b; 
   i++;
   ```

2. Add a space after commas.
   For example:

   ````c
   void foo(int a, char b);
   ```

## Header Files

1. Use the `_FILE_H_` macro to avoid duplicate includes.
2. Import headers in alphabetical order, with library headers first. Use `<>` for library headers and `""` for other
   headers.
   For example:

   ````c
   #include <stdlib.h>    
   #include <stdio.h>
   #include "bar.h"
   #include "foo.h"
   ```

# Naming Conventions

## Types

1. Enum types are all lowercase with underscores, enum constants start with uppercase.
   For example:

   ````c
   enum color_type
   {
       Red,  
       Green,
       Blue,
       Light_Pink,
   };
   ```

2. Structures, unions and typedefs are lowercase with underscores.
   For example:

   ````c
   struct foo
   {
       char *name;
       union coordinate
       {
           int x;
           int y; 
       }
   };
   ```

## Functions and Variables

1. Function names are lowercase with underscores.
   For example:

   ````c
   void do_something(int arg1, char arg2)
   {
       ...
   }
   ```

2. Functions should be block written by modules, with comments for large blocks that cannot be easily understood through
   code. Returning values without special meaning can be adjacent to other blocks, otherwise they should be single
   blocks.

3. Variable names are lowercase with underscores.
   For example:

   ````c
   int num_times;
   char *pointer;
   ```

4. Macro names are all uppercase with underscores. User configurable macros should have `CFG` prefix.
   For example:

   ````c
   #define MR_CFG_OBJECT_NAME_SIZE      15
   ```

5. Typedefs use the type name with `_t` suffix (avoid for structs/pointers for clarity).
   For example:

   ````c
   typedef unsigned int uint32_t;
   ```

6. Variables should have clear meaning when used. For pointers without writes, convert to variables.
   For example:

   ````c
   void func(int *args) 
   {
       /* Convert to avoid accidental overwrite */
       int value = *args;
       printf("%d\n", value);
   }
   ```

## File Names

1. File names are lowercase with underscores, no uppercase letters, config files end with `config`.
   For example:

   ````c
   foo_bar.c
   test_case.h
   mrconfig.h
   ```

## Macros

1. Macro names are ALL_CAPS with underscores, user configurable macros have `CFG` prefix.
   For example:

   ````c
   #define MR_CFG_OBJECT_NAME_SIZE      15
   ```

2. Static macro functions (no computation, assignment etc needed at runtime) follow macro naming. Dynamic macro
   functions (with runtime operations) follow function naming.

# Comment Conventions

1. Single line comments use `/* */`, multi-line comments:

    ```c
    /**
     * Multi-line comment 
     * Second line
     */
    ```

2. Function comments use Doxygen style:

   ````c
   /**
    * @brief This function registers a device.
    *
    * @param dev The device. 
    * @param name The name of the device.
    * @param type The type of the device.
    * @param sflags The support flags of the device.
    * @param ops The operations of the device.
    * @param drv The driver of the device.
    *
    * @return MR_EOK on success, otherwise an error code.
    */
   ```

3. Structs, macros etc need comments:

    ```c
   /**
    * @brief Driver structure.
    */
   struct mr_drv
   {
       uint32_t type;                                                  /**< Device type */
       void *ops;                                                      /**< Driver operations */
       void *data;                                                     /**< Driver data */
   };
   
   /**
    * @brief Null pointer.
    */
   #define MR_NULL                      (void *)0
   ````

4. Function declarations need comments to include all similar functions:

   ```c
   /**
    * @addtogroup Device description
    * @{
    */
   int mr_dev_open(const char *name, uint32_t oflags); 
   int mr_dev_close(int desc);
   ssize_t mr_dev_read(int desc, void *buf, size_t size);
   ssize_t mr_dev_write(int desc, const void *buf, size_t size);
   int mr_dev_ioctl(int desc, int cmd, void *args);
   /** @} */
   ```