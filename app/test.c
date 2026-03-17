#include "../common/user_syscalls.h"

void main(int arg1, int arg2, int arg3, int arg4) {
    call_syscall_write("[TEST]\nI'm loaded from the filesystem.\n");
    
    if (arg1 == 100) {
        call_syscall_write("TEST X0 SUPERATO\n");
    }
    if (arg2 == 101) {
        call_syscall_write("TEST X1 SUPERATO\n");
    }
    if (arg3 == 102) {
        call_syscall_write("TEST X2 SUPERATO\n");
    }

    call_syscall_write((char*)arg4);

    call_syscall_exit();
    call_syscall_write("This test should never appear.\n");
}
