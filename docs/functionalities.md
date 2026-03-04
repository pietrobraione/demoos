# Functionalities

In this section you can find a list of the functionalities provided by this 
operating system. 

## Bootloader

The bootloader is the first element of DemoOS which is executed. Its goals are:

- Initialize the CPU registers and memory
- Disables the secondary cores
- Creates the page tables for the kernel, both for code and for memory-mapped IO
- Jumps to the kernel

## Virtual memory

DemoOS gives a minimal support to the virtual memory management. 

The virtual addresses of the Raspberry Pi 3B are split into two different parts:

- High addresses, which starts from `0xffff000000000000`, are dedicated for the 
kernel addresses
- Low addresses, which starts from `0x0000000000000000`, can be used from the 
user processes

So, all the virtual addresses generated from the kernel will be contained in the 
high addresses, and cannot be generated from the user processes. 

Memory is split into pages with the dimension of 4KB; each process has 16 pages 
of memory available for its data. These pages are split in this way:

- Code (pages 0-n): the code that will be executed by the process is placed into
 this area. The number of pages occupied by the code segment can change 
 depending on the size of the code
- Stack (page 15): the stack lives inside the last page assigned to the process

All the other pages can be used by the process to save its data. 

### Page tables

The hardware of the Raspberry can handle two different page tables, one for the 
kernel and one for the user process; in this way, when a user process calls a 
system call and the kernel starts its execution, it can be executed without 
having to change the active page tables. 

The creation of the kernel page tables is handled inside `start.S`, which is the
 bootloader, while the user process page creation is handled during the 
 `move_to_user_mode` function, when the pages are assigned to the process. 
 However, it is necessary to switch the page tables everytime the running 
 process is changed. 

The page tables hierarcy is the following: PGD, PUD, PMD, PTE. Furthermore, the 
page tables can contain both page addresses and sectors, so you can map an 
entire sector using a single line of the PMD table. 

#### Kernel page tables

Inside `start.S`, the macro `__create_page_tables` handles the creation of the 
page tables for the kernel. 
This macro maps the whole kernel memory (1GB); as said before, the PMD can map 
an entire sector of 2MB and can have up to 512 records, which make possibile to 
map the entire memory extension. 

To achieve this goal we need two procedures:

- `create_table_entry`: given the address of the main table, creates an entry 
which points to the next table of the hierarcy
- `create_block_map`: given the address of the PMD, writes an entry which maps 
the given sector

So, the `__create_page_tables` macro calls the `create_table_entry` procedure 
twice, one to create the entry in the PMD and in the PTE, and then calls the 
`create_block_map` macro to write the entry of the sector in the PMD. In this 
way, the kernel has the entire memory mapped in its tables. 

## Kernel

After the page tables for the kernel have been created, the bootloader jumps to 
the kernel to start the core functionalities of DemoOS.  

The kernel starts its execution by initializing and enabling:

- IRQ, to intercept the interrupt requests
- UART, to communicate using the serial port
- Filesystem on the SD card

After the core functionalities are enabled, the kernel creates the first kernel 
process by calling the `copy_process` function; then this process is moved to 
the user mode by calling the `move_to_user_mode` function.

This function sets the registers of the process to start the execution from a 
precise point (the main function), assigns 16 pages of memory for the new 
process, maps them into its page tables and also maps the MMIO registers; after 
that, the process is ready to be executed as a user process. The first process 
created by the kernel will run the DemoOS Shell. 

## Processes

### Data

Each process has its own PCB, which contains all the informations about the 
process. Its informations are:

- `CPU context`: values of the CPU registers
- `state` (`RUNNING`, `WAITING`...)
- `counter`: the priority of the process in the current time
- `priority`: it is used to calculate the counter
- `preempt_disabled`: if `true`, the process won't be preempted
- `pid`: the identificator
- `stack`: the pointer
- `flags`: the flags with which the process has been cloned from its father
- `files`: an array with the files open by the process
- `mm`: a struct which contains all the data about the memory allocated for the 
process
- `messages_buffer`: a circular buffer with the messages arrived to the process

### Creation

As said before, the kernel creates the first process by calling the 
`copy_process` function. This function has two different behaviours, depending 
on the flags with which it is invoked:

- **Kernel process** (`PF_KTHREAD`): with this flag, this function creates a new
 process which will execute the function passed as a parameter. The new process 
 will be executed as a kernel process
- **User process**: if no flag is specified, the function will create a new 
process which will be executed in the user space

In both cases, the function creates a new `PCB` instance and initializes its 
attributes; if the process is a kernel one, the registers are set to execute the
given function, otherwhise the registers are a copy of the current process ones
and the whole virtual memory of the current process is copied into the new one. 

### Scheduler

The scheduler can be invoked from the kernel with the `_schedule()` function. 
This function puts the counter of the current process to zero and search for the
next process to be executed, which will be the one with the higher counter from 
the list of the active processes. 

The scheduler is also automatically invoked everytime the system timer ticks: 
when this happens, the CPU generates an IRQ that is handled by the 
`handle_timer_tick` function. This function sets the current process counter to 
-1 and invokes the `_schedule()` function to find the new process to run. This 
operation won't be executed if the current process has disabled the preempt, so
if the `preempt_disable` attribute of its PCB is `true`. 

### Memory management

As seen before, each process has 16 pages of memory, which are assigned during 
the creation of the process (`copy_process`). This function calls the function
`allocate_user_page` from the `allocator.c` file, which contains all the 
functions to manage processes and kernel memory. 

The kernel keeps track of every free page inside the array `memory_pages`. Then,
the pages can be allocated with the following functions:

- `get_free_page`: returns the first page of the list which is not allocated
- `allocate_kernel_page`: this function is called everytime the kernel needs a
memory page; it finds the first free page and then returns its *kernel* virtual 
address (so the physical address of the page summed to the `0xffff000000000000`
address)
- `allocate_user_page`: this function is called from the kernel to assign a page
to a process; it finds the first free page, maps it into the process page tables
and then returns the *kernel* virtual address of the page. 

### IPC

#### Messages

Processes can communicate by sending messages. Each process can accept a fixed 
number of messages, which are contained inside a circular buffer in the process 
PCB.  
The message will be contained inside a raw char array, and the parsing of this 
message must be handled by the destination process. 

There are two operations which can make this IPC possible:

- Send message: sends a message to another process and saves it inside its
circular buffer
- Receive message: reads the first message saved inside the process circular 
buffer

Both sending and receiving messages are blocking:

- When sending a message, if the destination process messages buffer is full, 
the sender will be blocked and resumed the first time the destination process
will pop a message from its buffer
- When receiving a message, the process will wait to have at least a message
in its buffer before continuing its execution

#### Signals

A process can also send signals to another process. There are 3 types of signals:

- Kill: terminates the process
- Stop: stops the process execution until a Resume message is sent
- Resume: resumes the execution of a stopped process

Signals are non blocking and handled entirely by the kernel, in fact for the moment it's not 
possible to define custom handlers for the signals. Furthermore, there is no control about who can
send signals to other processes; for example, the son of a process can kill its father process. 

## System calls

Each user process can invoke a system call to ask the kernel to perform a 
particular operation. When a process calls a system calls, the kernel generates 
an SVC exception; the `entry.S` file intercepts the exception, then calls the 
handler stored in the system call table at the index corresponding to the number
of the generated exception. 

The system calls are defined in the kernel, inside the `syscalls.c` file, 
together with the system call table array. Then, each user process must contain 
the `user_syscalls.h` and `user_syscalls.S` files: this libraries contain the 
functions to generate the SVC exception to invoke the system calls. 

Let's see the flow of a system call invocation from the user process:

- The user calls the `call_syscall_write` function from `user_syscalls.h`
- This function generates an SVC with the code of the `syscall_write` defined in
`user_syscalls.S`, which is `0`
- The `entry.S` calls the function stored at the `0` position
- This function prints on the UART the passed text
- Then, kernel exits and the control returns to the process

#### IO

| Function name                                                                | Description |
| -------------                                                                | ----------- |
| `syscall_write(char* buffer)`                                                | Writes the given buffer into the UART |
| `syscall_input(char* buffer, int len)`                                       | Reads a buffer from the UART and stores it in the passed buffer |
| `syscall_create_dir(char* dir_path)`                                         | Creates a new directory in the filesystem and returns its file descriptor |
| `syscall_open_dir(char dir_path*)`                                           | Opens the given directory and returns its file descriptor |
| `syscall_open_file(char* file_path, uint8_t flags)`                          | Opens the given file and returns its file descriptor |
| `syscall_close_file(int file_descriptor)`                                    | Closes the file with the given file descriptor 
| `syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes)` | Writes the given content in the file with the given file descriptor; bytes will be the number of written bytes |
| `syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes)`  | Reads the content of the open file with the given descriptor and puts it into the given buffer; bytes will be the number of read bytes |
| `syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info)`      | Saves into `entry_info` the info about the entry, and then increases the pointer in the directory |

#### Memory

| Function name                          | Description |
| -------------                          | ----------- |
| `syscall_malloc()`                     | Gets the first free page and allocates it for the kernel; returns the kernel address of the page |

#### Process

| Function name     | Description |
| -------------     | ----------- |
| `syscall_exit()`  | Terminates the current process |
| `syscall_fork()`  | Creates a copy of the current process and returns its PID |
| `syscall_yield()` | Forces the scheduler to assign the CPU to a new process |
| `syscall_send_message(int destination_pid, char* body)` | Sends a message to another process |
