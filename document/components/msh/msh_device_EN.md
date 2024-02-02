msh-device Usage Guide

## Viewing Device List

First, you need to understand the devices registered in the system. This can be done using the `dlist` command. 
The command will display the device type (read-only, write-only, read-write, etc.) and the device path.

```bash
dlist
```

The output will be similar to:

```
msh /> dlist
|----- dev         
       |-r--- adc1        
       |-r--- adc2        
       |--w-- dac1        
       |-rw-- pin         
       |-rw-- pwm1        
       |-rw-- pwm2             
       |-rwn- serial1      [0]
       |-rwn- serial2     
       |-rwn- serial3      
       |-rw-- spi1        
       |-rw-- spi2        
       |-rw-- timer1      
       |-rw-- timer2      
       |-rw-- timer3      
       |-rw-- i2c1        
              |-rw-- i2c10       
              |-rw-- i2c11       
msh /> 
```

The prefix indicates the supported open modes for the device: `-r---` read-only, `--w--` write-only,
`-rw--` read-write, `-rwn-` non-blocking read-write.

The middle part is the device name, which forms the complete path in a tree structure. For example, 
the complete path for the `i2c10` device is `/dev/i2c1/i2c10`. The `dev` path can be omitted to become `i2c1/i2c10`.

The suffix is the device descriptor (which will be introduced later).

## Opening a Device

To start interacting with a device, you need to open it first. Use the `dopen` command, 
specifying the device path and the open mode (read-only `r`, write-only `w`, read-write `rw`, non-blocking `-n`).

```bash
dopen <path> <r|w|rw|-g> [-n]
```

`<>` are required parameters, `[]` are optional parameters.

- `path`: Device path
- `r`: Read-only
- `w`: Write-only
- `rw`: Read-write
- `-g`: Get the supported open modes
- `-n`: Non-blocking

To open the `adc1` device in read-only mode. If successful, the output will be similar to:

```bash
msh /> dopen adc1 r
msh /dev/adc1/-1> 
```

After the correct open command, `msh` will display the current device path `/dev/adc1/-1`, 
and the `-1` at the end is the current write position (for `adc` devices, it's the write channel).

## Configuring a Device

Before reading or writing data, you may need to configure the device. Use the `dioctl` command to set device parameters.

```bash
dioctl <cmd> <args|-g>
dioctl pos <position|-g>
dioctl cfg <args|-g>
dioctl bufsz <bufsz (>=0)|-g> <-r|-w>
dioctl datasz <-c|-g> <-r|-w>
```

The standard format is:

- `cmd`: Command
- `args`: Parameters
- `-g`: Get device parameters

For convenience, some common commands are built-in:

- `pos`: Set the read/write position
    - `position`: Read/write position
    - `-g`: Get the read/write position

- `cfg`: Set configuration
    - `args`: Configuration parameters
    - `-g`: Get configuration parameters

- `bufsz`: Set buffer size
    - `bufsz`: Buffer size
    - `-g`: Get buffer size

- `datasz`: Set data size
    - `-c`: Character data
    - `-g`: Get data size

First, set the write position `pos` to channel `1`, which changes the path to `/dev/adc1/1`. 
Then, use `cfg -g` to get the configuration and enable channel `1`.

```bash
msh /dev/adc1/-1> dioctl pos 1
msh /dev/adc1/1> dioctl cfg -g
0 
msh /dev/adc1/1> dioctl cfg 1
msh /dev/adc1/1> 
```

## Reading Data

To read data from a device, use the `dread` command. You need to specify the number of data items to read, 
the length of each data byte, and the data type.

```bash
dread <count> [-1|-2|-4] [-x|-d|-u|-c]
       [-1]: <count (1-128)>
       [-2]: <count (1-64)>
       [-4]: <count (1-32)>
```

- `count`: The number of data items to read
- `-1|-2|-4`: The length of each data byte
- `-x|-d|-u|-c`: Data type

The range of `count` varies depending on the length of each data byte, with a buffer size of `128Byte` for each read.

```bash
msh /dev/adc1/1> dread 10 -4 -d
729 592 506 454 416 394 377 366 360 356 
msh /dev/adc1/1> 
```

## Closing a Device

Since the `adc` device does not support writing, we can close it first 
(although this is not necessary, as other methods will be discussed later).

To close the current device, simply use `dclose`. When you need to close other devices, 
you can add a `desc` descriptor parameter 
(you may not be familiar with the concept of descriptors yet, but don't worry, it will be explained in detail below).

```bash
dclose [desc (>=0)]
```

- `desc`: Device descriptor

```bash
msh /dev/adc1/1> dclose
msh /> 
```

## Writing Data

To write data to a device, use the `dwrite` command.

```bash
dwrite [-1|-2|-4] [-x|-d|-u|-c] <data>
```

- `-1|-2|-4`: The length of each data byte
- `-x|-d|-u|-c`: Data type
- `data`: The data to be written

First, open a serial port device for demonstration purposes, using serial port 1 as used by `msh`. 
Write data using `dwrite`, with `-c` as the data type, and the data to be written is `hello`,
which will be directly printed on the terminal.

```bash
msh /> dopen serial1 rw
msh /dev/serial1/-1> dioctl cfg -g
115200 8 1 0 0 0 
msh /dev/serial1/-1> dwrite -c h e l l o
hellomsh /dev/serial1/-1> 
```

## Switching Devices

To switch to another device without closing the current one, you can use the `dselect` command. 
This allows you to quickly switch between multiple devices.

```bash
dselect <desc (>=0)|-g>
```

- `desc`: Device descriptor
- `-g`: Get the current device descriptor

In the text above, we switched from an `adc` to a `serial1` device, but we needed to close the `adc` device first,
which is a bit inconvenient. However, before that, let's introduce the concept of descriptors.

### Descriptors

Those familiar with file systems might understand file descriptors. Device descriptors are a similar concept.

When a device is opened (`dopen`), the system automatically assigns a descriptor for the user. 
The descriptor records information such as the `device being operated`, the `current user's read/write permissions`, 
and the `current user's operation position`.

Holding the current descriptor represents a device user, with all the user's information.

A device can have an unlimited number of users (or `descriptors`), and `msh` has the highest privilege 
(you, sitting in front of the computer), allowing you to use any other user's descriptor,
whether it's one you just opened or a descriptor used by a running program in the code (yes, as God, 
you can switch to this descriptor and close it, and even close the descriptor used by msh, 
then your device will be disconnected).

First, let's use `dlist` to check the current devices and notice that compared to the initial state, 
the descriptor for `serial1` now has `[1]`, which is the descriptor we currently hold.

```bash
msh /> dlist
|----- dev         
       |-r--- adc1        
       |-r--- adc2        
       |--w-- dac1        
       |-rw-- pin          [2]
       |-rw-- pwm1        
       |-rw-- pwm2             
       |-rwn- serial1      [0] [1]
       |-rwn- serial2     
       |-rwn- serial3      
       |-rw-- spi1        
       |-rw-- spi2        
       |-rw-- timer1      
       |-rw-- timer2      
       |-rw-- timer3      
       |-rw-- i2c1        
              |-rw-- i2c10       
              |-rw-- i2c11       
msh /> 
```

We can open the `pin` device and then check with `dlist` again:

```bash
msh /> dlist
|----- dev         
       |-r--- adc1        
       |-r--- adc2        
       |--w-- dac1        
       |-rw-- pin          [2]
       |-rw-- pwm1        
       |-rw-- pwm2             
       |-rwn- serial1      [0] [1]
       |-rwn- serial2     
       |-rwn- serial3      
       |-rw-- spi1        
       |-rw-- spi2        
       |-rw-- timer1      
       |-rw-- timer2      
       |-rw-- timer3      
       |-rw-- i2c1        
              |-rw-- i2c10       
              |-rw-- i2c11       
msh /> 
```

You can see that `[2]` has been added after the `pin` device, which is the descriptor we just opened.

```bash
msh /dev/pin/-1> dselect 1
msh /dev/serial1/-1> dselect -g
1
msh /dev/serial1/-1> 
```

Using the `dselect` command, you can switch to the previously used `serial1` device without closing the current device. 
The `-g` option can be used to get the current descriptor in use.