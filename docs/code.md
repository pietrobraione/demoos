# Code structure

This section will explain in details each section of the operating system, how they work and all the important details. 

## Folders

The project is divided into different folders:

- `arch`: header files which contain data about CPU registers, memory mapped registers addresses...
- `boot`: the bootloader, stored in the `start.S` file
- `common`: libraries (`.h` and `.c` files) which are shared between kernel and user processes; in fact, there are some functions (like string and memory manipulation) which are used by the kernel, but which can also used by the user processes without any security issue, thanks to virtual memory
- `drivers`: drivers for the devices supported by demoos (uart, irq, sd...)
- `kernel`: the `kernel.c` is the main file launched by the bootloader, which initializes the components of the OS and the init process
- `libs`: miscellaneous files which are used by the kernel for its functionalities
- `script`: the linker script used in the building process
- `user`: the init user process, which is a shell
