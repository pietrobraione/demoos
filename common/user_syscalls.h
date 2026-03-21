#ifndef __USER_SYSCALLS_H
#define __USER_SYSCALLS_H

#include "../common/ipc_types.h"
#include "../common/fat_types.h"
#include "../common/syscalls_types.h"

// UART
void call_syscall_write(char* buffer);
void call_syscall_write_hex(int number);
int call_syscall_input(char* buffer, int len);

// Filesystem
int call_syscall_create_dir(char* dir_relative_path);
int call_syscall_open_dir(char* dir_relative_path);
int call_syscall_open_file(char* file_realtive_path, uint8_t flags);
int call_syscall_close_file(int file_descriptor);
int call_syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

// Memory
unsigned long call_syscall_malloc();

// Processes
void call_syscall_exit();
int call_syscall_fork();
void call_syscall_yield();
int call_syscall_exec(char* path, int n_arguments, char arguments[][SYSCALL_EXEC_ARGUMENT_DIMENSION]);
int call_syscall_wait(int pid);

// IPC
int call_syscall_send_message(int destination_pid, char* body);
void call_syscall_receive_message(char* body);
int call_syscall_send_signal(int destination_pid, int signal_flag);

#endif
