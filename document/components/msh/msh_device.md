# msh-device使用教程

## 查看设备列表
首先，您需要了解系统中已注册的设备。这可以通过`dlist`命令完成。该命令将显示设备的类型（只读、只写、可读写等）以及设备路径。

```bash
dlist
```

输出将类似于：
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

前缀为设备支持的打开方式：`-r---` 只读、`--w--`只写、`-rw--`可读可写、`-rwn-`可非阻塞读写。

中间为设备名，按树状结构拼成完整路径，以 `i2c10` 设备为例，完整路径为：`/dev/i2c1/i2c10` ， 可省略`dev`路径为 `i2c1/i2c10`。

后缀为设备描述符（后面会介绍）。

## 打开设备

要开始与设备交互，您需要先打开它。使用`dopen`命令，指定设备路径和打开模式（只读`r`、只写`w`、可读写`rw`、非阻塞 `-n`）。

```bash
dopen <path> <r|w|rw|-g> [-n]
```

`<>` 必要参数，`[]`可选参数。

- `path`：设备路径
- `r`：只读
- `w`：只写
- `rw`：可读可写
- `-g`：获取设备支持的打开方式
- `-n`：非阻塞

以只读方式打开`adc1`设备。如果成功，输出将类似于：

```
msh /> dopen adc1 r
msh /dev/adc1/-1> 
```

当正确打开命令后，`msh` 后将显示当前持有的设备路径 `/dev/adc1/-1`，最后的 `-1` 为当前的写入位置（对于`adc`设备而言是写入通道）。

## 配置设备

在读取或写入数据之前，您可能需要配置设备。使用`dioctl`命令来设置设备参数。

```bash
dioctl <cmd> <args|-g>
dioctl pos <position|-g>
dioctl cfg <args|-g>
dioctl bufsz <bufsz (>=0)|-g> <-r|-w>
dioctl datasz <-c|-g> <-r|-w>
```

标准格式为：

- `cmd`：命令
- `args`：参数
- `-g`：获取设备参数

为方便操作，内置了一些常用命令：

- `pos`：设置读写位置
  - `position`：读写位置 
  - `-g`：获取读写位置


- `cfg`：设置配置
  - `args`：配置参数
  - `-g`：获取配置参数


- `bufsz`：设置缓冲区大小
  - `bufsz`：缓冲区大小
  - `-g`：获取缓冲区大小


- `datasz`：设置数据大小
  - `-c`：字符数据
  - `-g`：获取数据大小


通过 `dioctl` 先设置写入位置 `pos` ，设置为通道`1`，此时路径变更为 `/dev/adc1/1`。同时通过`cfg -g`获取配置，配置通道`1`使能。

```bash
msh /dev/adc1/-1> dioctl pos 1
msh /dev/adc1/1> dioctl cfg -g
0 
msh /dev/adc1/1> dioctl cfg 1
msh /dev/adc1/1> 
```

## 读取数据

使用`dread`命令从设备读取数据。您需要指定要读取的数据个数、单个数据字节长度和数据类型。

```bash
dread <count> [-1|-2|-4] [-x|-d|-u|-c]
       [-1]: <count (1-128)>
       [-2]: <count (1-64)>
       [-4]: <count (1-32)>
```

- `count`：要读取的数据个数
- `-1|-2|-4`：单个数据字节长度
- `-x|-d|-u|-c`：数据类型

`count` 在单个数据字节长度不同时范围不同，单次读取缓冲区为 `128Byte`

```bash
msh /dev/adc1/1> dread 10 -4 -d
729 592 506 454 416 394 377 366 360 356 
msh /dev/adc1/1> 
```

## 关闭设备

由于`adc`设备不支持写入，所以我们可以先关闭他（当然这不是必须的，后面会讲到其他方法）。

要关闭当前的设备直接使用`dclose`，当需要关闭其他设备时可以添加一个`desc`描述符参数（可能你还不了解描述符概念，不过没有关系，下文会详细介绍）。

```bash
dclose [desc (>=0)]
```

- `desc`：设备描述符

```bash
msh /dev/adc1/1> dclose
msh /> 
```

## 写入数据

要向设备写入数据，使用`dwrite`命令。

```bash
dwrite [-1|-2|-4] [-x|-d|-u|-c] <data>
```

- `-1|-2|-4`：单个数据字节长度
- `-x|-d|-u|-c`：数据类型
- `data`：要写入的数据

先打开一个串口设备，为方便演示此处使用`msh`使用的串口1，通过`dwrite`写入数据，`-c`为数据类型，写入数据为`hello`直接打印在终端上。

```bash
msh /> dopen serial1 rw
msh /dev/serial1/-1> dioctl cfg -g
115200 8 1 0 0 0 
msh /dev/serial1/-1> dwrite -c h e l l o
hellomsh /dev/serial1/-1> 
```

## 切换设备

在不关闭当前设备的情况下切换到其他设备，可以使用`dselect`命令。这允许您在多个设备之间快速切换。

```bash
dselect <desc (>=0)|-g>
```

- `desc`：设备描述符
- `-g`：获取当前设备描述符

上文中我们做了一次`adc`切换到`serial1`设备，但是需要先关闭`adc`设备，这对于使用还是过于麻烦了，不过在此之前需要先介绍一下描述符。

### 描述符

可能用过文件系统的比较了解文件描述符，设备描述符也是类似的概念。

当打开设备（`dopen`）时，系统会自动分配一个描述符给用户使用，描述符记录了`操作的设备`、当前用户的`读写权限`、当前用户的`操作位置`等信息。
持有当前的描述符即代表着一个设备的使用者，拥有使用者的一切信息。

一个设备可以有无限的使用者（`描述符`），而 `msh` 拥有着最高权限（`坐在电脑面前的你`），可以任意使用其他用户的描述符，
可能是你刚刚打开的或者是代码中正在运行程序的描述符（`没错，作为上帝的你可以切换到这个描述符然后关掉它，你甚至可以关掉msh所使用的描述符，然后你的设备就和你失联了`）。

我们可以先`dlist`查看一下现在的设备，会发现和最初先比`serial1`后面的描述符多了`[1]`，也就是我们现在所持有的描述符。

```bash
msh /> dlist
|----- dev         
       |-r--- adc1        
       |-r--- adc2        
       |--w-- dac1        
       |-rw-- pin         
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

我们可以先打开一下`pin`设备，然后再`dlist`查看一下：

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

可以看到`pin`设备后多了`[2]`，就是我们刚打开分配的描述符。

```bash
msh /dev/pin/-1> dselect 1
msh /dev/serial1/-1> dselect -g
1
msh /dev/serial1/-1> 
```

通过 `dselect` 命令可以不关闭当前设备的情况下切换到之前使用的`serial1`设备，使用 `-g` 可以获取当前使用的描述符。
