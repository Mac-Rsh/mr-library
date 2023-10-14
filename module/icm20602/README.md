# ICM20602 使用指南

ICM20602是一款低功耗六轴MEMS运动跟踪设备,集成了三轴陀螺仪和三轴加速度计。它通过SPI总线进行通信。

----------

## 准备

1. 在 `mrconfig.h` 中 `Module config` 下添加宏开关启用icm20602组件（需同步启用SPI驱动）。

```c
//<------------------------------------ Module config ------------------------------------>

#define MR_CFG_ICM20602                 MR_CFG_ENABLE
```

2. 在 `mrlib.h` 中引用头文件.

```c
#include "icm20602.h"
```

----------

## 查找ICM20602

```c
mr_icm20602_t mr_icm20602_find(const char *name);
```

| 参数       | 描述        |
|:---------|:----------|
| name     | ICM20602名 |
| **返回**   |           |
| ICM20602 | 查找成功      |
| MR_NULL  | 查找失败      |

----------

## 添加ICM20602

```c
mr_err_t mr_icm20602_add(mr_icm20602_t icm20602, const char *name, mr_uint16_t cs_number, const char *bus_name);
```

| 参数        | 描述         |
|:----------|:-----------|
| icm20602  | ICM20602   |
| name      | ICM20602名  |
| cs_number | SPI-CS引脚编号 |
| bus_name  | SPI总线名     |
| **返回**    |            |
| MR_ERR_OK | 添加成功       |
| 错误码       | 添加失败       |

----------

## 配置ICM20602

```c
mr_err_t mr_icm20602_config(mr_icm20602_t icm20602, mr_icm20602_config_t config);
```

| 参数        | 描述       |
|:----------|:---------|
| icm20602  | ICM20602 |
| config    | 参数       |
| **返回**    |          |
| MR_ERR_OK | 添加成功     |
| 错误码       | 添加失败     |

ICM20602参数原型如下：

```c
struct mr_icm20602_config
{
    mr_uint16_t acc_range;                                          /* 加速度计量程 */
    mr_uint16_t gyro_range;                                         /* 陀螺仪量程 */
};
```

```c
/* 加速度计量程 */
MR_ICM20602_ACC_RANGE_2G                                            /* 2G */
MR_ICM20602_ACC_RANGE_4G                                            /* 4G */
MR_ICM20602_ACC_RANGE_8G                                            /* 8G */
MR_ICM20602_ACC_RANGE_16G                                           /* 16G */

/* 陀螺仪量程 */
MR_ICM20602_GYRO_RANGE_250DPS                                       /* 250dps */
MR_ICM20602_GYRO_RANGE_500DPS                                       /* 500dps */
MR_ICM20602_GYRO_RANGE_1000DPS                                      /* 1000dps */
MR_ICM20602_GYRO_RANGE_2000DPS                                      /* 2000dps */
```

----------

```c
struct mr_icm20602_3_axis
{
    mr_int16_t x;                                                   /* X轴 */                                                   
    mr_int16_t y;                                                   /* Y轴 */
    mr_int16_t z;                                                   /* Z轴 */
};
```

## 读取加速度计数据

```c
struct mr_icm20602_3_axis mr_icm20602_read_acc_3_axis(mr_icm20602_t icm20602);
```

| 参数       | 描述       |
|:---------|:---------|
| icm20602 | ICM20602 |
| **返回**   |          |
| 加速度计数据   |          |

----------

## 读取陀螺仪数据

```c
struct mr_icm20602_3_axis mr_icm20602_read_gyro_3_axis(mr_icm20602_t icm20602);
```

| 参数       | 描述       |
|:---------|:---------|
| icm20602 | ICM20602 |
| **返回**   |          |
| 陀螺仪数据    |          |

----------

使用示例：

```c
/* 定义ICM20602 */
mr_icm20602_t icm20602;

int main(void)
{
    /* 添加ICM20602 */
    mr_icm20602_add(&icm20602, "icm20602", 15, "spi1");

    /* 配置ICM20602 */
    struct mr_icm20602_config config = {ICM20602_ACC_RANGE_8G, ICM20602_GYRO_RANGE_2000DPS};
    mr_icm20602_config(&icm20602, &config);

    /* 读取加速度计和陀螺仪数据 */
    struct mr_icm20602_3_axis acc = mr_icm20602_read_acc_3_axis(&icm20602); 
    struct mr_icm20602_3_axis gyro = mr_icm20602_read_gyro_3_axis(&icm20602);
}
```