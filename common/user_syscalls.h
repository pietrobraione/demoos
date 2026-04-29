#ifndef __USER_SYSCALLS_H
#define __USER_SYSCALLS_H

#include "../common/ipc_types.h"
#include "../common/fat_types.h"
#include "../common/syscalls_types.h"

/**
 * user_syscalls.h defines the system calls callers, which are the functions that generate the svc
 * exceptions to pass the control to the kernel system calls handlers
 */

// -------------------------------------------------------------------------
// UART
// -------------------------------------------------------------------------

// Writes the given buffer into the UART
void call_syscall_write(char* buffer);
//  Writes the given number into the UART in the hexadecimal representation
void call_syscall_write_hex(int number);
// Reads a buffer from the UART and stores it in the passed buffer
int call_syscall_input(char* buffer, int len);

// -------------------------------------------------------------------------
// Filesystem
// -------------------------------------------------------------------------

// Creates a new directory in the filesystem
int call_syscall_create_dir(char* dir_relative_path);
// Opens the given directory
int call_syscall_open_dir(char* dir_relative_path);
// Opens the given file
int call_syscall_open_file(char* file_realtive_path, uint8_t flags);
// Closes the given file
int call_syscall_close_file(int file_descriptor);
// Writes the buffer content inside the given file
int call_syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
// Puts in the given buffer the content of the file
int call_syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
// Saves in entry_info the data about the directory with the given file_descriptor and increases 
// the dir pointer 
int call_syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

// -------------------------------------------------------------------------
// Memory
// -------------------------------------------------------------------------

// Allocates a kernel page
unsigned long call_syscall_malloc();

// -------------------------------------------------------------------------
// Processes
// -------------------------------------------------------------------------

// Stops the execution of the current process
void call_syscall_exit();
// Creates a copy of the current process
int call_syscall_fork();
// Asks the scheduler to execute another process
void call_syscall_yield();
// Substitutes the current process addresses space with the code contained in the given file path
int call_syscall_exec(char* path, int n_arguments, char arguments[][SYSCALL_EXEC_ARGUMENT_DIMENSION]);
// Puts the current process in waiting state, until the desired process invokes the exit system call
int call_syscall_wait(int pid);

// -------------------------------------------------------------------------
// IPC
// -------------------------------------------------------------------------

// Sends a raw message to another process
int call_syscall_send_message(int destination_pid, char* body);
// Blocks the current process until it has received a message, and puts its content in body
void call_syscall_receive_message(char* body);
// Sends a signal to another process
int call_syscall_send_signal(int destination_pid, int signal_flag);

#endif
