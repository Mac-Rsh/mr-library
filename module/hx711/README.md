# HX711 模块说明

HX711是一款24位ADC(模数转换器)芯片,主要用于数字量程的应用,常用于压力传感器、应力传感器、称重传感器等数字量程应用中。

HX711常与应力传感器或其他模拟传感器组合,采集传感器的模拟电平信号,通过内置的ADC转换为数字信号输出。它可以放大微小的模拟信号,提高采集精度。在电子秤、计量设备等应用中应用广泛。

## 注册HX711

```c
int hx711_register(struct hx711 *hx711, const char *name, int sck_pin, int dout_pin);
```

|    参数    |    描述    |
|:--------:|:--------:|
|  hx711   | HX711模块  |
|   name   | HX711模块名 |
| sck_pin  |  SCK管脚   |   
| dout_pin |  DOUT管脚  |

## 设置滤波位数

滤波位数：舍弃ADC低位数据，例如24bit ADC 设置4位滤波，ADC数据低4位将恒定为0。

```c
#define FILTER_BITS                     4

mr_dev_ioctl(desc, HX711_CTRL_SET_FILTER_BITS, mr_make_local(int, FILTER_BITS));
```

## 自校准

自校准零点，使用前请确保输入值稳定。

```c
mr_dev_ioctl(desc, HX711_CTRL_SET_SELF_CALIBRATION, MR_NULL);
```

## 读取数据

```c
uint32_t data = 0;

mr_dev_read(desc, &data, sizeof(data));
```

# 使用示例

```c
#define HX711_SCK_PIN                   0
#define HX711_DOUT_PIN                  1

#define HX711_FILTER_BITS               4

struct struct hx711 hx711;

int main(void)
{
    /* 注册hx711 */
    hx711_register(&hx711, "hx711", HX711_SCK_PIN, HX711_DOUT_PIN);
    
    /* 打开hx711 */
    int desc = mr_dev_open("hx711", MR_OFLAG_RDONLY);
    if (desc < 0)
    {
        mr_log("HX711 error %d", desc);
        return 0;
    }
    
    /* 设置滤波位数 */
    mr_dev_ioctl(desc, HX711_CTRL_SET_FILTER_BITS, mr_make_local(int, HX711_FILTER_BITS));
    
    /* 自校准 */
    mr_dev_ioctl(desc, HX711_CTRL_SET_SELF_CALIBRATION, MR_NULL);
    
    /* 读取数据 */
    uint32_t data = 0;
    mr_dev_read(desc, &data, sizeof(data));
}
```







