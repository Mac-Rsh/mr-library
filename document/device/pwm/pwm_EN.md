# PWM Device

[中文](pwm.md)

<!-- TOC -->
* [PWM Device](#pwm-device)
  * [Open PWM Device](#open-pwm-device)
  * [Close PWM Device](#close-pwm-device)
  * [Control PWM Device](#control-pwm-device)
    * [Set/Get Channel Number](#setget-channel-number)
    * [Set/Get Channel Configuration](#setget-channel-configuration)
    * [Set/Get Frequency](#setget-frequency)
  * [Read PWM Channel Duty Cycle](#read-pwm-channel-duty-cycle)
  * [Write PWM Channel Duty Cycle](#write-pwm-channel-duty-cycle)
  * [Example:](#example)
<!-- TOC -->

## Open PWM Device

```c
int mr_dev_open(const char *path, int flags);
```

| Parameter        | Description             |  
|------------------|-------------------------|
| path             | Device path             |
| flags            | Flag for opening device |
| **Return Value** |                         |
| `>=0`            | Device descriptor       |     
| `<0`             | Error code              |

- `path`: PWM device path usually is: `pwmx`、`pwm1`、`pwm2`.
- `flags`: Flag for opening device, support `MR_O_RDONLY`、 `MR_O_WRONLY`、 `MR_O_RDWR`.

Note: When using, the PWM device should be opened separately for different tasks with the appropriate `flags` 
for management and permission control, to ensure they will not interfere with each other.

## Close PWM Device

```c
int mr_dev_close(int desc);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| **Return Value** |                    |
| `=0`             | Close successfully |
| `<0`             | Error code         |  

Note: When closing the device, all channels will be automatically restored to the default configuration. 
The channels need to be reconfigured after reopening (can disable this feature).

## Control PWM Device

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

- `cmd`: Command code, support:
    - `MR_IOC_PWM_SET_CHANNEL`: Set channel number.
    - `MR_IOC_PWM_SET_CHANNEL_CONFIG`: Set channel configuration.
    - `MR_IOC_PWM_SET_FREQ`: Set frequency.
    - `MR_IOC_PWM_GET_CHANNEL`: Get channel number.
    - `MR_IOC_PWM_GET_CHANNEL_CONFIG`: Get channel configuration.
    - `MR_IOC_PWM_GET_FREQ`: Get frequency.

### Set/Get Channel Number

Channel number range: `0` ~ `31`.

```c
/* Define channel number */
#define CHANNEL_NUMBER                  1

/* Set channel number */   
mr_dev_ioctl(ds, MR_IOC_PWM_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;
mr_dev_ioctl(ds, MR_IOC_PWM_GET_CHANNEL, &number);
```

Independent of PWM interface:

```c
/* Define channel number */
#define CHANNEL_NUMBER                  1

/* Set channel number */   
mr_dev_ioctl(ds, MR_IOC_SPOS, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));

/* Get channel number */
int number;
mr_dev_ioctl(ds, MR_IOC_GPOS, &number);
```

### Set/Get Channel Configuration

Channel configuration:

- `MR_DISABLE`: Disable channel.
- `MR_ENABLE`: Enable channel.

```c
struct mr_pwm_config config = {MR_ENABLE, MR_PWM_POLARITY_NORMAL};

/* Set channel configuration */
mr_dev_ioctl(ds, MR_IOC_PWM_SET_CHANNEL_CONFIG, &config);
/* Get channel configuration */  
mr_dev_ioctl(ds, MR_IOC_PWM_GET_CHANNEL_CONFIG, &config);
```

Independent of PWM interface:

```c
int config[] = {MR_ENABLE, 0};

/* Set channel configuration */
mr_dev_ioctl(ds, MR_IOC_SCFG, &config);
/* Get channel configuration */  
mr_dev_ioctl(ds, MR_IOC_GCFG, &config);
```

### Set/Get Frequency

```c
/* Define frequency */
#define PWM_FREQ                        1000

/* Set frequency */
mr_dev_ioctl(ds, MR_IOC_PWM_SET_FREQ, MR_MAKE_LOCAL(uint32_t, PWM_FREQ));  

/* Get frequency */
uint32_t freq;
mr_dev_ioctl(ds, MR_IOC_PWM_GET_FREQ, &freq);
```

Independent of PWM interface:

```c
/* Define frequency */
#define PWM_FREQ                        1000

/* Set frequency */
mr_dev_ioctl(ds, (0x01), MR_MAKE_LOCAL(uint32_t, PWM_FREQ));  

/* Get frequency */
uint32_t freq;
mr_dev_ioctl(ds, (-(0x01)), &freq);
```

## Read PWM Channel Duty Cycle

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| Parameter        | Description          |
|------------------|----------------------|
| desc             | Device descriptor    |
| buf              | Read data buffer     |
| count            | Read data size       |
| **Return Value** |                      |
| `>=0`            | Size of reading data |
| `<0`             | Error code           |

```c
/* Read duty cycle */
uint32_t duty;
int ret = mr_dev_read(ds, &duty, sizeof(duty));
/* Check if reading succeeds */
if (ret != sizeof(duty))
{
    return ret;
}
```

Note: The reading data is PWM duty cycle, ranging from `0` to `1000000`. 
The minimum unit for single reading is `uint32_t`, which is 4 bytes.

## Write PWM Channel Duty Cycle

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t count);  
```

| Parameter        | Description          |
|------------------|----------------------|
| desc             | Device descriptor    |
| buf              | Write data buffer    |
| count            | Write data size      |
| **Return Value** |                      |
| `>=0`            | Size of writing data |
| `<0`             | Error code           |

```c
/* Write duty cycle */
uint32_t duty = 500000;
int ret = mr_dev_write(ds, &duty, sizeof(duty));
/* Check if writing succeeds */
if (ret != sizeof(duty))
{
    return ret;
}
```

Note: The writing data is PWM duty cycle, ranging from `0` to `1000000`. 
The minimum unit for single writing is `uint32_t`, which is 4 bytes.

## Example:

```c
#include "include/mr_lib.h"

/* Define channel number and frequency */
#define CHANNEL_NUMBER                  1
#define FREQ                            1000

/* PWM device descriptor */  
int pwm_ds = -1;

void pwm_init(void)
{
    int ret = MR_EOK;

    /* PWM initialization */
    pwm_ds = mr_dev_open("pwm1", MR_O_RDWR);
    if (pwm_ds < 0)
    {
        mr_printf("PWM1 open failed: %s\r\n", mr_strerror(pwm_ds));
        return;
    }
    /* Print PWM descriptor */
    mr_printf("PWM1 desc: %d\r\n", pwm_ds);
    /* Set to channel 1*/ 
    mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_CHANNEL, MR_MAKE_LOCAL(int, CHANNEL_NUMBER));
    /* Set channel enable */
    ret = mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_CHANNEL_CONFIG, MR_MAKE_LOCAL(struct mr_pwm_config, MR_ENABLE, MR_PWM_POLARITY_NORMAL));
    if (ret < 0)
    {
        mr_printf("Channel%d enable failed: %s\r\n", CHANNEL_NUMBER, mr_strerror(ret));
        return;
    }
    ret = mr_dev_ioctl(pwm_ds, MR_IOC_PWM_SET_FREQ, MR_MAKE_LOCAL(uint32_t, FREQ));
    if (ret < 0)
    {
        mr_printf("Freq configure failed: %s\r\n", mr_strerror(ret));
    }
}
/* Export to automatic initialization (APP level) */
MR_INIT_APP_EXPORT(pwm_init);

int main(void)
{
    /* Automatically initialize (pwm_init function will be automatically called here) */
    mr_auto_init();
    
    /* Write duty cycle */
    uint32_t duty = 500000;
    int ret = mr_dev_write(pwm_ds, &duty, sizeof(duty));
    /* Check if writing succeeds */
    if (ret != sizeof(duty))
    {
        mr_printf("Write failed: %s\r\n", mr_strerror(ret));
        return ret;
    }

    while(1)
    {

    }
}
```

The PWM frequency is set to 1000Hz, and channel 1 outputs 50% duty cycle.
