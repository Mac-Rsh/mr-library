# SERIAL Device

[中文](serial.md)

<!-- TOC -->
* [SERIAL Device](#serial-device)
  * [Open SERIAL Device](#open-serial-device)
  * [Close SERIAL Device](#close-serial-device)
  * [Control SERIAL Device](#control-serial-device)
    * [Set/Get SERIAL Device Configuration](#setget-serial-device-configuration)
    * [Set/Get Read/Write Buffer Size](#setget-readwrite-buffer-size)
    * [Clear Read/Write Buffer](#clear-readwrite-buffer)
    * [Get Read/Write Buffer Data Size](#get-readwrite-buffer-data-size)
    * [Set/Get Read/Write Callback Function](#setget-readwrite-callback-function)
  * [Read Data from SERIAL Device](#read-data-from-serial-device)
  * [Write Data to SERIAL Device](#write-data-to-serial-device)
  * [Example](#example)
<!-- TOC -->

## Open SERIAL Device

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

- `path`: The SERIAL device name is usually `serialx`, such as `serial1`, `serial2`, `serial3`.
- `flags`: Flags for opening the device,
  supports `MR_O_RDONLY`, `MR_O_WRONLY`, `MR_O_RDWR`, `MR_O_NONBLOCK`.

Note: When using it, different tasks should open the SERIAL device separately according to actual situations, and use
appropriate `flags` for management and permission control to ensure they do not interfere with each other.

## Close SERIAL Device

```c 
int mr_dev_close(int desc);
```

| Parameter        | Description         |
|------------------|---------------------|
| desc             | Device descriptor   |
| **Return Value** |                     |
| `=0`             | Closed successfully |  
| `<0`             | Error code          |

## Control SERIAL Device

```c
int mr_dev_ioctl(int desc, int cmd, void *args);
``` 

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| cmd              | Command code      |
| args             | Command parameter |
| **Return Value** |                   |
| `=0`             | Set successfully  |
| `<0`             | Error code        |

- `cmd`: Command code, supports the following commands:
    - `MR_IOC_SERIAL_SET_CONFIG`: Set SERIAL device configuration.
    - `MR_IOC_SERIAL_SET_RD_BUFSZ`: Set read buffer size.
    - `MR_IOC_SERIAL_SET_WR_BUFSZ`: Set write buffer size.
    - `MR_IOC_SERIAL_CLR_RD_BUF`: Clear read buffer.
    - `MR_IOC_SERIAL_CLR_WR_BUF`: Clear write buffer.
    - `MR_IOC_SERIAL_SET_RD_CALL`: Set read callback function.
    - `MR_IOC_SERIAL_SET_WR_CALL`: Set write callback function.
    - `MR_IOC_SERIAL_GET_CONFIG`: Get SERIAL device configuration.
    - `MR_IOC_SERIAL_GET_RD_BUFSZ`: Get read buffer size.
    - `MR_IOC_SERIAL_GET_WR_BUFSZ`: Get write buffer size.
    - `MR_IOC_SERIAL_GET_RD_DATASZ`: Get read buffer data size.
    - `MR_IOC_SERIAL_GET_WR_DATASZ`: Get write buffer data size.
    - `MR_IOC_SERIAL_GET_RD_CALL`: Get read callback function.
    - `MR_IOC_SERIAL_GET_WR_CALL`: Get write callback function.

### Set/Get SERIAL Device Configuration

SERIAL device configuration:

- `baud_rate`: Baud rate
- `data_bits`: Data bits
- `stop_bits`: Stop bits
- `parity`: Parity check
- `bit_order`: Data transmission order
- `polarity`: Polarity inversion

```c
/* Set default configuration */
struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;

/* Set SERIAL device configuration */
mr_dev_ioctl(ds, MR_IOC_SERIAL_SET_CONFIG, &config);

/* Get SERIAL device configuration */
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_CONFIG, &config);
```

Independent of SERIAL interface:

```c
/* Set default configuration */
int config[] = {115200, 8, 1, 0, 0, 0};

/* Set SERIAL device configuration */
mr_dev_ioctl(ds, MR_IOC_SCFG, &config);

/* Get SERIAL device configuration */
mr_dev_ioctl(ds, MR_IOC_GCFG, &config);
```

Note: If not configured manually, the default configuration is:

- Baud rate: `115200`
- Data bits: `MR_SERIAL_DATA_BITS_8`
- Stop bits: `MR_SERIAL_STOP_BITS_1`
- Parity check: `MR_SERIAL_PARITY_NONE`
- Data transmission order: `MR_SERIAL_BIT_ORDER_LSB`
- Polarity inversion: `MR_SERIAL_POLARITY_NORMAL`

### Set/Get Read/Write Buffer Size

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_SERIAL_SET_RD_BUFSZ, &size);

/* Get read buffer size */  
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_RD_BUFSZ, &size);

/* Set write buffer size */
mr_dev_ioctl(ds, MR_IOC_SERIAL_SET_WR_BUFSZ, &size);

/* Get write buffer size */
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_WR_BUFSZ, &size);
```

Independent of SERIAL interface:

```c
size_t size = 256;

/* Set read buffer size */
mr_dev_ioctl(ds, MR_IOC_SRBSZ, &size);

/* Get read buffer size */  
mr_dev_ioctl(ds, MR_IOC_GRBSZ, &size);

/* Set write buffer size */
mr_dev_ioctl(ds, MR_IOC_SWBSZ, &size);

/* Get write buffer size */
mr_dev_ioctl(ds, MR_IOC_GWBSZ, &size);
```

Note: If not configured manually, the size configured in `Kconfig` will be used (default is 32Byte).

### Clear Read/Write Buffer

```c
mr_dev_ioctl(ds, MR_IOC_SERIAL_CLR_RD_BUF, MR_NULL);
mr_dev_ioctl(ds, MR_IOC_SERIAL_CLR_WR_BUF, MR_NULL);
```

Independent of SERIAL interface:

```c
mr_dev_ioctl(ds, MR_IOC_CRBD, MR_NULL);
mr_dev_ioctl(ds, MR_IOC_CWBD, MR_NULL);
```

### Get Read/Write Buffer Data Size

```c
size_t size = 0;

/* Get read buffer data size */
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_RD_DATASZ, &size);

/* Get write buffer data size */  
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_WR_DATASZ, &size);
```

Independent of SERIAL interface:

```c
size_t size = 0;

/* Get read buffer data size */
mr_dev_ioctl(ds, MR_IOC_GRBDSZ, &size);

/* Get write buffer data size */  
mr_dev_ioctl(ds, MR_IOC_GWBDSZ, &size);
```

### Set/Get Read/Write Callback Function

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_SERIAL_SET_RD_CALL, &fn);

/* Get read callback function */
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_RD_CALL, &callback);

/* Set write callback function */
mr_dev_ioctl(ds, MR_IOC_SERIAL_SET_WR_CALL, &fn);

/* Get write callback function */
mr_dev_ioctl(ds, MR_IOC_SERIAL_GET_WR_CALL, &callback);
```

Independent of SERIAL interface:

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set read callback function */
mr_dev_ioctl(ds, MR_IOC_SRCB, &fn);

/* Get read callback function */
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback);

/* Set write callback function */
mr_dev_ioctl(ds, MR_IOC_SWCB, &fn);

/* Get write callback function */
mr_dev_ioctl(ds, MR_IOC_GWCB, &callback);
```

## Read Data from SERIAL Device

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| Parameter        | Description          |
|------------------|----------------------|
| desc             | Device descriptor    |  
| buf              | Read data buffer     |
| count            | Read data size       |
| **Return Value** |                      |
| `>=0`            | Number of bytes read |
| `<0`             | Error code           |

```c
char buf[128];
/* Read data from SERIAL device */
ssize_t size = mr_dev_read(ds, buf, sizeof(buf));
/* Check if read successfully */  
if (size < 0) 
{
    
}
```

Note: When no read buffer is set, it will use polling mode for synchronous reading. When a read buffer is set, it will read the specified amount of data from the read buffer (return the actual
number of bytes read).

## Write Data to SERIAL Device

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);
```

| Parameter        | Description             |
|------------------|-------------------------|
| desc             | Device descriptor       |
| buf              | Write data buffer       |
| count            | Write data size         |
| **Return Value** |                         |
| `>=0`            | Number of bytes written |
| `<0`             | Error code              |

```c
char buf[] = {"hello world"};
/* Write data to SERIAL device */
ssize_t size = mr_dev_write(ds, buf, sizeof(buf));
/* Check if write successfully */
if (size < 0) 
{
    
}
```

Note: When opened with `MR_O_NONBLOCK`, the data is written to the write buffer (returns the size of the data actually written), 
the data is sent asynchronously by interrupt or DMA, and the write callback function is triggered when the sending is complete.
When data is sent asynchronously, the write lock is automatically locked. In this case,
data cannot be written synchronously until the asynchronous transmission is complete.

## Example

```c
#include "include/mr_lib.h"

/* Define serial device descriptor */
int serial_ds = -1;

void serial_init(void) 
{
    /* Initialize serial */
    serial_ds = mr_dev_open("serial1", MR_O_RDWR);  
    if (serial_ds < 0)
    {
        mr_printf("serial open failed: %s\r\n", mr_strerror(serial_ds));
        return;
    }
    
    /* Set serial configuration */
    struct mr_serial_config config = MR_SERIAL_CONFIG_DEFAULT;  
    int ret = mr_dev_ioctl(serial_ds, MR_IOC_SERIAL_SET_CONFIG, &config);
    if (ret < 0) 
    {
        mr_printf("serial set config failed: %s\r\n", mr_strerror(ret));
    }
}
/* Export to auto initialization (APP level) */
MR_INIT_APP_EXPORT(serial_init);

int main(void) 
{
    /* Auto initialization (serial_init function will be automatically called here) */
    mr_auto_init();
    
    while (1) 
    {
        /* Loopback test */
        char buf[64];
        ssize_t ret = mr_dev_read(serial_ds, buf, sizeof(buf));
        mr_dev_write(serial_ds, buf, ret); 
    }
}
```

Connect the serial port 1. Loopback test can be performed by sending data in the serial port software and
seeing the received data displayed in the serial port software.
