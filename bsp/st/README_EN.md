# ST configuration tutorial

[中文](README.md)

Note:

- This `BSP` only supports the `HAL` library, and does not currently support `standard` library or `LL` library requirements.
- This `BSP` needs to be used together with `CubeMx`.

## Create project

Use `CubeMx` to create a project for the corresponding chip. Enable the required functions (only need to configure pin
mapping relationships, no need to set specific parameter values).

Note: Do not enable interrupts, otherwise it will report redefinition errors (the `MR` framework has taken over
interrupts).

## Import project into project (2. Add driver)

Copy `bsp/st/driver` and `stm32xxx/driver` files to `driver`.

## Port driver

Refer to the `stm32xxx/driver` path `Kconfig` and `mr_board.h` in the previous step to modify parameters and complete
the migration and tailoring.

## Continue to add mr-library by pressing `README` in the repository directory
