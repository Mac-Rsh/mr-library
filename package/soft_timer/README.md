# 软件定时器文档

## 使用示例：

```c

/* 定义定时器服务器和客户端 */
struct soft_timer_server server;
struct soft_timer_client timer1, timer2, timer3;

int timer1_callback(soft_timer_client_t client, void *args)
{
    printf("timer1_callback\r\n");
    return MR_ERR_OK;
}

int timer2_callback(soft_timer_client_t client, void *args)
{
    printf("timer2_callback\r\n");
    return MR_ERR_OK;
}

int timer3_callback(soft_timer_client_t client, void *args)
{
    printf("timer3_callback\r\n");
    soft_timer_client_stop(client);
    return MR_ERR_OK;
}

int main(void)
{
    /* 初始化服务器 */
    soft_timer_server_init(&server);
    
    /* 添加客户端到服务器并启动 */
    soft_timer_client_add_then_start(&timer1, 5, timer1_callback, MR_NULL, &server);
    soft_timer_client_add_then_start(&timer2, 10, timer2_callback, MR_NULL, &server);
    soft_timer_client_add_then_start(&timer3, 15, timer3_callback, MR_NULL, &server);
        
    while (1)
    {
        /* 更新服务器时钟 */
        soft_timer_server_update(&server, 1);
        
        /* 服务器处理客户端超时（放在哪里，回调就将在哪里被调用） */
        soft_timer_server_handle(&server);
    }
}
```

 ----------

# 许可协议

遵循 **Apache License 2.0** 开源许可协议，可免费应用于商业产品，无需公开私有代码。