# 事件框架

 ----------

## 框架介绍

事件驱动是一种常用的软件设计模式，其将程序的执行方式从线性的代码流程转变为由外部事件的触发来驱动。
相比于传统的线性执行方式，事件驱动可以提高系统的响应速度和资源利用率，并且能够解耦各部分的处理代码，提高代码的可维护性和开发效率。
通过合理的事件驱动框架的设计，可以使得代码逻辑更加清晰，降低系统复杂度，增强可扩展性。

使用场景：

* 前后台系统（裸机模式）：可以在中断中通知事件发生，交由 `main` 函数中 `while` 循环处理事件。
* 实时操作系统（RTOS模式）：可以将多个相同优先级的任务以事件的形式集中加入到一个事件管理器中，然后由一个线程负责处理事件，这种方式可以减少创建多个线程的开销，提高系统的性能表现。

 ----------

## 框架组成

1. 事件源：用于产生事件。可以是外部实体，例如传感器、用户输入设备等，也可以是软件触发，如代码流程的转换等。
2. 事件管理器：用于管理事件，其被注册到内核事件管理器容器中。
3. 事件处理器：等待事件的发生，处理事件的函数或任务。
4. 事件队列：当有事件产生时，将被加入到事件队列中，按照先进先出的顺序由事件处理器执行。

 ----------

## 使用样例

```
/* 引用 mr-library 头文件 */
#include <mrlib.h>

/* 定义事件管理器、事件队列内存池(内存池大小 = 4byte * 同时发生的事件个数) */
struct mr_event_manager manager;
mr_uint8_t queue_pool[8];

/* 定义事件1 */
struct mr_event event1;

/* 定义事件ID */
#define EVENT1_ID 1234
#define EVENT2_ID 5678

/* 定义事件回调函数 */
mr_err_t event1_callback(mr_event_manager_t event_manager, void *args)
{
    /* 将参数转换为预期的数据类型 */
    mr_int32_t *event_data = (mr_int32_t *)args;

    /* 处理事件 */
    mr_printf("Event1 happened: %d\n", *event_data);

    return MR_ERR_OK;
}

mr_err_t event2_callback(mr_event_manager_t event_manager, void *args)
{
    /* 将参数转换为预期的数据类型 */
    mr_int32_t *event_data = (mr_int32_t *)args;

    /* 处理事件 */
    mr_printf("Event2 happened: %d\n", *event_data);

    return MR_ERR_OK;
}

int main(void)
{
    /* 添加事件管理器到内核容器 */
    mr_event_manager_add(&manager,
                         "manager",
                         MR_EVENT_MANAGER_TYPE_GENERAL,
                         queue_pool,
                         sizeof(queue_pool));
	
    /* 添加事件1到事件管理器 */
    mr_int32_t event1_data = 4321;
    mr_event_add(&manager, &event1, EVENT1_ID, event1_callback, &event1_data);
    
    /* 创建事件2到事件管理器 */
    mr_int32_t event2_data = 8765;
    mr_event_create(&manager, EVENT2_ID, event2_callback, &event2_data);

    /* 通知事件1、2发生， 事件2先于事件1发生 */
    mr_event_manager_notify(&manager, EVENT2_ID);
    mr_event_manager_notify(&manager, EVENT1_ID);

    /* 事件处理器处理已发生事件 */
    mr_event_manager_handler(&manager);

    return 0;
}
```

程序输出：

```
Event2 happened: 8765
Event1 happened: 4321
```