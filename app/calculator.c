#include "../common/user_syscalls.h"

void main() {
    call_syscall_write("[CALCULATOR]\n");

    call_syscall_write("Insert the first number: "); 
    int size = 256;
    char buffer[size];
    call_syscall_input(buffer, size);
    
    call_syscall_write("You wrote: ");
    call_syscall_write(buffer);
    call_syscall_write("\n");
    
    call_syscall_exit();
    call_syscall_write("This text should not appear.\n");
}
