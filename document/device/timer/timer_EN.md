# TIMER Device

[中文](timer.md)

<!-- TOC -->
* [TIMER Device](#timer-device)
  * [Open TIMER Device](#open-timer-device)
  * [Close TIMER Device](#close-timer-device)
  * [Control TIMER Device](#control-timer-device)
    * [Set/Get TIMER Device Configuration](#setget-timer-device-configuration)
    * [Set/Get Timeout Callback Function](#setget-timeout-callback-function)
  * [Read Running Time of TIMER Device](#read-running-time-of-timer-device)
  * [Write Timing Time to TIMER Device](#write-timing-time-to-timer-device)
  * [Usage Example](#usage-example)
<!-- TOC -->

## Open TIMER Device

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

- `path`: The TIMER device path is usually `timerx`, such as `timer1`, `timer2`, `timer3`.
- `flags`: Flags for opening the device, supports `MR_O_RDONLY`, `MR_O_WRONLY`, `MR_O_RDWR`.

Note: When using, the TIMER devices should be opened separately for different tasks according to actual situations, and
the appropriate `flags` should be used for management and permission control to ensure that they will not interfere
with each other.

## Close TIMER Device

```c 
int mr_dev_close(int desc);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| **Return Value** |                    |
| `=0`             | Close successfully |
| `<0`             | Error code         |

## Control TIMER Device

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
    - `MR_IOC_TIMER_SET_MODE`: Set TIMER device mode.
    - `MR_IOC_TIMER_SET_TIMEOUT_CALL`: Set timeout callback function.
    - `MR_IOC_TIMER_GET_MODE`: Get TIMER device mode.
    - `MR_IOC_TIMER_GET_TIMEOUT_CALL`: Get timeout callback function.

### Set/Get TIMER Device Configuration

TIMER device configuration:

- `mode`: Periodic or single mode.

```c
/* Define TIMER device mode */
#define TIMER_MODE                      MR_TIMER_MODE_ONESHOT

/* Set TIMER device mode */
mr_dev_ioctl(ds, MR_IOC_TIMER_SET_MODE, MR_MAKE_LOCAL(int, TIMER_MODE));

/* Get TIMER device mode */
int mode;
mr_dev_ioctl(ds, MR_IOC_TIMER_GET_MODE, &mode);
```

Independent of TIMER interface:

```c
/* Define TIMER device mode */
#define TIMER_MODE                      1

/* Set TIMER device mode */
mr_dev_ioctl(ds, MR_IOC_SCFG, MR_MAKE_LOCAL(int, TIMER_MODE));

/* Get TIMER device mode */
int mode;
mr_dev_ioctl(ds, MR_IOC_GCFG, &mode);
```

Note: The default configuration is:

- Mode: `MR_TIMER_MODE_PERIOD` if not manually configured.

### Set/Get Timeout Callback Function

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set timeout callback function */
mr_dev_ioctl(ds, MR_IOC_TIMER_SET_TIMEOUT_CALL, &fn);
/* Get timeout callback function */
mr_dev_ioctl(ds, MR_IOC_TIMER_GET_TIMEOUT_CALL, &callback);
```

Independent of TIMER interface:

```c
/* Define callback function */
void fn(int desc, void *args)
{
    /* Handle interrupt */
}
void (*callback)(int desc, void *args);

/* Set timeout callback function */
mr_dev_ioctl(ds, MR_IOC_SRCB, &fn);
/* Get timeout callback function */
mr_dev_ioctl(ds, MR_IOC_GRCB, &callback);
```

## Read Running Time of TIMER Device

```c
ssize_t mr_dev_read(int desc, void *buf, size_t count);
```

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| buf              | Read data buffer  |
| count            | Read data size    |
| **Return Value** |                   |
| `>=0`            | Read data size    |
| `<0`             | Error code        |

```c
uint32_t time;
/* Read TIMER device data */
ssize_t size = mr_dev_read(ds, &time, sizeof(time));
/* Whether read successfully */
if (size < 0)
{
    
}
```

Note:

- Time unit is microseconds. Single read minimum unit is `uint32_t`, i.e. 4 bytes.
- Running time refers to the time running after the timer starts until timeout. It resets after timeout.

## Write Timing Time to TIMER Device

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

Note:

- Time unit is microseconds. Single write minimum unit is `uint32_t`, i.e. 4 bytes.
- Only the last valid data is actually valid if multiple units are written (timer will be closed if the last valid data
  is `0`).

## Usage Example

```c
#include "include/mr_lib.h"

void timeout_call(int desc, void *args)
{
    mr_printf("Timeout\r\n");
}

/* Timing time */
#define TIMEOUT                         500000

void timer_init(void) 
{
    /* Initialize timer */
    int timer_ds = mr_dev_open("timer1", MR_O_RDWR);
    if (timer_ds < 0)
    {
        mr_printf("timer open failed: %s\r\n", mr_strerror(timer_ds));
        return;
    }
    /* Print timer descriptor */
    mr_printf("TIMER desc: %d\r\n", timer_ds);
    /* Set timeout callback function */
    mr_dev_ioctl(timer_ds, MR_IOC_TIMER_SET_TIMEOUT_CALL, timeout_call);
    /* Set timing time */
    uint32_t timeout = TIMEOUT;
    int ret = mr_dev_write(timer_ds, &timeout, sizeof(timeout));
    if(ret < 0)
    {
        mr_printf("timer write failed: %s\r\n", mr_strerror(ret));
    }
}
/* Export to automatic initialization (APP level) */
MR_INIT_APP_EXPORT(timer_init);

int main(void)
{
    /* Automatic initialization (timer_init function will be automatically called here) */
    mr_auto_init();

    while(1)
    {
        
    }
}
```

The timer prints "Timeout" every 500ms periodically.