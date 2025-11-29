#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "fork.h"
#include "allocator.h"

void syscall_write(char* buffer) {
    uart_puts(buffer);
}

int syscall_clone(unsigned long stack) {
    return fork(0, 0, 0, stack);
}

unsigned long syscall_malloc() {
    unsigned long address = get_free_page();
    if (!address) {
        return -1;
    }
    return address;
}


void syscall_exit() {
    exit_process();
}

void* const sys_call_table = {syscall_write, syscall_malloc, syscall_clone, syscall_exit};
