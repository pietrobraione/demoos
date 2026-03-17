#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#define __NR_SYSCALLS 19

#ifndef __ASSEMBLER__

#include "./fat32/fat.h"
#include "./ipc.h"
#include "../common/ipc_types.h"
#include "../common/fat_types.h"

void syscall_write(char* buffer);
int syscall_copy_process();
int syscall_create_dir(char* dir_relative_path);
int syscall_open_dir(const char* dir_relative_path);
int syscall_open_file(char* file_relative_path, uint8_t flags);
int syscall_close_file(int file_descriptor);
int syscall_write_file(int file_descriptor, char* buffer, int len, int* bytes);
int syscall_read_file(int file_descriptor, char* buffer, int len, int* bytes);
void syscall_yield();
int syscall_input(char* buffer, int len);
int syscall_fork();
int syscall_send_message(int destination_pid, char* body);
void syscall_receive_message(char* body);
int syscall_send_signal(int destination_pid, int signal_flag);
int syscall_exec(char* path, unsigned long* trap_frame);

int syscall_get_next_entry(int file_descriptor, FatEntryInfo* entry_info);

void syscall_dispatcher(unsigned long* registers);

#endif
#endif
