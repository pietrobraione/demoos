#ifndef __USER_SYSCALLS_H
#define __USER_SYSCALLS_H

#include "../common/ipc_types.h"
#include "../common/fat_types.h"

void call_syscall_write(char* buffer);
unsigned long call_syscall_malloc();
int call_syscall_clone(unsigned long function, unsigned long argument, unsigned long stack);
void call_syscall_exit();

int call_syscall_create_dir(char* dir_relative_path);
int call_syscall_open_dir(char* dir_relative_path);
int call_syscall_open_file(char* file_realtive_path, uint8_t flags);
int call_syscall_close_file(int file_descriptor);
int call_syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
int call_syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

void call_syscall_yield();
int call_syscall_input(char* buffer, int len);
int call_syscall_fork();

int call_syscall_send_message(int destination_pid, char* body);
void call_syscall_receive_message(char* body);

int call_syscall_send_signal(int destination_pid, int signal_flag);

#endif
