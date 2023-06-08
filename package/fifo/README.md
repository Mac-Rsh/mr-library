# FIFO队列文档

### 初始化FIFO缓存

```c
void fifo_init(fifo_t fifo, void *pool, size_t pool_size);
```

| 参数        | 描述     |
|:----------|:-------|
| fifo      | FIFO句柄 |
| pool      | 内存池    |
| pool_size | 内存池大小  | 

### 重置FIFO缓存

```c
void fifo_reset(fifo_t fifo);
```

| 参数   | 描述       |
|:-----|:---------|
| fifo | FIFO句柄   |

### 获取FIFO缓存的数据大小

```c
size_t fifo_get_data_size(fifo_t fifo);
```

| 参数        | 描述      |
|:----------|:--------|
| fifo      | FIFO句柄  |  
| **返回**    |         |
| size      | 缓存的数据大小 |

### 从FIFO缓存读取数据

```c
size_t fifo_read(fifo_t fifo, void *buffer, size_t size);
```

| 参数      | 描述               |
|:--------|:-----------------|
| fifo    | FIFO句柄           |
| buffer  | 要从FIFO缓存读取的数据缓冲区 |
| size    | 读取大小             |
| **返回**  |                  |
| size    | 实际读取的数据大小        |

### 向FIFO缓存写入数据 

```c
size_t fifo_write(fifo_t fifo, const void *buffer, size_t size);
```

| 参数        | 描述              | 
|:----------|:----------------|
| fifo      | FIFO句柄          |
| buffer    | 要写入FIFO缓存的数据缓冲区 |
| size      | 写入大小            |
| **返回**    |                 |
| size      | 实际写入的数据大小       | 

### 强制向FIFO缓存写入数据 

```c
size_t fifo_write_force(fifo_t fifo, const void *buffer, size_t size);
```

| 参数      | 描述              | 
|:--------|:----------------|
| fifo    | FIFO句柄          |
| buffer  | 要写入FIFO缓存的数据缓冲区 |
| size    | 写入大小            |
| **返回**  |                 | 
| size    | 实际写入的数据大小       |   

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。