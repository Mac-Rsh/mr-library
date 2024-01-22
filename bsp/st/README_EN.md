# ST configuration tutorial

[中文](README.md)

Note:

- This `BSP` only supports the `HAL` library, and does not currently support `standard` library(`WCH` driver can be used, you can configure yourself) or `LL` library requirements.
- This `BSP` needs to be used together with `CubeMx`.

## Create project

Use `CubeMx` to create a project for the corresponding chip. Enable the required functions(Configure pin mapping, etc).

Note: Do not enable interrupts, otherwise it will report redefinition errors (the `MR` framework has taken over
interrupts).

## Import project into project (2. Add driver)

Copy `bsp/st/driver` and `stm32xxx/driver` files to `driver`.

## Port driver

Refer to the `stm32xxx/driver` path `Kconfig` and `mr_board.h` in the previous step to modify parameters and complete
the migration and tailoring(Note that the `BSP` is ported to the highest model in the same series,
When selecting and using peripherals, please refer to the chip manual to check whether the current chip has additional devices).

## Continue to add mr-library by pressing `README` in the repository directory
