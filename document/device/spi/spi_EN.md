# SPI Devices

[中文](spi.md)

<!-- TOC -->
* [SPI Devices](#spi-devices)
  * [Register SPI Device](#register-spi-device)
  * [Open SPI Device](#open-spi-device)
  * [Close SPI Device](#close-spi-device)
  * [Control SPI Device](#control-spi-device)
    * [Set/Get SPI Device Configuration](#setget-spi-device-configuration)
    * [Set/Get Register Value](#setget-register-value)
    * [Set/Get Read Buffer Size](#setget-read-buffer-size)
    * [Clear Read Buffer](#clear-read-buffer)
    * [Get Read Buffer Data Size](#get-read-buffer-data-size)
    * [Set/Get Read Callback Function](#setget-read-callback-function)
    * [Full-Duplex Transmission](#full-duplex-transmission)
  * [Read SPI Device Data](#read-spi-device-data)
  * [Write SPI Device Data](#write-spi-device-data)
  * [Usage Example:](#usage-example)
<!-- TOC -->

Note: SPI-CS control requires enabling the PIN device.

## Register SPI Device

```c
int mr_spi_dev_register(struct mr_spi_dev *spi_dev, const char *path, int cs_pin, int cs_active);
```

| Parameter        | Description                  |
|------------------|------------------------------|
| spi_dev          | SPI device structure pointer | 
| path             | Device path                  |
| cs_pin           | Chip select pin number       |
| cs_active        | Chip select enable state     |
| **Return Value** |                              |
| `=0`             | Registration succeeded       |
| `<0`             | Error code                   |

- `path`: The SPI device needs to be bound to a specified SPI bus, and the path needs to add the bus name, such
  as: `spix/dev-name`, `spi1/spi10`.
- `cs_pin`: Chip select pin number (refer to PIN device documentation).
- `cs_active`: Chip select enable state:
    - `MR_SPI_CS_ACTIVE_LOW`: Low level enables.
    - `MR_SPI_CS_ACTIVE_HIGH`: High level enables.
    - `MR_SPI_CS_ACTIVE_HARDWARE`: Do not enable (Slave mode can only be used in this mode, and `cs_pin` will automatically be set to `-1`).

## Open SPI Device

```c
int mr_dev_open(const char *path, int flags);
```

| Parameter        | Description                  |
|------------------|------------------------------|
| path             | Device path                  |
| flags            | Flags for opening the device |
| **Return Value** |                              |
| `>=0`            | Device descriptor            |
| `<0`             | Error code                   |

- `path`: The SPI device is bound to the SPI bus, so the bus path needs to be added, such
  as: `spix/dev-name`, `spi1/spi10`.
- `flags`: Flags for opening the device, supporting `MR_O_RDONLY`, `MR_O_WRONLY`, `MR_O_RDWR`.

Note: When using, different tasks should open the SPI device separately according to actual situations and use
appropriate `flags` for management and permission control to ensure they do not interfere with each other.

## Close SPI Device

```c
int mr_dev_close(int desc);
```

| Parameter        | Description         |
|------------------|---------------------|
| desc             | Device descriptor   |
| **Return Value** |                     |
| `=0`             | Closed successfully |
| `<0`             | Error code          |

## Control SPI Device

```c 
int mr_dev_ioctl(int desc, int cmd, void *args);
```

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| cmd              | Command code      |
| args             | Command parameter |
| **Return Value** |                   |
| `=0`             | Setting succeeded |
| `<0`             | Error code        |

- `cmd`: Command code, supports:
    - `MR_IOC_SPI_SET_CONFIG`: Set SPI device configuration.
    - `MR_IOC_SPI_SET_REG`: Set register value.
    - `MR_IOC_SPI_SET_RD_BUFSZ`: Set read buffer size.
    - `MR_IOC_SPI_CLR_RD_BUF`: Clear read buffer.
    - `MR_IOC_SPI_SET_RD_CALL`: Set read callback function.
    - `MR_IOC_SPI_TRANSFER`: Full-duplex transmission.
    - `MR_IOC_SPI_GET_CONFIG`: Get SPI device configuration.
    - `MR_IOC_SPI_GET_REG`: Get register value.
    - `MR_IOC_SPI_GET_RD_BUFSZ`: Get read buffer size.
    - `MR_IOC_SPI_GET_RD_DATASZ`: Get read buffer data size.
    - `MR_IOC_SPI_GET_RD_CALL`: Get read callback function.

### Set/Get SPI Device Configuration

SPI device configuration:

- `baud_rate`: Baud rate.
- `host_slave`: Host/slave mode.
- `mode`: Mode.
- `data_bits`: Data bits.
- `bit_order`: Data transmission order.
- `reg_bits`: Register bits.

```c
/* Set default configuration */
struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;

/* Set SPI device configuration */
mr_dev_ioctl(ds, MR_IOC_SPI_SET_CONFIG, &config);

/* Get SPI device configuration */
mr_dev_ioctl(ds, MR_IOC_SPI_GET_CONFIG, &config);
```

Independent of SPI interface:

```c
/* Set default configuration */
int config[] = {3000000, 0, 0, 8, 1, 8};

/* Set SPI device configuration */
mr_dev_ioctl(ds, MR_IOC_SCFG, &config);

/* Get SPI device configuration */
mr_dev_ioctl(ds, MR_IOC_GCFG, &config);
```

Note:

- If not configured manually, the default configuration is:
    - Baud rate: `3000000`
    - Host/slave mode: `MR_SPI_HOST`
    - Mode: `MR_SPI_MODE_0`
    - Data bits: `MR_SPI_DATA_BITS_8`
    - Data transmission order: `MR_SPI_BIT_ORDER_MSB`
    - Register bits: `MR_SPI_REG_BITS_8`
- When an SPI device on the SPI bus is configured as slave mode, it will continuously occupy the SPI bus until the slave
  mode SPI device is reconfigured to master mode. Other SPI devices cannot perform read/write operations during this
  time.
- When hardware CS is forced to be used in slave mode, software IO will be restored to the default mode. For example, 
  to determine the use of slave mode at the beginning, set `cs_pin` to `-1` and `cs_active` to `MR_SPI_CS_ACTIVE_HARDWARE`.

### Set/Get Register Value

The register value will be written first (range: `0` ~ `INT32_MAX`) before reading and writing data.

```c
/* Set register value */
mr_dev_ioctl(ds, MR_IOC_SPI_SET_REG, MR_MAKE_LOCAL(int, 0x12));

/* Get register value */
uint8_t reg;
mr_dev_ioctl(ds, MR_IOC_SPI_GET_REG, &reg);
```

Independent of SPI interface:

```c
/* Set register value */
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, 0x12));

/* Get register value */
uint8_t reg;
mr_dev_ioctl(ds, MR_IOC_GPOS, &reg);
```

Note:

- The register value is only effective in master mode.
- Not effective if set to negative number.
- Register value not included in read/write size.

### Set/Get Read Buffer Size

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_SPI_SET_RD_BUFSZ, &size);

/* Get read buffer size */  
mr_dev_ioctl(ds, MR_IOC_SPI_GET_RD_BUFSZ, &size);
```

Independent of SPI interface:

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_SRBSZ, &size);

/* Get read buffer size */  
mr_dev_ioctl(ds, MR_IOC_GRBSZ, &size);
```

Note: If not set manually, it will use the size configured in `Kconfig` (default 32Byte). The read buffer is only used
in slave mode.

### Clear Read Buffer

```c
mr_dev_ioctl(ds, MR_IOC_SPI_CLR_RD_BUF, MR_NULL);
```

Independent of SPI interface:

```c
mr_dev_ioctl(ds, MR_IOC_CRBD, MR_NULL);
```

### Get Read Buffer Data Size

```c
size_t size = 0;

/* Get read buffer data size */
mr_dev_ioctl(ds, MR_IOC_SPI_GET_RD_DATASZ, &size);
```

Independent of SPI interface:

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
    /* Handle interrupt */
}
void (*callback)(int, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_SPI_SET_RD_CALL, &fn);

/* Get read callback function */
mr_dev_ioctl(ds, MR_IOC_SPI_GET_RD_CALL, &callback); 
```

Independent of SPI interface:

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Handle interrupt */
}
void (*callback)(int, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_SRCB, &fn);

/* Get read callback function */
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback); 
```

### Full-Duplex Transmission

```c
/* Define transfer structure */
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
struct mr_spi_transfer transfer =
{
    .rd_buf = buf,
    .wr_buf = buf,
    .size = sizeof(buf),
};

/* Full-duplex transmission */
ssize_t size = mr_dev_ioctl(ds, MR_IOC_SPI_TRANSFER, &transfer);

/* Check if transmission succeeded */
if (size < 0)
{

}
```

Independent of SPI interface:

```c
/* Define transfer structure */
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
struct
{
    void *rd_buf;
    const void *wr_buf;
    size_t size;
} transfer = {buf, buf, sizeof(buf)};

/* Full-duplex transmission */
ssize_t size = mr_dev_ioctl(ds, (0x01), &transfer);

/* Check if transmission succeeded */
if (size < 0)
{

}
```

## Read SPI Device Data

```c  
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| buf              | Read data buffer  |
| count            | Read data size    |
| **Return Value** |                   |
| `>=0`            | Size of data read |  
| `<0`             | Error code        |

```c
uint8_t buf[128];

/* Read SPI device data */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));

/* Check if read succeeded */
if (size < 0) 
{
    
}
```

Note:

- In master mode, polling will be used for synchronous reading. In slave mode, 
  reads a specified amount of data from the read buffer (returns the size of the data actually read).
- The register value writing will be inserted before the reading operation if the register parameter is not negative.

## Write SPI Device Data

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
```

| Parameter        | Description          |
|------------------|----------------------|
| desc             | Device descriptor    |
| buf              | Write data buffer    |
| count            | Write data size      |
| **Return Value** |                      |
| `>=0`            | Size of data written |
| `<0`             | Error code           |

```c
uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};

/* Write SPI device data */
ssize_t size = mr_dev_write(ds, buf, sizeof(buf));

/* Check if write succeeded */
if (size < 0)
{
    
}
```

Note: The register value writing will be inserted before the writing operation if the register parameter is not
negative.

## Usage Example:

```c
#include "include/mr_lib.h"

/* Define SPI devices */
struct mr_spi_dev host_dev;
struct mr_spi_dev slave_dev;

/* Define SPI device descriptors */  
int host_ds = -1;
int slave_ds = -1;

void spi_init(void)
{
    int ret = MR_EOK;
    
    /* Register HOST SPI device */
    ret = mr_spi_dev_register(&host_dev, "spi1/host", 0, MR_SPI_CS_ACTIVE_LOW);
    if (ret < 0) 
    {
        mr_printf("host spi device register failed: %d\r\n", mr_strerror(ret));
        return;
    }
    
    /* Register SLAVE SPI device */
    ret = mr_spi_dev_register(&slave_dev, "spi2/slave", 1, MR_SPI_CS_ACTIVE_LOW);
    if (ret < 0)
    {
        mr_printf("slave spi device register failed: %d\r\n", mr_strerror(ret));
        return;
    }
    
    /* Open HOST SPI device */
    host_ds = mr_dev_open("spi1/host", MR_O_RDWR);
    if (host_ds < 0)
    {
        mr_printf("host spi device open failed: %d\r\n", mr_strerror(ret));
        return; 
    }
    
    /* Set register value */
    mr_dev_ioctl(host_ds, MR_IOC_SPI_SET_REG, MR_MAKE_LOCAL(int, 0x12));
    
    /* Open SLAVE SPI device */
    slave_ds = mr_dev_open("spi2/slave", MR_O_RDWR);
    if (slave_ds < 0)
    {
        mr_printf("slave spi device open failed: %d\r\n", mr_strerror(ret));
        return;
    }
    
    /* Set slave mode */
    struct mr_spi_config config = MR_SPI_CONFIG_DEFAULT;
    config.host_slave = MR_SPI_SLAVE;
    ret = mr_dev_ioctl(slave_ds, MR_IOC_SPI_SET_CONFIG, &config);
    if (ret < 0)
    {
        mr_printf("slave spi device set config failed: %d\r\n", mr_strerror(ret));
    }
}
/* Export to auto init (APP level) */
MR_INIT_APP_EXPORT(spi_init);

int main(void) 
{
    /* Auto init (spi_init function will be automatically called here) */
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
          mr_printf("spi test success\r\n");
          return 0;
        }
    }
    
    while (1)
    {
    
    }
}
```

Connect SPI1 and SPI2, with pin 0 and 1 connected (refer to PIN device documentation), to perform send and receive test.
SPI1 acts as master and SPI2 acts as slave. SPI2 will compare the received data with the sent data.
Since the register value is set, the register value is received first before the written data.