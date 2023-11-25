## 格式与风格

### 缩进与换行

1. 仅使用4个空格进行代码缩进，禁止使用tab键。
2. 每行代码长度控制在120个字符内，超过换行，下一行缩进4个空格。
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

1. 操作符两边各加一个空格，自加自减不加空格。
   例如:

   ```c  
   x = 1;  
   a + b;
   i++;
   ```

2. 逗号`,`后加一个空格。
   例如:

   ```c
   void foo(int a, char b);
   ```

### 头文件

1. 头文件为避免重复包含，使用`_FILE_H_`宏进行保护。
2. 头文件按字母顺序导入，库文件优先。库头文件使用`<>`导入，其余头文件使用`""`导入。
   例如:

   ```c  
   #include <stdlib.h>   
   #include <stdio.h>
   #include "bar.h"
   #include "foo.h"
   ```

## 命名规范

### 类型

1. 枚举类型命名全部小写，下划线分隔，枚举常量首字母大写。
   例如:

   ```c
   enum color_type
   {
       Red,  
       Green,
       Blue,
       Light_Pink,
   };
   ```

2. 结构体、联合体、类型定义命名小写，下划线分隔。
   例如:

   ```c
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

### 函数与变量

1. 函数命名小写，下划线分隔。
   例如:

   ```c
   void do_something(int arg1, char arg2)
   {
       ...
   }
   ```

2. 函数中功能模块应分块书写，当出现较大块且无法通过代码简单明了的解读时需要标注代码块功能。当返回值无特殊功能时允许紧贴其他代码块，当有其余功能时，需单独成块。

   ```c
   void do_something1(int arg1, char arg2)
   {
        /* do something */
        ...
   
        /* do something */
        ...
        return OK;
   }
   
   void do_something2(int arg1, char arg2)
   {
        /* do something */
        ...
   
        /* do something */
        ...
   
        return /* do something */;
   }
   ```

3. 变量命名小写，下划线分隔
   例如:

   ```c
   int num_times;
   char *pointer;
   ```

4. 宏定义全部大写，下划线分隔。宏函数遵守函数命名规则。
   例如:

   ```c
   #define PI                           3.14
   ```

5. typedef定义使用类型名加`_t`后缀（为使结构体或指针类型特征明显，对于结构体或指针请尽量不使用）。
   例如:

   ```c
   typedef unsigned int uint32_t;
   ```

6. 变量使用时请给出清晰的含义。对于指针当出现不需要写入时，请转换成变量使用。
   例如:

   ```c
   void func(int *args)
   {
        /* 转换成变量，防止意外覆写 */
        int value = *args;
        printf("%d\n", value);
   }
   ```

### 文件名

1. 文件名使用小写字母和下划线，不使用大写字母，配置文件以`config`结尾。
   例如:

   ```c
   foo_bar.c
   test_case.h
   mrconfig.h
   ```

### 宏

1. 宏定义全部大写，下划线分隔，用户可修改的宏配置需加`CFG`标识。
   例如:

   ```c
   #define MR_CFG_OBJECT_NAME_SIZE      15
   ```

2. 静态宏函数（无计算，无赋值等需要运行时才进行的操作）参考宏定义规则。动态宏函数（有任何需要运行过程中进行的操作），参考函数命名规则。

## 注释规范

1. 单行注释使用`/* */`，多行注释使用:

    ```c
    /**
     * 多行注释
     * 第二行
     */
    ```

2. 函数注释使用Doxygen风格:

   ```c
    /**
     * @brief This function register a device.
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

3. 结构体，宏等定义需添加注释：

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
   ```

4. 函数声明时需用注释将所有同类函数包含：

   ```c
   /**
    * @addtogroup Device description.
    * @{
    */
   int mr_dev_open(const char *name, uint32_t oflags);
   int mr_dev_close(int desc);
   ssize_t mr_dev_read(int desc, void *buf, size_t size);
   ssize_t mr_dev_write(int desc, const void *buf, size_t size);
   int mr_dev_ioctl(int desc, int cmd, void *args);
   /** @} */
    ```
   