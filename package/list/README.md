# LIST链表文档

### 初始化单链表

```c
void slist_init(slist_t list);
```

| 参数        | 描述     |
|:----------|:-------|
| list      | 单链表句柄  |

### 向单链表尾部追加节点

```c
void slist_append(slist_t list, slist_t node);
```

| 参数      | 描述     |
|:--------|:-------|
| list    | 单链表句柄  | 
| node    | 要追加的节点 |  

### 在单链表中节点后插入节点 

```c
void slist_insert_after(slist_t list, slist_t node);
```

| 参数     | 描述        |
|:-------|:----------|
| list   | 单链表句柄     | 
| node   | 要插入的节点    |

### 从单链表中移除节点 

```c
void slist_remove(slist_t list, slist_t node);
```

| 参数     | 描述        |
|:-------|:----------|
| list   | 单链表句柄     | 
| node   | 要移除的节点    |

### 获取单链表长度

```c
size_t slist_get_length(slist_t list);
```

| 参数     | 描述       |
|:-------|:---------| 
| list   | 单链表句柄    | 
| **返回** |          |
| size   | 单链表长度    |

### 获取单链表尾节点

```c
slist_t slist_get_tail(slist_t list);
```

| 参数     | 描述     | 
|:-------|:-------|
| list   | 单链表句柄  |
| **返回** |        |  
| slist  | 单链表尾节点 | 

### 检查单链表是否为空 

```c
int slist_is_empty(slist_t list);
```

| 参数     | 描述      | 
|:-------|:--------|
| list   | 单链表句柄   | 
| **返回** |         | 
| bool   | 单链表是否为空 |

### 初始化双链表

```c
void list_init(list_t list);
```

| 参数     | 描述       |
|:-------|:---------|
| list   | 双链表句柄    |

###  在双链表后插入节点

```c
void list_insert_after(list_t list, list_t node);
```

| 参数      | 描述       |
|:--------|:---------|
| list    | 双链表句柄    |  
| node    | 要插入的节点   | 

### 在双链表前插入节点

```c
void list_insert_before(list_t list, list_t node);
```

| 参数     | 描述       |
|:-------|:---------|
| list   | 双链表句柄    | 
| node   | 要插入的节点   | 

### 从双链表中移除节点 

```c
void list_remove(list_t node);
```

| 参数      | 描述       |
|:--------|:---------| 
| node    | 要移除的节点   |

### 获取双链表长度 

```c
size_t list_get_length(list_t list);
```

| 参数     | 描述       |
|:-------|:---------|
| list   | 双链表句柄    | 
| **返回** |          | 
| size   | 双链表长度    |

### 检查双链表是否为空

```c
int list_is_empty(list_t list);
```

| 参数     | 描述      | 
|:-------|:--------|
| list   | 双链表句柄   | 
| **返回** |         | 
| bool   | 双链表是否为空 |

### 通过子结构体指针获取结构体指针

```c
#define slist_container_of(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))
```

| 参数     | 描述              |
|:-------|:----------------|
| node   | 结构体中子结构体的指针     |
| type   | 结构体类型           |
| member | 子结构体在结构体中的位置    |
| **返回** |                 |
| struct | 结构体指针           |

#### 使用示例：

```c
/* 示例结构体 */
struct example 
{
    int a;
    
    struct list list;
};

int main(void)
{
    struct example e1;
    struct example *e2;
    struct list *list = &e1.list;
    
    /* 获取e1 */
    e2 = mr_container_of(list, struct example, list);
}
```

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。