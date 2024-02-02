# I2C Devices

[中文](i2c.md)

<!-- TOC -->
* [I2C Devices](#i2c-devices)
  * [Register I2C Device](#register-i2c-device)
  * [Open I2C Device](#open-i2c-device)
  * [Close I2C Device](#close-i2c-device)
  * [Control I2C Device](#control-i2c-device)
    * [Set/Get I2C Device Configuration](#setget-i2c-device-configuration)
    * [Set/Get Register Value](#setget-register-value)
    * [Set/Get Read Buffer Size](#setget-read-buffer-size)
    * [Clear Read Buffer](#clear-read-buffer)
    * [Get Read Buffer Data Size](#get-read-buffer-data-size)
    * [Set/Get Read Callback Function](#setget-read-callback-function)
  * [Read I2C Device Data](#read-i2c-device-data)
  * [Write I2C Device Data](#write-i2c-device-data)
  * [Usage Example:](#usage-example)
  * [Software I2C](#software-i2c)
    * [Register Software I2C Bus](#register-software-i2c-bus)
<!-- TOC -->

## Register I2C Device

```c
int mr_i2c_dev_register(struct mr_i2c_dev *i2c_dev, const char *path, int addr, int addr_bits);
```

| Parameter        | Description                  |
|------------------|------------------------------|
| i2c_dev          | I2C device structure pointer | 
| path             | Device path                  |
| addr             | Device address               |
| addr_bits        | Device address bits          |
| **Return Value** |                              |
| `=0`             | Registration succeeds        |
| `<0`             | Error code                   |

- `name`: The I2C device needs to bind to the specified I2C bus, and the name needs to add the bus name, such
  as: `i2cx/dev-name`, `i2c1/i2c10`.
- `addr`: Device address (the lowest bit is read/write bit, please pass the address shifted to the left).
  When serving as a host, the address is the peer's address. When serving as a slave, the address is its own address.
- `addr_bits`: Device address bits:
    - `MR_I2C_ADDR_BITS_7`: 7-bit address.
    - `MR_I2C_ADDR_BITS_10`: 10-bit address.

## Open I2C Device

```c
int mr_dev_open(const char *path, int flags);
``` 

| Parameter        | Description       |
|------------------|-------------------|
| path             | Device path       |
| flags            | Open device flags |
| **Return Value** |                   |
| `>=0`            | Device descriptor |
| `<0`             | Error code        |

- `path`: The I2C device path, such as: `i2cx/dev-name`,`i2c1/i2c10`.
- `flags`: Open device flags, support `MR_O_RDONLY`, `MR_O_WRONLY`, `MR_O_RDWR`.

Note: When using, the I2C device should be opened separately for different tasks according to the actual situation, and
the appropriate `flags` should be used for management and permission control to ensure that they will not interfere
with each other.

## Close I2C Device

```c
int mr_dev_close(int desc);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| **Return Value** |                    |
| `=0`             | Close successfully |
| `<0`             | Error code         |

## Control I2C Device

```c 
int mr_dev_ioctl(int desc, int cmd, void *args);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| cmd              | Command code       |
| args             | Command parameters |
| **Return Value** |                    |
| `=0`             | Setting succeeds   |
| `<0`             | Error code         |

- `cmd`: Command code, supports the following commands:
    - `MR_IOC_I2C_SET_CONFIG`: Set I2C device configuration.
    - `MR_IOC_I2C_SET_REG`: Set register value.
    - `MR_IOC_I2C_SET_RD_BUFSZ`: Set read buffer size.
    - `MR_IOC_I2C_CLR_RD_BUF`: Clear read buffer.
    - `MR_IOC_I2C_SET_RD_CALL`:Set read callback function.
    - `MR_IOC_I2C_GET_CONFIG`: Get I2C device configuration.
    - `MR_IOC_I2C_GET_REG`: Get register value.
    - `MR_IOC_I2C_GET_RD_BUFSZ`: Get read buffer size.
    - `MR_IOC_I2C_GET_RD_DATASZ`: Get read buffer data size.
    - `MR_IOC_I2C_GET_RD_CALL`:Get read callback function.

### Set/Get I2C Device Configuration

I2C device configuration:

- `baud_rate`: Baud rate.
- `host_slave`: Master/slave mode.
- `reg_bits`: Register bits.

```c
/* Set default configuration */
struct mr_i2c_config config = MR_I2C_CONFIG_DEFAULT;

/* Set I2C device configuration */  
mr_dev_ioctl(ds, MR_IOC_I2C_SET_CONFIG, &config);
/* Get I2C device configuration */
mr_dev_ioctl(ds, MR_IOC_I2C_GET_CONFIG, &config);
```

Independent of I2C interface:

```c
/* Set default configuration */
int config[] = {100000, 0, 8};

/* Set I2C device configuration */  
mr_dev_ioctl(ds, MR_IOC_SCFG, &config);
/* Get I2C device configuration */
mr_dev_ioctl(ds, MR_IOC_GCFG, &config);
```

Note:

- If not manually configured, the default configuration is:
    - Baud rate: `100000`
    - Master/slave mode: `MR_I2C_HOST`
    - Register bits: `MR_I2C_REG_BITS_8`
- When an I2C device on the I2C bus is configured to slave mode, it will continuously occupy the I2C bus. At this point,
  other I2C devices cannot perform read/write operations until the I2C device in slave mode is reconfigured to master
  mode.

### Set/Get Register Value

The register value will be written first (range: `0` ~ `INT32_MAX`) before reading and writing data.

```c
/* Set register value */
mr_dev_ioctl(ds, MR_IOC_I2C_SET_REG, MR_MAKE_LOCAL(int, 0x12));

/* Get register value */  
uint8_t reg;
mr_dev_ioctl(ds, MR_IOC_I2C_GET_REG, &reg);
```

Independent of I2C interface:

```c
/* Set register value */
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, 0x12));

/* Get register value */  
uint8_t reg;
mr_dev_ioctl(ds, MR_IOC_GPOS, &reg);
```

Note:

- The register value only takes effect in master mode.
- It is not effective if set to a negative number.
- The register value is not included in the read/write size.

### Set/Get Read Buffer Size

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_I2C_SET_RD_BUFSZ, &size);
/* Get read buffer size */
mr_dev_ioctl(ds, MR_IOC_I2C_GET_RD_BUFSZ, &size);
```

Independent of I2C interface:

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_SRBSZ, &size);
/* Get read buffer size */
mr_dev_ioctl(ds, MR_IOC_GRBSZ, &size);
```

Note: If not manually configured, it will use the size configured in `Kconfig` (default 32Byte). The read buffer is only
used in slave mode.

### Clear Read Buffer

```c
mr_dev_ioctl(ds, MR_IOC_I2C_CLR_RD_BUF, MR_NULL);
```

Independent of I2C interface:

```c
mr_dev_ioctl(ds, MR_IOC_CRBD, MR_NULL);
```

### Get Read Buffer Data Size

```c  
size_t size = 0;

/* Get read buffer data size */
mr_dev_ioctl(ds, MR_IOC_I2C_GET_RD_DATASZ, &size);
```

Independent of I2C interface:

```c  
size_t size = 0;

/* Get read buffer data size */
mr_dev_ioctl(ds, MR_IOC_GRBDSZ, &size);
```

### Set/Get Read Callback Function

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Get buffer data size */
    ssize_t data_size = *(ssize_t *)args;  
    
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_I2C_SET_RD_CALL, &fn);
/* Get read callback function */  
mr_dev_ioctl(ds, MR_IOC_I2C_GET_RD_CALL, &callback);
```

Independent of I2C interface:

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Get buffer data size */
    ssize_t data_size = *(ssize_t *)args;  
    
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_SRCB, &fn);
/* Get read callback function */  
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback);
```

## Read I2C Device Data

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| Parameter        | Description       |
|------------------|-------------------|
|                  |                   |
| desc             | Device descriptor |  
| buf              | Read data buffer  |
| count            | Read data size    |
| **Return Value** |                   |
| `>=0`            | Read data size    |
| `<0`             | Error code        |

```c
uint8_t buf[128];
/* Read I2C device data */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));
/* Whether the read succeeded */
if (size < 0)
{
    
}
```

Note:

- In host mode, data is read synchronously in polling mode. In slave mode,
  reads a specified amount of data from the read buffer (returns the size of the data actually read).
- When the register parameter is not negative, the write operation of the register value is inserted before the read
  operation.

## Write I2C Device Data

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
```

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| buf              | Write data buffer |
| count            | Write data size   |  
| **Return Value** |                   |
| `>=0`            | Write data size   |
| `<0`             | Error code        |

```c
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
/* Write data to I2C device */
ssize_t size = mr_dev_write(ds, buf, sizeof(buf));
/* Whether write succeeds */
if (size < 0)
{
    
} 
```

Note: When the register parameter is not negative, a register value write operation will be inserted before the write
operation.

## Usage Example:

```c
#include "include/mr_lib.h"

/* Define I2C devices */
struct mr_i2c_dev host_dev;
struct mr_i2c_dev slave_dev;

/* Define I2C device descriptors */  
int host_ds = -1;
int slave_ds = -1;

void i2c_init(void)
{
    int ret = MR_EOK;
    
    /* Register I2C-HOST device */
    ret = mr_i2c_dev_register(&host_dev, "i2c1/host", 0x62, MR_I2C_ADDR_BITS_7);
    if (ret < 0)
    {
        mr_printf("host i2c device register failed: %d\r\n", mr_strerror(ret));
        return;
    }
    
    /* Register I2C-SLAVE device */
    ret = mr_i2c_dev_register(&slave_dev, "i2c2/slave", 0x62, MR_I2C_ADDR_BITS_7);
    if (ret < 0)
    {
        mr_printf("slave i2c device register failed: %d\r\n", mr_strerror(ret));
        return;
    }
    
    /* Open I2C-HOST device */
    host_ds = mr_dev_open("i2c1/host", MR_O_RDWR);
    if (host_ds < 0)
    {
       mr_printf("host i2c device open failed: %d\r\n", mr_strerror(ret));
       return;
    }
    /* Set register value */
    mr_dev_ioctl(host_ds, MR_IOC_I2C_SET_REG, MR_MAKE_LOCAL(int, 0x12));
    
    /* Open I2C-SLAVE device */
    slave_ds = mr_dev_open("i2c2/slave", MR_O_RDWR);
    if (slave_ds < 0)
    {
       mr_printf("slave i2c device open failed: %d\r\n", mr_strerror(ret));
       return;
    }
    /* Set slave mode */
    struct mr_i2c_config config = MR_I2C_CONFIG_DEFAULT;
    config.host_slave = MR_I2C_SLAVE;
    ret = mr_dev_ioctl(slave_ds, MR_IOC_I2C_SET_CONFIG, &config);
    if (ret < 0)
    {
       mr_printf("slave i2c device set config failed: %d\r\n", mr_strerror(ret));
    }
}
/* Export to auto init (APP level) */
MR_INIT_APP_EXPORT(i2c_init);

int main(void) 
{
    /* Auto init (i2c_init function will be called automatically here) */
    mr_auto_init();
    
    /* Send test data */
    uint8_t wr_buf[] = {0x01, 0x02, 0x03, 0x04};
    mr_dev_write(host_ds, wr_buf, sizeof(wr_buf));
    
    /* Receive test data */
    uint8_t rd_buf[128];
    mr_dev_read(slave_ds, rd_buf, sizeof(rd_buf));
    
    /* Compare register value */
    if (rd_buf[0] == 0x12)
    {
       /* Compare data */
       if (memcmp(wr_buf, (rd_buf + 1), sizeof(wr_buf)) == 0)
       {
           mr_printf("i2c test success\r\n");
           return 0;
       }
    }
    
    while (1)
    {
        
    }
}
```

Connect I2C1 and I2C2 and perform sending and receiving test. I2C1 works as master and I2C2 works as slave. I2C2 will
compare the received data with the sent data.
Since the register value is set, the register value will be received first before the data writing.

## Software I2C

Note: Software I2C requires enabling PIN device.

### Register Software I2C Bus

```c
int mr_soft_i2c_bus_register(struct mr_soft_i2c_bus *soft_i2c_bus, const char *path, int scl_pin, int sda_pin);
```

| Parameter        | Description                        |
|------------------|------------------------------------|
| soft_i2c_bus     | Software I2C bus structure pointer |
| path             | Bus path                           |  
| scl_pin          | SCL pin number                     |
| sda_pin          | SDA pin number                     |
| **Return Value** |                                    |
| `=0`             | Registration succeeds              |
| `<0`             | Error code                         |

```c
/* Define SCL, SDA pin numbers for software I2C bus */
#define SCL_PIN_NUMBER                  0    
#define SDA_PIN_NUMBER                  1

/* Define software I2C bus */
struct mr_soft_i2c_bus soft_i2c_bus;

/* Register software I2C bus */
mr_soft_i2c_bus_register(&soft_i2c_bus, "i2c1", SCL_PIN_NUMBER, SDA_PIN_NUMBER);
```

After registration, the software I2C bus will simulate a hardware I2C.

Note: The software I2C bus only supports master mode.
