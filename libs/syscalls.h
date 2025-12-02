#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#define __NR_SYSCALLS 4

// Each system call is defined by a number used by the assembly
#define SYSCALL_WRITE_NUMBER  0
#define SYSCALL_MALLOC_NUMBER 1
#define SYSCALL_CLONE_NUMBER  2
#define SYSCALL_EXIT_NUMBER   3

#ifndef __ASSEMBLER__

void syscall_write(char* buffer);
int syscall_fork();

void call_syscall_write(char* buffer);
int call_syscall_clone(unsigned long function, unsigned long argument, unsigned long stack);
unsigned long call_syscall_malloc();
void call_syscall_exit();

#endif
#endif