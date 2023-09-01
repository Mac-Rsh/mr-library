# 状态机使用文档

## 应用场景

状态机通过有限个状态和触发条件之间的状态转移来描述一个系统的动态行为，通过预定义所有的可能状态和各状态之间的转换规则，可以有效地降低系统的复杂度，并且易于理解和验证系统的正确性。

## 接口介绍

### 查找状态机

```c
mr_fsm_t mr_fsm_find(const char *name);
```

| 参数      | 描述   |
|:--------|:-----|
| name    | 状态机名 |
| **返回**  |      |
| 状态机句柄   | 查找成功 |
| MR_NULL | 查找失败 |

### 添加状态机

```c
mr_err_t mr_fsm_add(mr_fsm_t fsm, const char *name, mr_fsm_table_t table, mr_size_t table_size);
```

| 参数         | 描述      |
|:-----------|:--------|
| fsm        | 状态机句柄   |
| name       | 状态机名    |
| table      | 状态机列表   |
| table_size | 状态机列表个数 |
| **返回**     |         |
| MR_ERR_OK  | 添加成功    |
| 错误码        | 添加失败    |

### 移除状态机

```c
mr_err_t mr_fsm_remove(mr_fsm_t fsm);
```

| 参数        | 描述    |
|:----------|:------|
| fsm       | 状态机句柄 |
| **返回**    |       |
| MR_ERR_OK | 移除成功  |
| 错误码       | 移除失败  |

### 状态机处理

```c
void mr_fsm_handle(mr_fsm_t fsm);
```

| 参数  | 描述    |
|:----|:------|
| fsm | 状态机句柄 |

### 发送状态机信号

```c
mr_err_t mr_fsm_signal(mr_fsm_t fsm, mr_uint32_t signal);
```

| 参数     | 描述        |
|:-------|:----------|
| fsm    | 状态机句柄     |
| index  | 状态机索引     |
| **返回** |           |
| 错误码    | 信号敏感函数返回值 |

### 切换状态机当前状态

```c
mr_err_t mr_fsm_shift(mr_fsm_t fsm, mr_uint32_t index);
```

| 参数          | 描述    |
|:------------|:------|
| fsm         | 状态机句柄 |
| index       | 状态机索引 |
| **返回**      |       |
| MR_ERR_OK   | 切换成功  |
| 错误码         | 切换失败  |

## 状态机列表介绍

```c
struct mr_fsm_table
```

| 参数        | 描述        |
|:----------|:----------|
| cb        | 状态回调函数    |
| args      | 状态回调函数参数  |
| signal    | 状态信号敏感函数  |

状态信号敏感函数将获取到事件（内部事件、外部事件）等发送的信号，此函数需判断信号是否为敏感信号，完成状态转移。

## 使用示例


```c
/* 定义状态索引 */
enum fsm_state_index
{
    Fsm_State_1 = 0,
    Fsm_State_2,
};

/* 定义信号 */
enum fsm_signal
{
    Fsm_Signal_1 = 0,
    Fsm_Signal_2,
};

/* 定义状态回调 */
mr_err_t fsm1_cb(mr_fsm_t fsm, void *args)
{
    printf("fsm1_cb\r\n");
    return MR_ERR_OK;
}

mr_err_t fsm2_cb(mr_fsm_t fsm, void *args)
{
    printf("fsm2_cb\r\n");
    return MR_ERR_OK;
}

/* 定义信号敏感函数 */
mr_err_t fsm1_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    switch (signal)
    {
        case Fsm_Signal_2:
        {
            mr_fsm_shift(fsm, Fsm_State_2);
            return MR_ERR_OK;
        }
        
        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

mr_err_t fsm2_signal(mr_fsm_t fsm, mr_uint32_t signal)
{
    switch (signal)
    {
        case Fsm_Signal_1:
        {
            mr_fsm_shift(fsm, Fsm_State_1);
            return MR_ERR_OK;
        }

        default:
            return -MR_ERR_UNSUPPORTED;
    }
}

/* 定义状态列表 */
struct mr_fsm_table fsm_table[] =
{
    {fsm1_cb, MR_NULL, fsm1_signal},
    {fsm2_cb, MR_NULL, fsm2_signal},
};

/* 定义状态机 */
struct mr_fsm fsm;

int main(void)
{
    /* 添加状态机 */
    mr_fsm_add(&fsm, "fsm", fsm_table, MR_ARRAY_SIZE(fsm_table));
    
    while (1)
    {
        /* 发送事件信号2，状态1->状态2 */
        mr_fsm_signal(&fsm, Fsm_Signal_2);
        
        /* 状态机处理 */
        mr_fsm_handle(&fsm);
        
        /* 发送事件信号1，状态2->状态1 */
        mr_fsm_signal(&fsm, Fsm_Signal_1);
    }
}
```