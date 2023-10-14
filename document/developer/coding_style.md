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

2. 变量命名小写，下划线分隔
   例如:

   ```c
   int num_times;
   char *pointer;
   ```

3. 宏定义全部大写，下划线分隔。宏函数遵守函数命名规则。
   例如:

   ```c
   #define PI                           3.14
   ```

4. typedef定义使用类型名加`_t`后缀， 结构体等默认`_t`为指针类型。
   例如:

   ```c
   typedef unsigned int mr_uint32_t;
   typedef struct mr_object *mr_object_t;
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
    * @brief This function change the type of the object.
    *
    * @param object The object to be changed.
    * @param type The type of the object.
    *
    * @return MR_ERR_OK on success, otherwise an error code.
    */
   mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_object_type type)
   {
      ...
   }
   ```

3. 结构体，宏等定义需添加注释：

    ```c
   /**
    * @struct Object
    */
   struct mr_object
   {
       char name[MR_CFG_OBJECT_NAME_SIZE];
       enum mr_object_type type;          
       struct mr_list list;               
   };
   
   /**
    * @def Null pointer
    */
   #define MR_NULL                      (void *)0
   ```

4. 函数声明时需用注释将所有同类函数包含：

   ```c
   /**
    * @addtogroup Object
    * @{
    */
   mr_object_container_t mr_object_container_find(enum mr_object_type type);
   mr_object_t mr_object_find(const char *name, enum mr_object_type type);
   mr_err_t mr_object_add(mr_object_t object, const char *name, enum mr_object_type type);
   mr_err_t mr_object_remove(mr_object_t object);
   mr_err_t mr_object_change_type(mr_object_t object, enum mr_object_type type);
   void mr_object_rename(mr_object_t object, char *name);
   /** @} */
   ```