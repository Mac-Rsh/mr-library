# PIN Device

[中文](pin.md)

<!-- TOC -->
* [PIN Device](#pin-device)
  * [Open PIN Device](#open-pin-device)
  * [Close PIN Device](#close-pin-device)
  * [Control PIN Device](#control-pin-device)
    * [Set/Get Pin Number](#setget-pin-number)
      * [Pin Number](#pin-number)
    * [Set Pin Mode](#set-pin-mode)
      * [Pin Mode](#pin-mode)
    * [Set/Get External Interrupt Callback Function](#setget-external-interrupt-callback-function)
  * [Read PIN Device Pin Level](#read-pin-device-pin-level)
  * [Write PIN Device Pin Level](#write-pin-device-pin-level)
  * [Usage Example:](#usage-example)
<!-- TOC -->

## Open PIN Device

```c
int mr_dev_open(const char *name, int oflags);
```

| Parameter        | Description       |
|------------------|-------------------|
| name             | Device name       |
| oflags           | Open device flags |
| **Return Value** |                   |  
| `>=0`            | Device descriptor |
| `<0`             | Error code        |

- `name`: The PIN device name is usually `"pin"`.
- `oflags`: Open device flags, supports `MR_OFLAG_RDONLY`, `MR_OFLAG_WRONLY`, `MR_OFLAG_RDWR`.

Note: When using, different tasks should open the PIN device separately according to actual situations and use
appropriate `oflags` for management and permission control to ensure they won't interfere with each other.

## Close PIN Device

```c
int mr_dev_close(int desc);
```

| Parameter        | Description        |
|------------------|--------------------|
| desc             | Device descriptor  |
| **Return Value** |                    |
| `=0`             | Close successfully |  
| `<0`             | Error code         |

Note: Closing the device will not restore the previous configuration by default. The user needs to restore it according
to the actual situation.

## Control PIN Device

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

- `cmd`: Command code, supports:
    - `MR_CTL_PIN_SET_NUMBER`: Set pin number
    - `MR_CTL_PIN_SET_MODE`: Set pin mode
    - `MR_CTL_PIN_SET_EXTI_CALL`: Set external interrupt callback function
    - `MR_CTL_PIN_GET_NUMBER`: Get pin number
    - `MR_CTL_PIN_GET_EXTI_CALL`: Get external interrupt callback function

### Set/Get Pin Number

#### Pin Number

Different MCUs have different GPIO quantities, functions, naming rules, etc. So MR uses digital numbers to define GPIO
pins for unified interfaces on different MCUs.

The default calculation formula is: `Number = Port * 16 + Pin`, where `Port` is the GPIO port number and `Pin` is the
GPIO pin number.
For example, `PC13` corresponds to `Port` C, `Pin` 13, then `Number = (C - A) * 16 + 13 = 32 + 13 = 45`.

Note: This rule may not apply to all MCUs. Special requirements need to check the low-level driver settings.

```c
/* Define pin number */
#define PIN_NUMBER                      45

/* Set pin number */  
mr_dev_ioctl(ds, MR_CTL_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, PIN_NUMBER));

/* Get pin number */
int number;
mr_dev_ioctl(ds, MR_CTL_PIN_GET_NUMBER, &number);
```

Independent of PIN interface:

```c
/* Define pin number */
#define PIN_NUMBER                      45

/* Set pin number */  
mr_dev_ioctl(ds, MR_CTL_SET_OFFSET, MR_MAKE_LOCAL(int, PIN_NUMBER));

/* Get pin number */
int number;
mr_dev_ioctl(ds, MR_CTL_GET_OFFSET, &number);
```

### Set Pin Mode

#### Pin Mode

MR supports 6 common modes:

- `MR_PIN_MODE_NONE`: No mode, pin returns to default status
- `MR_PIN_MODE_OUTPUT`: Output mode, pin is push-pull output mode
- `MR_PIN_MODE_OUTPUT_OD`: Output mode, pin is open-drain output mode
- `MR_PIN_MODE_INPUT`: Input mode, pin is floating input mode
- `MR_PIN_MODE_INPUT_DOWN`: Input mode, pin is pull-down input mode
- `MR_PIN_MODE_INPUT_UP`: Input mode, pin is pull-up input mode

And 5 external interrupt modes:

- `MR_PIN_MODE_IRQ_RISING`: External interrupt mode, triggered on rising edge
- `MR_PIN_MODE_IRQ_FALLING`: External interrupt mode, triggered on falling edge
- `MR_PIN_MODE_IRQ_EDGE`: External interrupt mode, triggered on both edges
- `MR_PIN_MODE_IRQ_LOW`: External interrupt mode, triggered on low level
- `MR_PIN_MODE_IRQ_HIGH`: External interrupt mode, triggered on high level

```c
/* Define pin mode */
#define PIN_MODE                        MR_PIN_MODE_OUTPUT

/* Set pin mode */
mr_dev_ioctl(ds, MR_CTL_PIN_SET_MODE, MR_MAKE_LOCAL(int, PIN_MODE)); 
```

Independent of PIN interface:

```c
/* Define pin mode */
#define PIN_MODE                        1

/* Set pin mode */
mr_dev_ioctl(ds, MR_CTL_SET_CONFIG, MR_MAKE_LOCAL(int, PIN_MODE)); 
```

### Set/Get External Interrupt Callback Function

```c
#define PIN_NUMBER                      45
/* Define external interrupt callback function */
int call(int desc, void *args)
{
  /* Get pin number */
  ssize_t number = *(ssize_t *)args;  

  /* Handle external interrupt event */

  return MR_EOK;
}

/* Set external interrupt callback function */ 
mr_dev_ioctl(ds, MR_CTL_PIN_SET_EXTI_CALL, call);

/* Get external interrupt callback function */
int (*callback)(int desc, void *args);
mr_dev_ioctl(ds, MR_CTL_PIN_GET_EXTI_CALL, &callback);
```

Independent of PIN interface:

```c
#define PIN_NUMBER                      45
/* Define external interrupt callback function */
int call(int desc, void *args)
{
  /* Get pin number */
  ssize_t number = *(ssize_t *)args;  

  /* Handle external interrupt event */

  return MR_EOK;
}

/* Set external interrupt callback function */ 
mr_dev_ioctl(ds, MR_CTL_SET_RD_CALL, call);

/* Get external interrupt callback function */
int (*callback)(int desc, void *args);
mr_dev_ioctl(ds, MR_CTL_GET_RD_CALL, &callback);
```

Note:

- Before setting the external interrupt mode, you need to configure the callback function, otherwise, it becomes a
  callback-free interrupt.
- Even if the PIN device is closed, the callback function will not be invalid until the pin is set to a common mode (
  external interrupts will be ignored when the PIN device is closed).

## Read PIN Device Pin Level

```c
ssize_t mr_dev_read(int desc, void *buf, size_t size);  
```

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |  
| buf              | Read data buffer  |
| size             | Read data size    |
| **Return Value** |                   |
| `>=0`            | Read data size    |
| `<0`             | Error code        |

```c
/* Read pin level */
uint8_t pin_level;
int ret = mr_dev_read(ds, &pin_level, sizeof(pin_level));
/* Check if read successfully */
if (ret != sizeof(pin_level))
{
    return ret;
}
```

## Write PIN Device Pin Level

```c
ssize_t mr_dev_write(int desc, const void *buf, size_t size);
``` 

| Parameter        | Description       |
|------------------|-------------------|
| desc             | Device descriptor |
| buf              | Write data buffer |  
| size             | Write data size   |
| **Return Value** |                   |
| `>=0`            | Write data size   |
| `<0`             | Error code        |

- Level: `0`: Low level; `1`: High level.

```c
/* Write pin level */
uint8_t pin_level = 1;
int ret = mr_dev_write(ds, &pin_level, sizeof(pin_level));
/* Check if write successfully */  
if (ret != sizeof(pin_level))
{
    return ret;
}
```

## Usage Example:

```c
#include "include/mr_lib.h"

/* Define LED pin (PC13) and KEY pin (PA0) */
#define LED_PIN_NUMBER                  45
#define KEY_PIN_NUMBER                  0

int key_call(int desc, void *args)
{
    ssize_t number = *((ssize_t *)args);

    if (number == KEY_PIN_NUMBER)
    {
        /* Print the callback function descriptor */
        mr_printf("KEY callback, desc: %d\r\n", desc);
        /* Toggle the LED pin level */
        uint8_t level = 0;
        mr_dev_read(desc, &level, sizeof(level));
        level = !level;
        mr_dev_write(desc, &level, sizeof(level));
        return MR_EOK;
    }
    return MR_EINVAL;
}

int led_key_init(void)  
{
    int ret = MR_EOK;

    /* Initialize LED */
    int led_ds = mr_dev_open("pin", MR_OFLAG_RDWR);
    if (led_ds < 0)
    {
        mr_printf("led open failed: %s\r\n", mr_strerror(led_ds));
        return led_ds;
    }
    /* Print LED descriptor */
    mr_printf("LED desc: %d\r\n", led_ds);
    /* Set to LED pin */
    mr_dev_ioctl(led_ds, MR_CTL_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, LED_PIN_NUMBER));
    /* Set LED pin to push-pull output mode */
    ret = mr_dev_ioctl(led_ds, MR_CTL_PIN_SET_MODE, MR_MAKE_LOCAL(int, MR_PIN_MODE_OUTPUT));
    if (ret < 0)
    {
        mr_printf("led set mode failed: %s\r\n", mr_strerror(ret));
        return ret;
    }
    /* Set KEY external interrupt callback function (for demonstrating descriptor inheritance, use LED device descriptor to configure KEY callback function) */
    mr_dev_ioctl(led_ds, MR_CTL_PIN_SET_EXTI_CALL, key_call);

    /* Initialize KEY */
    int key_ds = mr_dev_open("pin", MR_OFLAG_RDWR);
    if (key_ds < 0)
    {
        mr_printf("key open failed: %s\r\n", mr_strerror(key_ds));
        return key_ds;
    }
    /* Print KEY descriptor */
    mr_printf("KEY desc: %d\r\n", key_ds);
    /* Set to KEY pin */
    mr_dev_ioctl(key_ds, MR_CTL_PIN_SET_NUMBER, MR_MAKE_LOCAL(int, KEY_PIN_NUMBER));
    /* Set KEY pin to external interrupt (falling edge) mode (without reconfiguring the callback function, use the previous result, i.e. the callback function set by LED) */
    ret = mr_dev_ioctl(key_ds, MR_CTL_PIN_SET_MODE, MR_MAKE_LOCAL(int, MR_PIN_MODE_IRQ_FALLING));
    if (ret < 0)
    {
        mr_printf("key set mode failed: %s\r\n", mr_strerror(ret));
        return ret;
    }
    return MR_EOK;
}
/* Export to auto initialization (APP level) */
MR_INIT_APP_EXPORT(led_key_init);

int main(void)
{
    /* Auto initialization (led_key_init function will be automatically called here) */
    mr_auto_init();

    while(1)
    {
        
    }
}
```

After pressing the KEY, the LED will flip.
Observe the serial port print, you can see the LED and KEY descriptors. Although the external interrupt of the KEY pin
is configured by the KEY,
no callback function is configured. Therefore, the KEY pin callback function inherits the previous configuration, that
is, the callback function of the LED configuration,
so the descriptor printed in the KEY callback function is the descriptor of the LED.
