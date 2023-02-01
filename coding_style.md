# **mr-library 编程规范**


----------


## **Change Logs**:
|Date|Author|Notes|
|:--|:--|:--|
|2023-01-28|MacRsh|first version|


----------


## **总体原则**

 - 清晰，易于维护、易于重构。
 - 简洁，易于理解，并且易于实现。
 - 风格统一，代码整体风格保持统一。


----------

## **1.目录名称**
目录名称如果无特殊的需求，请使用全小写的形式；目录名称应能够反映部分的意思。
## **2.文件名称**
文件名称如果无特殊的需求(如果是引用其他地方，可以保留相应的名称)，请使用全小写的形式。另外为了避免文件名重名的问题，一些地方请尽量不要使用通用化、使用频率高的名称。

 - 驱动源码文件：`drv_class.c` 命名方式，如：
```
drv_gpio.c
drv_spi.c
```
 - 设备源码文件：`dev_class.c` 命名方式，如：
```
dev_oled.c
dev_sensor.c
```
 - 模块源码文件：`mod_class.c` 命名方式，如：
```
mod_fatfs.c
mod_ulog.c
```
## **3.头文件定义**
C 语言头文件为了避免多次重复包含，需要定义一个符号。这个符号的定义形式请采用如下的风格：
```
#ifndef __FILE_H__
#define __FILE_H__

/* header file content */

#endif
```
## **4.头文件注释**
在每个源文件文件头上，应该包括相应的版权信息，Change Log 记录：
```
/*
 * Copyright (c), mr-library Development Team
 * 
 * SPDX-License-Identifier: Apache-2.0
 *  * Change Logs:
 * Date           Author       Notes
 * 2023-01-01     MacRsh       first version
 */
```
## **5.命名**

|类型|风格|形式|样例|
|:--|:--|:--|:--|
|函数|全小写，并下划线分割|`aaa_bbb` | `void gpio_init(void);`|
|局部变量，函数参数，宏参数，结构体成员，联合体成员|全小写，并下划线分割|`aaa_bbb`| `int temp_arg;`|
|全局变量|全小写，以'g'开头，并下划线分割|`g_aaa_bbb` | `int g_arg;`|
|宏，枚举值|全大写，并下划线分割|`AAA_BBB` | `NAME_MAX`|


 - typedef 修饰后的类型请在结尾加上"_t"，如：
```
typedef struct mr_object *mr_object_t;

```
 - 与内核有关同时对外暴露的函数: `mr_function_name` 命名方式，如：
```
void mr_kernel_init(void);

```
 - 不对外暴露的函数命名为 `_function_name` 命名方式,同时使用 `static` 修饰，如：
```
 static void _mr_kernel_init(void);
 
```
 - 与实际硬件有关，用于注册设备的函数: `mr_hw_function_name` 命名方式，如：
```
void mr_hw_gpio_init(void);

```
## **6.注释**
 请使用 **英文** 做为注释，在不同编码格式切换中不会出现注释乱码等情况，同时可提高编写代码效率。
### **代码注释**
 代码因高效，并且 **通俗易懂**，尽可能使用代码来解释功能，而不是长篇的注释。当出现功能关键点或算法等，应主动添加注释辅助阅读。注释只能添加到 **上方或右方**。
 
```
/* 你的英文注释 */
mr_kernel_init(); /* 你的英文注释 */

```
### **函数注释**
注释以 /** 开头，以 */ 结尾，中间写入函数注释，组成元素如下，每个元素描述之间空一行，且首列对齐：

- @brief + 简述函数作用。在描述中，着重说明该函数的作用，每句话首字母大写，句尾加英文句号。
- @note + 函数说明。在上述简述中未能体现到的函数功能或作用的一些点，可以做解释说明，每句话首字母大写，句尾加英文句号。
- @see + 相关 API 罗列。若有与当前函数相关度较高的 API，可以进行列举。
- @param + 以参数为主语 + be 动词 + 描述，说明参数的意义或来源。
- @return + 枚举返回值 + 返回值的意思，若返回值为数据，则直接介绍数据的功能。
- @warning + 函数使用注意要点。在函数使用时，描述需要注意的事项，如使用环境、使用方式等。每句话首字母大写，句尾加英文句号。

如：
```
/**
 * @brief    The function will initialize a object.
 *  * @note     For the object, its memory space is allocated by the compiler during compiling,
 *           and shall placed on the read-write data segment or on the uninitialized data segment.
 *           The object will add it to object system.
 *  * @see      mr_object_get_information()
 *  * @param    object is a pointer to the object to initialize. 
 *  * @param    type mr_object_class_type enum the type.
 *  * @param    name is a pointer to the name that given to the object.
 *  * @return   Return the operation status. When the return value is MR_EOK, the initialization is successful.
 *           If the return value is any other values, it represents the initialization failed.
 *  * @warning  Null.
 */
mr_err_t mr_object_init(struct mr_object *object, enum mr_object_class_type type,const char *name)
{
   ... 
}
```
## **7.缩进及分行**
缩进请采用 4 个空格的方式。如果没有什么特殊意义，请在 "{" 后进行分行，并在下一行都采用缩进的方式，如：
```
if (condition)
{
    /* others */
}
```

 * switch-case 语句采用 case 语句与 switch 对齐的方式，如：
```
switch (value)
{
case value1:
    break;
}
```
## **8.括号与空格**
请将每个每个大括号单独占用一行，而不是跟在语句的后面。括号前留出一个空格(if、for、while、switch)。运算表达式中，运算符与字符串间留一个空格。如：
```
if (x <= y)
{
    /* others */
}

for (index = 0; index < MAX_NUMBER; index ++)
{
    /* others */
}
```

 - 单一if语句可省去大括号，如：
```
/* only if */
if (x <= y)
    x = y;
    
/* if-else */
if (x <= y)
    x = y;
else
    y = x;
```
## **9.排版与格式**
一行只写一条语句。一条语句不能过长，如不能缩短语句则需要分行写。换行时将操作符留在行末，新行进行同类对齐或缩进一层。如：
```
/* 假设下面第一行不满足行宽要求 */
if (value > MIN &&  /* 换行后，布尔操作符放在行末 */
    value < MAX)    /* 与(&&)操作符的两个操作数同类对齐 */
{  
    /* others */
}
```

```
/* 假设下面的函数调用不满足行宽要求，需要换行  */
mr_err_t result = function_name(param_name1,
                                param_name2,
                                param_name3); /* 保持与上方参数对齐 */

```

- 指针类型 '*'应靠右跟随变量或函数名，如：
```
int *p;  /* 右跟随变量，和左边的类型隔了1个空格 */

```


 
