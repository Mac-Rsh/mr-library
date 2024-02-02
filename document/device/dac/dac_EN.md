# DAC Device

[中文](dac.md)

<!-- TOC -->
* [DAC Device](#dac-device)
  * [Open DAC Device](#open-dac-device)
  * [Close DAC Device](#close-dac-device)
  * [Control DAC Device](#control-dac-device)
    * [Set/Get Channel Number](#setget-channel-number)
    * [Set/Get Channel Configure](#setget-channel-configure)
  * [Write DAC Device Channel Value](#write-dac-device-channel-value)
  * [Usage Example:](#usage-example)
<!-- TOC -->

## Open DAC Device

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

- `path`: DAC device path usually is: `dacx`, `dac1`, `dac2`.
- `flags`: Open device flags, support `MR_O_WRONLY`.

Note: When using, the DAC device should be opened separately for different tasks according to actual situations, and the
appropriate `flags` should be used for management and permission control to ensure they will not interfere with each
other.

## Close DAC Device

```c
int mr_dev_close(int desc); 
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| **Return Value** |                    |
| `=0`             | Close successfully |   
| `<0`             | Error code         |

Note: When closing the device, all channels will be automatically restored to the default state. The channel needs to be
reconfigured after reopening(This feature can be turned off).

## Control DAC Device

```c
int mr_dev_ioctl(int desc, int cmd, void *args);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| cmd              | Command code       |
| args             | Command parameters |
| **Return Value** |                    |
| `=0`             | Set successfully   |
| `<0`             | Error code         |

- `cmd`: Command code, supports the following commands:
    - `MR_IOC_DAC_SET_CHANNEL`: Set channel number.
    - `MR_IOC_DAC_SET_CHANNEL_CONFIG`: Set channel configure.
    - `MR_IOC_DAC_GET_CHANNEL`: Get channel number.
    - `MR_IOC_DAC_GET_CHANNEL_CONFIG`: Get channel configure.

### Set/Get Channel Number

Channel number range: `0` ~ `31`.

```c
/* Define channel number */
#define CHANNEL_NUMBER                  5

/* Set channel number */   
mr_dev_ioctl(ds, MR_IOC_DAC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;  
mr_dev_ioctl(ds, MR_IOC_DAC_GET_CHANNEL, &number);
```

Independent of DAC interface:

```c
/* Define channel number */
#define CHANNEL_NUMBER                  5

/* Set channel number */   
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;  
mr_dev_ioctl(ds, MR_IOC_GPOS, &number);
```

### Set/Get Channel Configure

Channel configure:

- `MR_DISABLE`: Disable channel.
- `MR_ENABLE`: Enable channel.

```c
/* Set channel status */
mr_dev_ioctl(ds, MR_IOC_DAC_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(int, MR_ENABLE));

/* Get channel status */   
int state;
mr_dev_ioctl(ds, MR_IOC_DAC_GET_CHANNEL_CONFIG, &state);
```

Independent of DAC interface:

```c
/* Set channel status */
mr_dev_ioctl(ds, MR_IOC_SCFG, MR_MAKE_LOCAL(int, MR_ENABLE));

/* Get channel status */   
int state;
mr_dev_ioctl(ds, MR_IOC_GCFG, &state);
```

## Write DAC Device Channel Value

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
/* Write channel value */
uint32_t data = 2048;
int ret = mr_dev_write(ds, &data, sizeof(data));
/* Check if write successfully */
if (ret != sizeof(data))
{
    return ret;
}
```

Note: The write data is the raw DAC data. The minimum write unit is `uint32_t`, that is, 4 bytes each time.

## Usage Example:

```c
#include "include/mr_lib.h"

/* Define channel number */
#define CHANNEL_NUMBER                  1

/* Define DAC device descriptor */   
int dac_ds = -1;

int dac_init(void)
{
   int ret = MR_EOK;

   /* Initialize DAC */
   dac_ds = mr_dev_open("dac1", MR_O_WRONLY);
   if (dac_ds < 0)
   {
       mr_printf("DAC1 open failed: %s\r\n", mr_strerror(dac_ds));  
       return dac_ds;
   }
   /* Print DAC descriptor */
   mr_printf("DAC1 desc: %d\r\n", dac_ds);
   /* Set to channel 1 */
   mr_dev_ioctl(dac_ds, MR_IOC_DAC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
   /* Set channel enable */
   ret = mr_dev_ioctl(dac_ds, MR_IOC_DAC_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(int, MR_ENABLE));
   if (ret < 0)
   {
       mr_printf("Channel%d enable failed: %s\r\n", CHANNEL_NUMBER, mr_strerror(ret));
       return ret;
   }
   return MR_EOK;
}
/* Export to automatic initialization (APP level) */
MR_INIT_APP_EXPORT(dac_init);

/* Define DAC data maximum value */
#define DAC_DATA_MAX                    4000

int main(void) 
{
   /* Automatic initialization (dac_init function will be automatically called here) */
   mr_auto_init();
    
   while(1)
   {
       uint32_t data = 0;
       for (data = 0; data <= DAC_DATA_MAX; data += 500)
       {
           int ret = mr_dev_write(dac_ds, &data, sizeof(data));
           if (ret != sizeof(data))
           {
               mr_printf("Write failed: %s\r\n", mr_strerror(ret));
           }
           mr_printf("DAC value: %d\r\n", data);
           mr_delay_ms(500);
       }
   }
}
```

Enable DAC1 channel 1, output the DAC value every 500ms and print it
(the output value increases by 500 each time until reaching the maximum value).
