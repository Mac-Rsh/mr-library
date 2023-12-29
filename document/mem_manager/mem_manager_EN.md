# Dynamic Memory Management

[中文](mem_manager.md)

<!-- TOC -->
* [Dynamic Memory Management](#dynamic-memory-management)
  * [Main Functions:](#main-functions)
  * [Idea of Memory Merging](#idea-of-memory-merging)
    * [Front Merging Situation:](#front-merging-situation)
    * [Back Merging Situation:](#back-merging-situation)
    * [Non-merging Situation:](#non-merging-situation)
  * [Create Memory Array](#create-memory-array)
  * [Create Memory Management Structure](#create-memory-management-structure)
  * [Memory Management Initialization](#memory-management-initialization)
  * [Memory Allocation](#memory-allocation)
  * [Memory Release](#memory-release)
  * [Insert Memory Block](#insert-memory-block)
<!-- TOC -->

## Main Functions:

- Dynamic memory block allocation - Select an unallocated memory block according to the application's request to
  allocate.
- Memory block release and recycling - Release it and mark it as unallocated when the block is no longer needed.
- Records of allocated and unallocated blocks - Real-time tracking of status information of each block using data
  structures such as linked lists or arrays.
- Memory block merging - After releasing a block, check the status of adjacent blocks. If both are unallocated, merge
  them into a larger block to reduce fragmentation.

## Idea of Memory Merging

### Front Merging Situation:

```
(START) -> (Memory block A, size=5) -> (Memory block B, size=3) 

Insert memory block C(size=2), find C adjacent to A, and A is in front, then:

(START) -> (Memory block A+C, size=5+2=7) -> (Memory block B, size=3)
```

### Back Merging Situation:

```
(START) -> (Memory block A, size=5) -> (Memory block B, size=3) -> (Memory block C, size=2)

Insert memory block D(size=3), find D adjacent to B, and B is behind, then: 

(START) -> (Memory block A, size=5) -> (Memory block D+B, size=3+3=6) -> (Memory block C, size=2)
```

### Non-merging Situation:

```
(START) -> (Memory block A, size=5) -> (Memory block B, size=3) -> (Memory block C, size=2)

Insert memory block D(size=1), D between A and B and not connected, then:

(START) -> (Memory block A, size=5) -> (Memory block D, size=1) -> (Memory block B, size=3) -> (Memory block C, size=2)
```

## Create Memory Array

Create a static array to be used as the memory for memory management allocation.

```c
/* Define 4K space */
#define MR_CFG_HEAP_SIZE                (4 * 1024)
static uint8_t heap_mem[MR_CFG_HEAP_SIZE] = {0};
```

## Create Memory Management Structure

Define memory block, consisting of next memory block pointer, memory block size, memory allocation flag.

- Next memory block pointer: Used to implement linked memory block storage, indicating the address of the next memory
  block.
- Memory block size: Records the size of this memory block.
- Memory allocation flag: Use 1 bit to indicate the current status of the memory block, 0 means unallocated, 1 means
  allocated.

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

## Memory Management Initialization

Initialize the memory block for the entire memory block as a single memory block.

```c
int mr_heap_init(void)
{
    struct mr_heap_block *first_block = (struct mr_heap_block *)&heap_mem;

    /* Initialize memory block (consuming sizeof(struct mr_heap_block)) */
    first_block->next = MR_NULL;
    first_block->size = sizeof(heap_mem) - sizeof(struct mr_heap_block); 
    first_block->allocated = MR_HEAP_BLOCK_FREE;

    /* Initialize starting memory block, start memory management */
    heap_start.next = first_block;
    return MR_EOK;
}
```

## Memory Allocation

```c
void *mr_malloc(size_t size)
{
    struct mr_heap_block *block_prev = &heap_start;
    struct mr_heap_block *block = block_prev->next;
    void *memory = MR_NULL;
    size_t residual = 0;
    
    /* Check if the requested memory size is too small, too large,
       or if there is no memory available in the memory manager */
    if ((size == 0) || (size > (UINT32_MAX >> 1) || (block == MR_NULL)))
    {
        return MR_NULL;
    }

    /* Align the size to the next multiple of 4 bytes */
    size = MR_ALIGN4_UP(size);

    /* Find a memory block that can accommodate the requested size */
    while (block->size < size)
    {
        if (block->next == MR_NULL)
        {
            return MR_NULL;
        }
        /* Move to the next memory block */
        block_prev = block;
        block = block->next;
    }
    /* Disconnect the memory block from the linked list */
    block_prev->next = block->next;

    /* Create a new memory block and return the memory */
    memory = (void *)((uint8_t *)block) + sizeof(struct mr_heap_block);
    /* Calculate the residual memory size */
    residual = block->size - size;

    /* Set the allocated memory block */
    block->size = size;
    block->next = MR_NULL;
    block->allocated = MR_HEAP_BLOCK_ALLOCATED;

    /* Check if there is enough space to create a new memory block
       (MR_HEAP_BLOCK_MIN_SIZE shifted left by 2 is equivalent to 2 times),
       a new memory block is created if there is more than 2 times
       the size of the memory block */
    if (residual > MR_HEAP_BLOCK_MIN_SIZE)
    {
        struct mr_heap_block *new_block = (struct mr_heap_block *)(((uint8_t *)memory) + size);

        /* Set the new memory block */
        new_block->size = residual - sizeof(struct mr_heap_block);
        new_block->next = MR_NULL;
        new_block->allocated = MR_HEAP_BLOCK_FREE;

        /* Insert the new memory block into the linked list of memory blocks */
        heap_insert_block(new_block);
    }
    return memory;
}
```

## Memory Release

```c
void mr_free(void *memory)
{
    /* Check if the memory is valid */
    if (memory != MR_NULL)
    {
        struct mr_heap_block *block = (struct mr_heap_block *)((uint8_t *)memory - sizeof(struct mr_heap_block));
        
        /* Check if the memory block can be released */  
        if (block->allocated == MR_HEAP_BLOCK_ALLOCATED && block->size != 0)
        {
            block->allocated = MR_HEAP_BLOCK_FREE;
            
            /* Insert the memory block into the memory block linked list */
            heap_insert_block(block);
        }
    }
}
```

## Insert Memory Block

```c
void heap_insert_block(struct mr_heap_block *block)
{
    struct mr_heap_block *block_prev = &heap_start;
  
    /* Search for the previous memory block */
    while (((block_prev->next != MR_NULL) && ((uint32_t)block_prev->next < (uint32_t)block))) 
    {
        block_prev = block_prev->next;
    }
  
    if (block_prev->next != MR_NULL)
    {
        /* If the previous memory block is connected to the to-be-inserted memory block, merge forward */
        if ((void *)(((uint8_t *)block_prev) + sizeof(struct mr_heap_block) + block_prev->size) == (void *)block)  
        {
            block_prev->size += block->size + sizeof(struct mr_heap_block);
            block = block_prev;
        }
        
        /* If the to-be-inserted memory block is connected to the next memory block, merge backward */
        if ((void *)(((uint8_t *)block) + sizeof(struct mr_heap_block) + block->size) == (void *)block_prev->next)
        {
            block->size += block_prev->next->size + sizeof(struct mr_heap_block);
            block->next = block_prev->next->next;
            
            /* Determine if the current memory block is inserted*/
            if (block != block_prev)
            {
               block_prev->next = block;  
               block = block_prev;
            }
        }
    }
  
    /* If the memory block is not inserted, insert the memory block */
    if (block != block_prev) 
    {
       block->next = block_prev->next;
       block_prev->next = block;
    }
}
```
