# ADC Device

[中文](adc.md)

<!-- TOC -->
* [ADC Device](#adc-device)
  * [Open ADC Device](#open-adc-device)
  * [Close ADC Device](#close-adc-device)
  * [Control ADC Device](#control-adc-device)
    * [Set/Get Channel Number](#setget-channel-number)
    * [Set/Get Channel Status](#setget-channel-status)
  * [Read ADC Device Channel Value](#read-adc-device-channel-value)
  * [Usage Example:](#usage-example)
<!-- TOC -->

## Open ADC Device

```c
int mr_dev_open(const char *name, int oflags);
```

|    Parameter     |    Description    |
|:----------------:|:-----------------:|
|       name       |    Device name    |
|      oflags      | Open device flags |
| **Return Value** |                   |
|      `>=0`       | Device descriptor |
|       `<0`       |    Error code     |

- `name`: ADC device name usually is: `adcx`, `adc1`, `adc2`.
- `oflags`: Open device flags, support `MR_OFLAG_RDONLY`.

Note: When using, the ADC device should be opened separately for different tasks according to actual situations, and the appropriate `oflags` should be used for management and permission control to ensure they will not interfere with each other.

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

Note: When closing the device, all channels will be automatically restored to the default state. The channel needs to be reconfigured after reopening.

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
    - `MR_CTL_ADC_SET_CHANNEL`: Set channel number.
    - `MR_CTL_ADC_SET_CHANNEL_STATE`: Set channel state.
    - `MR_CTL_ADC_GET_CHANNEL`: Get channel number.
    - `MR_CTL_ADC_GET_CHANNEL_STATE`: Get channel state.

### Set/Get Channel Number

Channel number range: `0` ~ `31`.

```c
/* Define channel number */
#define CHANNEL_NUMBER                  5

/* Set channel number */  
mr_dev_ioctl(ds, MR_CTL_ADC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;
mr_dev_ioctl(ds, MR_CTL_ADC_GET_CHANNEL, &number);
```

### Set/Get Channel Status

Channel status:

- `MR_ADC_STATE_DISABLE`: Disable channel.
- `MR_ADC_STATE_ENABLE`: Enable channel.

```c
/* Set channel status */
mr_dev_ioctl(ds, MR_CTL_ADC_SET_CHANNEL_STATE, MR_MAKE_LOCAL(int, MR_ADC_STATE_ENABLE)); 

/* Get channel status */  
int state;
mr_dev_ioctl(ds, MR_CTL_ADC_GET_CHANNEL_STATE, &state);
```

## Read ADC Device Channel Value

```c
ssize_t mr_dev_read(int desc, void *buf, size_t size);
```

| Parameter        | Description       |
|------------------|-------------------|
|                  |                   |
| desc             | Device descriptor |  
| buf              | Read data buffer  |
| size             | Read data size    |
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

int adc_init(void)
{
   int ret = MR_EOK;

   /* Initialize ADC */
   adc_ds = mr_dev_open("adc1", MR_OFLAG_RDONLY);
   if (adc_ds < 0)
   {
       mr_printf("ADC1 open failed: %s\r\n", mr_strerror(adc_ds));
       return adc_ds;
   }
   /* Print ADC descriptor */
   mr_printf("ADC1 desc: %d\r\n", adc_ds);
   /* Set to channel 5 */
   mr_dev_ioctl(adc_ds, MR_CTL_ADC_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
   /* Set channel enable */
   ret = mr_dev_ioctl(adc_ds, MR_CTL_ADC_SET_CHANNEL_STATE, MR_MAKE_LOCAL(int, MR_ADC_STATE_ENABLE));
   if (ret < 0)
   {
       mr_printf("Channel5 enable failed: %s\r\n", mr_strerror(ret));
       return ret;
   }
   return MR_EOK;
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
