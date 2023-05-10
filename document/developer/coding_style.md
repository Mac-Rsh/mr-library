## 格式与风格

### 缩进与换行

1. 仅使用4个空格进行代码缩进,禁止使用tab键。
2. 每行代码长度控制在120个字符内,超过换行,下一行缩进4个空格。
3. 大括号`{`与`}`单独占一行。
   例如:

   ```c
   if (condition)  
   {
       do_something();
   }

   void func(void)  
   {
       ...
   }
   ```

### 空格与空行

1. 操作符两边各加一个空格。
   例如:

   ```c  
   x = 1;  
   a + b;
   ```

2. 逗号`,`后加一个空格。
   例如:

   ```c
   void foo(int a, char b)
   ```

### 头文件

1. 头文件为避免重复包含,使用`_FILE_H_`宏进行保护。
2. 头文件按字母顺序导入，库文件优先。库头文件使用`<>`导入，用户头文件使用`""`导入。
   例如:

   ```c  
   #include <assert.h>   
   #include <stdio.h>
   #include "bar.h"
   #include "foo.h"
   ```

## 命名规范

### 类型

1. 枚举类型命名全部小写,下划线分隔，枚举常量全部大写。仅开发者使用的枚举常量以`_`开头
   例如:

   ```c
   enum color_type
   {
       RED,  
       GREEN,
       BLUE,
       _PRIVATE_ENUM,
   };
   ```

2. 结构体、联合体、类型定义命名小写,下划线分隔。
   例如:

   ```c
   struct foo
   {
       int bar;
       union
       {
           int baz;
           char qux;
       }
   };
   ```

### 函数与变量

1. 函数命名小写,下划线分隔,硬件相关函数加`hw`标识,中断服务函数加`isr`标识。
   例如:

   ```c
   void do_something(int arg1, char arg2)
   {
       ...
   }
   
   mr_err_t mr_hw_serial_add(mr_serial_t serial, const char *name, struct mr_serial_ops *ops, void *data)
   {
       ...
   }
   
   void mr_hw_serial_isr(mr_serial_t serial, mr_uint32_t event)
   {
       ...
   }
   ```

2. 变量命名小写,下划线分隔
   例如:

   ```c
   int num_times;
   char *p;
   ```

3. 常量命名全部大写,下划线分隔。
   例如:

   ```c
   #define PI 3.14
   ```

4. typedef定义使用类型名加`_t`后缀。
   例如:

   ```c
   typedef unsigned int mr_uint32_t;
   typedef struct mr_object *mr_object_t;
   ```

### 文件名

1. 文件名使用小写字母和下划线,不使用大写字母，配置文件以`config`结尾。
   例如:

   ```c
   foo_bar.c
   test_case.h
   mrconfig.h
   ```

### 宏

1. 宏定义全部大写,下划线分隔,用户可修改的宏配置需加`CONF`标识，仅开发者使用的宏以`_`开头。
   例如:

   ```c
   #define MR_CONF_NAME_MAX 15
   #define _PRIVATE_MACRO  
   ```

## 注释规范

1. 单行注释使用`/* */`，多行注释使用:

    ```c
    /*
    * 多行注释
    * 第二行
    */
    ```

2. 函数注释使用Doxygen风格:

   ```c
   /**
   * @brief This function add object to the container.
   *
   * @param object The object to be added.
   * @param name The name of the object.
   * @param type The target container flag.
   *
   * @return MR_ERR_OK on success, otherwise an error code.
   */
   mr_err_t mr_object_add(mr_object_t object, const char *name, enum container_type type)
   {
       ...
   }
   ```