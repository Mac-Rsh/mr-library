`mr-library` `mr-soft-spi`
# 简介
**mr-soft-spi** 模块为 **mr-library** 项目下的可裁剪模块，以C语言编写，可快速移植到各种平台（主要以嵌入式mcu为主）。
**mr-soft-spi** 模块通过 **io** 模拟实现 **spi** 协议。


----------
# **mr-soft-spi** 特点
* 资源占用低。
- 简单易用。
- 通用性强，移植便捷。
* 松解耦，模块化。


----------

# 代码目录
**mr-soft-spi** 源代码目录结构如下图所示：
|名称|描述|
|:---|:---|
|mr_soft_spi.c|源代码|
|mr_soft_spi.h|头文件|
|mr_soft_spi_cfg.h|配置文件|


----------


# 需适配接口
|函数|描述|
|:---|:---|
|void (*set_clk)(mr_uint8_t level)|设置 spi-clk 电平|
|void (*set_mosi)(mr_uint8_t level)|设置 spi-mosi 电平|
|mr_uint8_t (*get_miso)(void)|读取 spi-miso 电平|
|void (*set_cs)(mr_uint8_t level)|设置 spi-cs 电平|


----------


# 可使用接口
|函数|描述|
|:---|:---|
|void mr_soft_spi_attach(struct mr_soft_spi *spi, struct mr_soft_spi_bus *spi_bus)|将 spi 设备挂载到 spi 总线|
|mr_err_t mr_soft_spi_transfer(struct mr_soft_spi *spi, struct mr_soft_spi_msg msg)|通过 spi 总线发送 spi 消息|


----------


# 配置选项
可通过修改配置文件( mr_soft_spi_cfg.h )实现功能修改
|配置|描述|
|:---|:---|
|USING_SOFT_SPI|使用 soft-spi 功能|
|U_SOFT_SPI_BUS_CS_VALID|spi-cs 引脚仅在使用时有效|
|U_SOFT_SPI_DEBUG|使用 spi-debug 检测参数功能|

可选配置

|配置|描述|
|:---|:---|
|mr_hw_interrupt_disable()|rtos 临界段关闭中断|
|mr_hw_interrupt_enable(EX)|rtos 临界段使能中断|


----------

# 使用流程
1. 复制 `模块（mr_soft_spi）` 文件夹到您的工程文件。
2. 按需修改 `配置文件（mr_soft_spi_cfg.h）`的配置.
3. 查看 `源代码（mr_soft_spi.c）` 文件中的 user specification，按说明适配接口。
4. 将 `头文件（mr_soft_spi.h）` 引用到您的工程。
5. 开始愉快的使用。


----------

# 使用示例
```
/* -------------------- 配置 -------------------- */

/* 创建一条 spi 总线 */
struct mr_soft_spi_bus spi_bus;

/* 适配 spi 总线接口 */
void set_clk(mr_uint8_t level)
{
<<<<<<< HEAD
=======

>>>>>>> 82a8986bea92f9e9a81c7b96ab835bf8ee9a111f
  GPIO_WriteBit(GPIOA,GPIO_Pin_0,level);
}

void set_mosi(mr_uint8_t level)
{
  GPIO_WriteBit(GPIOA,GPIO_Pin_1,level);
}

mr_uint8_t get_miso(void)
{
  return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2);
}

/* 配置 spi 总线 */
spi_bus.set_clk = set_clk;
spi_bus.set_mosi = set_mosi;
spi_bus.get_miso = get_miso;
spi_bus.lock = MR_UNLOCK;
spi_bus.owner = MR_NULL;

/* 创建一个 spi 设备 */
struct mr_soft_spi spi_device;

/* 适配 spi 设备接口 */
void set_cs(mr_uint8_t level)
{
  GPIO_WriteBit(GPIOA,GPIO_Pin_3,level);
}

/* 配置 spi 设备 */
<<<<<<< HEAD
spi_device.mode = SPI_MODE_0;       //SPI MODE 0
spi_device.cs_active = LEVEL_LOW;   //CS 引脚低电平有效
=======
spi_device.mode = SPI_MODE_0;       // SPI MODE 0
spi_device.cs_active = LEVEL_LOW;   // CS 引脚低电平有效
>>>>>>> 82a8986bea92f9e9a81c7b96ab835bf8ee9a111f
spi_device.set_cs = set_cs;

/* -------------------- 使用 -------------------- */

int main(void)
{
    /* 需要发送的数据 */
    mr_uint8_t buffer[10]={0,1,2,3,4,5,6,7,8,9};

    /* 初始化 gpio */
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 挂载 spi 设备到 spi 总线 */
    mr_soft_spi_attach(&spi_device,&spi_bus);
    
    /* 创建 spi 消息 */
    struct mr_soft_spi_msg spi_msg;
<<<<<<< HEAD
    spi_msg.send_buffer = buffer;   //发送数据地址
    spi_msg.send_size = 10;         //发送数据数量
    spi_msg.recv_buffer = MR_NULL;  //读取数据地址
    spi_msg.recv_size = 0;          //读取数据数量
    spi_msg.read_write = SPI_WR;    //只读模式
=======
    spi_msg.send_buffer = buffer;   // 发送数据地址
    spi_msg.send_size = 10;         // 发送数据数量
    spi_msg.recv_buffer = MR_NULL;  // 读取数据地址
    spi_msg.recv_size = 0;          // 读取数据数量
    spi_msg.read_write = SPI_WR;    // 只写模式
>>>>>>> 82a8986bea92f9e9a81c7b96ab835bf8ee9a111f
    
    /* 发送消息 */
    mr_soft_spi_transfer(&spi_device,spi_msg);
}

```

----------
# 贡献代码
如果您在使用 **mr-soft-spi** 模块中遇到了 bug 或是 您有自己的想法，欢迎您提交 pr 或者联系我(email)：macrsh@outlook.com

## 感谢各位对本仓库的贡献！