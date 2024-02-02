# ADC Device

[中文](adc.md)

<!-- TOC -->
* [ADC Device](#adc-device)
  * [Open ADC Device](#open-adc-device)
  * [Close ADC Device](#close-adc-device)
  * [Control ADC Device](#control-adc-device)
    * [Set/Get Channel Number](#setget-channel-number)
    * [Set/Get Channel Configure](#setget-channel-configure)
  * [Read ADC Device Channel Value](#read-adc-device-channel-value)
  * [Usage Example:](#usage-example)
<!-- TOC -->

## Open ADC Device

```c
int mr_dev_open(const char *path, int flags);
```

|    Parameter     |    Description    |
|:----------------:|:-----------------:|
|       path       |    Device path    |
|      flags       | Open device flags |
| **Return Value** |                   |
|      `>=0`       | Device descriptor |
|       `<0`       |    Error code     |

- `path`: ADC device path usually is: `adcx`, `adc1`, `adc2`.
- `flags`: Open device flags, support `MR_O_RDONLY`.

Note: When using, the ADC device should be opened separately for different tasks according to actual situations, and the
appropriate `flags` should be used for management and permission control to ensure they will not interfere with each
other.

## Close ADC Device

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

## Control ADC Device

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
    - `MR_IOC_ADC_SET_CHANNEL`: Set channel number.
    - `MR_IOC_ADC_SET_CHANNEL_CONFIG`: Set channel configure.
    - `MR_IOC_ADC_GET_CHANNEL`: Get channel number.
    - `MR_IOC_ADC_GET_CHANNEL_CONFIG`: Get channel configure.

### Set/Get Channel Number

Channel number range: `0` ~ `31`.

```c
/* Define channel number */
#define CHANNEL_NUMBER                  5

/* Set channel number */  
mr_dev_ioctl(ds, MR_IOC_ADC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;
mr_dev_ioctl(ds, MR_IOC_ADC_GET_CHANNEL, &number);
```

Independent of ADC interface:

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
/* Set channel configure */
mr_dev_ioctl(ds, MR_IOC_ADC_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(int, MR_ENABLE)); 

/* Get channel configure */  
int state;
mr_dev_ioctl(ds, MR_IOC_ADC_GET_CHANNEL_CONFIG, &state);
```

Independent of ADC interface:

```c
/* Set channel configure */
mr_dev_ioctl(ds, MR_IOC_SCFG, MR_MAKE_LOCAL(int, MR_ENABLE)); 

/* Get channel configure */  
int state;
mr_dev_ioctl(ds, MR_IOC_GCFG, &state);
```

## Read ADC Device Channel Value

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
/* Read channel value */
uint32_t data;
int ret = mr_dev_read(ds, &data, sizeof(data));
/* Check if read successfully */
if (ret != sizeof(data))
{
    return ret;  
}
```

Note: The read data is the raw ADC data. The minimum read unit is `uint32_t`, that is, 4 bytes each time.

## Usage Example:

```c
#include "include/mr_lib.h"

/* Define channel number */
#define CHANNEL_NUMBER                  5

/* Define ADC device descriptor */  
int adc_ds = -1;

void adc_init(void)
{
   int ret = MR_EOK;

   /* Initialize ADC */
   adc_ds = mr_dev_open("adc1", MR_O_RDONLY);
   if (adc_ds < 0)
   {
       mr_printf("ADC1 open failed: %s\r\n", mr_strerror(adc_ds));
       return;
   }
   /* Print ADC descriptor */
   mr_printf("ADC1 desc: %d\r\n", adc_ds);
   /* Set to channel 5 */
   mr_dev_ioctl(adc_ds, MR_IOC_ADC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
   /* Set channel enable */
   ret = mr_dev_ioctl(adc_ds, MR_IOC_ADC_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(int, MR_ENABLE));
   if (ret < 0)
   {
       mr_printf("Channel5 enable failed: %s\r\n", mr_strerror(ret));
   }
}
/* Export to automatic initialization (APP level) */
MR_INIT_APP_EXPORT(adc_init); 

int main(void)
{
   /* Automatic initialization (adc_init function will be automatically called here) */
   mr_auto_init();

   while(1)
   {
       uint32_t data;
       int ret = mr_dev_read(adc_ds, &data, sizeof(data));
       if (ret != sizeof(data)) 
       {
           mr_printf("Read failed: %s\r\n", mr_strerror(ret));
       }
       mr_printf("ADC value: %d\r\n", data);
       mr_delay_ms(1000);
   }
}
```

Enable ADC1 channel 5, read the ADC value every second and print it.
