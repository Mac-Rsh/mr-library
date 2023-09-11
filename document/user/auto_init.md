# 自动初始化使用指南

----------

**mr-library** 支持自动初始化，通过宏导出后，将在 `mr_auto_init` 函数处自动执行所有初始化。

自动初始化支持不同等级导出：

| 执行顺序 | 宏                       | 描述      |
|:-----|:------------------------|:--------|
| 1    | MR_INIT_DRIVER_EXPORT   | 驱动程序初始化 |
| 2    | MR_INIT_DEVICE_EXPORT   | 设备初始化   |
| 3    | MR_INIT_MODULE_EXPORT   | 组件初始化   |
| 4    | MR_INIT_APP_EXPORT      | 应用初始化   |

![流程图](https://gitee.com/MacRsh/mr-library/raw/develop/document/resource/auto_init_process.jpg)

自动初始化仅指定类型的函数：

```c
typedef int (*init_fn_t)(void);
```

使用前请使能 `mrconfig.h` 头文件中 `MR_CFG_AUTO_INIT` 宏开关。

## 使用示例：

```c
/* 定义初始化函数 */
int init_function(void)
{
    printf("Auto init\r\n");
}
MR_INIT_APP_EXPORT(init_function);

int main(void)
{
    /* 自动初始化 */
    mr_auto_init();
}
```

如果无法实现自动初始化，请修改链接文件（link.ld），在.text中加入以下内容：

```c
. = ALIGN(4);
_mr_auto_init_start = .;
KEEP(*(SORT(.auto_init*)))
_mr_auto_init_end = .;
```

示例：

```c
.text :
{
    . = ALIGN(4);
    *(.text)
    *(.text.*)
    *(.rodata)
    *(.rodata*)
    *(.gnu.linkonce.t.*)
    . = ALIGN(4);
    
    /* 添加以下内容 */
    . = ALIGN(4);
    _mr_auto_init_start = .;
    KEEP(*(SORT(.auto_init*)))
    _mr_auto_init_end = .;
} >FLASH AT>FLASH 
```