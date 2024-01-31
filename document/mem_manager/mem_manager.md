# 动态内存管理

[English](mem_manager_EN.md)

<!-- TOC -->
* [动态内存管理](#动态内存管理)
  * [主要功能：](#主要功能)
  * [内存合并的思路](#内存合并的思路)
    * [前合并的情况：](#前合并的情况)
    * [后合并的情况：](#后合并的情况)
    * [不合并的情况：](#不合并的情况)
  * [创建内存数组](#创建内存数组)
  * [创建内存管理结构体](#创建内存管理结构体)
  * [初始化内存管理](#初始化内存管理)
  * [分配内存](#分配内存)
  * [释放内存](#释放内存)
  * [插入内存块](#插入内存块)
<!-- TOC -->

## 主要功能：
- 内存块的动态分配 - 根据应用的请求,从未分配内存块中选择一个块进行分配。
- 内存块的释放回收 - 当块不再需要时,将其释放并标记为未分配状态。
- 已分配与未分配块记录 - 使用链表或数组等数据结构实时跟踪各状态块的信息。
- 内存块合并 - 在释放块后,检查相邻块状态,若均未分配则合并为一个大块减少碎片。

## 内存合并的思路

### 前合并的情况：

```
(START) -> (内存块A, size=5) -> (内存块B, size=3)

插入内存块C(size=2),发现C紧邻A，且A在前,则:

(START) -> (内存块A+C, size=5+2=7) -> (内存块B, size=3)
```

### 后合并的情况：
```
(START) -> (内存块A, size=5) -> (内存块B, size=3) -> (内存块C, size=2)

插入内存块D(size=3),发现D紧邻B，且B在后，则:

(START) -> (内存块A, size=5) -> (内存块D+B, size=3+3=6) -> (内存块C, size=2)
```
### 不合并的情况：

```
(START) -> (内存块A, size=5) -> (内存块B, size=3) -> (内存块C, size=2)

插入内存块D(size=1),D在A和B之间且都不相连,则:

(START) -> (内存块A, size=5) -> (内存块D, size=1) -> (内存块B, size=3) -> (内存块C, size=2)
```

## 创建内存数组

创建静态数组，用来作为内存管理分配的内存。

```c
/* 定义4K空间 */
#define MR_CFG_HEAP_SIZE                (4 * 1024)
static uint8_t heap_mem[MR_CFG_HEAP_SIZE] = {0};
```

## 创建内存管理结构体

定义内存块，由下一块内存块指针、内存块大小、内存分配标志位组成。

- 下一块内存块指针：用于实现链式内存块存储,表示下一个内存块的地址。
- 内存块大小：记录该内存块的大小。
- 内存分配标志位：使用1比特来表示内存块当前的状态,0表示未分配,1表示已分配。

```c
#define MR_HEAP_BLOCK_FREE              (0)
#define MR_HEAP_BLOCK_ALLOCATED         (1)
#define MR_HEAP_BLOCK_MIN_SIZE          (sizeof(struct mr_heap_block) << 1)

static struct mr_heap_block
{
    struct mr_heap_block *next;
    uint32_t size: 31;
    uint32_t allocated: 1;
} heap_start = {MR_NULL, 0, MR_HEAP_BLOCK_FREE};
```

## 初始化内存管理

为内存初始化内存块，将整块内存作为单个内存块。

```c
int mr_heap_init(void)
{
    struct mr_heap_block *first_block = (struct mr_heap_block *)&heap_mem;

    /* 初始化内存块（消耗一个 sizeof(struct mr_heap_block)） */
    first_block->next = MR_NULL;
    first_block->size = sizeof(heap_mem) - sizeof(struct mr_heap_block);
    first_block->allocated = MR_HEAP_BLOCK_FREE;

    /* 初始化起始内存块，启动内存管理 */
    heap_start.next = first_block;
    return MR_EOK;
}
```

## 分配内存

```c
 void *mr_malloc(size_t size)
{
    struct mr_heap_block *block_prev = &heap_start;
    struct mr_heap_block *block = block_prev->next;
    void *memory = MR_NULL;
    size_t residual = 0;
    
    /* 检查需要申请的内存过小、内存过大，以及内存管理器中还有无内存 */
    if ((size == 0) || (size > (UINT32_MAX >> 1) || (block == MR_NULL)))
    {
        return MR_NULL;
    }

    /* 字节向上做4对齐 */
    size = MR_ALIGN_UP(size, 4);

    /* 找到符合内存分配大小的内存块 */
    while (block->size < size)
    {
        if (block->next == MR_NULL)
        {
            return MR_NULL;
        }
        /* 脱离合理的内存块 */
        block_prev = block;
        block = block->next;
    }
    /* 断开内存块链接 */
    block_prev->next = block->next;

    /* 生成新的内存块并返回内存 */
    memory = (void *)((uint8_t *)block) + sizeof(struct mr_heap_block);
    /* 剩余内存大小*/
    residual = block->size - size;

    /* 设置被分配的内存块 */
    block->size = size;
    block->next = MR_NULL;
    block->allocated = MR_HEAP_BLOCK_ALLOCATED;

    /* 检测是否够空间生成新的内存块 （MR_HEAP_BLOCK_MIN_SIZE）左移两位等于2倍，需要有大于2个内存块大小，才生成新的内存块 */
    if (residual > MR_HEAP_BLOCK_MIN_SIZE)
    {
        struct mr_heap_block *new_block = (struct mr_heap_block *)(((uint8_t *)memory) + size);

        /* 设置新内存块 */
        new_block->size = residual - sizeof(struct mr_heap_block);
        new_block->next = MR_NULL;
        new_block->allocated = MR_HEAP_BLOCK_FREE;

        /* 将内存块插入到内存块链表中 */
        heap_insert_block(new_block);
    }
    return memory;
}
```

## 释放内存

```c
void mr_free(void *memory)
{
	/* 判断内存是否为有效 */
    if (memory != MR_NULL)
    {
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory - sizeof(struct mr_heap_block));

        /* 检查内存块是否可以释放 */
        if (block->allocated == MR_HEAP_BLOCK_ALLOCATED && block->size != 0)
        {
            block->allocated = MR_HEAP_BLOCK_FREE;

            /* 将内存块插入到内存块链表中 */
            heap_insert_block(block);
        }
    }
}
```

## 插入内存块

```c
void heap_insert_block(struct mr_heap_block *block)
{
    struct mr_heap_block *block_prev = &heap_start;

    /* 搜索前一内存块 */
    while (((block_prev->next != MR_NULL) && ((uint32_t)block_prev->next < (uint32_t)block)))
    {
        block_prev = block_prev->next;
    }

    if (block_prev->next != MR_NULL)
    {
        /* 如果前一内存块与需要插入的内存块相连则向前合并 */
        if ((void *)(((uint8_t *)block_prev) + sizeof(struct mr_heap_block) + block_prev->size) == (void *)block)
        {
            block_prev->size += block->size + sizeof(struct mr_heap_block);
            block = block_prev;
        }

        /* 如果需要插入的内存块与后一内存块于相连则向后合并 */
        if ((void *)(((uint8_t *)block) + sizeof(struct mr_heap_block) + block->size) == (void *)block_prev->next)
        {
            block->size += block_prev->next->size + sizeof(struct mr_heap_block);
            block->next = block_prev->next->next;
			
			/* 判断当前内存块是否插入*/
            if (block != block_prev)
            {
                block_prev->next = block;
                block = block_prev;
            }
        }
    }

    /* 若内存块未插入，则插入内存块 */
    if (block != block_prev)
    {
        block->next = block_prev->next;
        block_prev->next = block;
    }
}
```
