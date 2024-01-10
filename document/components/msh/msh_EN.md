# MSH Component

[中文](msh.md)

<!-- TOC -->
* [MSH Component](#msh-component)
  * [Supported Keys](#supported-keys)
  * [Command Format](#command-format)
  * [Built-in Commands](#built-in-commands)
  * [Custom Commands](#custom-commands)
  * [Parameter Access](#parameter-access)
  * [Usage Example:](#usage-example)
  * [How to use](#how-to-use)
<!-- TOC -->

The `msh` is a command line component of the `mr-library`. `msh` is a command line interpreter that provides a user
interface for interaction. Users can input commands and parameters in the command line to have `msh` interpret and
execute the user input commands.

## Supported Keys

| Key              | Description                                               |
|------------------|-----------------------------------------------------------|
| Backspace        | Delete the character before the cursor                    | 
| Delete           | Delete the character after the cursor                     |
| Left/Right Arrow | Move the cursor left and right                            |
| Tab              | Autocomplete the command according to the cursor position |
| Enter            | Run the command                                           |

## Command Format

All commands are separated by spaces, in the format of:

`command [param1] [param2] ... [paramn]`

## Built-in Commands

`msh` has some basic built-in commands such as `help` and `clear`.

Use the `help` command after startup to view help information for all built-in commands.

```c
msh> help  
lsdev    - List all devices.
logo     - Show the logo. 
clear    - Clear the screen.
help     - Show help information.
```

## Custom Commands

Export commands through built-in macros.

```c
MR_MSH_CMD_EXPORT(name, fn, help);
```

| Parameter | Description               |
|-----------|---------------------------|
| name      | Command name              |
| fn        | Command handling function |  
| help      | Command help description  |

Command handling function:

```c
int cmd_fn(int argc, void *argv)
{
    
}
```

| Parameter | Description          |
|-----------|----------------------|
| argc      | Number of parameters |
| argv      | Parameter table      |

Note: The command handling function needs to output `\r\n` at the end, otherwise the prompt will follow the current
line.

## Parameter Access

```c
char *p = MR_MSH_GET_ARG(index);
```

## Usage Example:

```c
int hello_fn(int argc, void *argv)
{
    for (size_t i = 0; i < argc; i++)
    {
        mr_msh_printf("hello %s\r\n", MR_MSH_GET_ARG(i));
    }
    return MR_EOK;
}
MR_MSH_CMD_EXPORT(hello, hello_fn, "Hello every one");
```

Run the `hello` command:

```c
msh> hello 1 2 3 4
hello 1
hello 2 
hello 3
hello 4
msh>
```

## How to use

```c
int main(void)
{
    /* Automatic initialization */
    mr_auto_init();
    
    while(1)
    {
        /* msh processing */
        mr_msh_handle();
    }
}
```
