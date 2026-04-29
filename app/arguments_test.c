#include "../common/user_syscalls.h"
#include "../common/string.h"

void main(int n_arguments, char* arg1, char* arg2) {
    call_syscall_write("[TEST]\nI'm loaded from the filesystem.\n");

    call_syscall_write("TEST ARGUMENTS NUMBER (EXPECTED 1) - ");
    if (n_arguments == 1) {
        call_syscall_write("SUCCESS\n");
    } else {
        call_syscall_write("FAIL\n");
    }

    call_syscall_write("Argument 1: ");
    call_syscall_write(arg1);
    call_syscall_write("\n");

    call_syscall_write("Argument 2: ");
    call_syscall_write(arg2);
    call_syscall_write("\n");

    call_syscall_exit();
    call_syscall_write("This text should never appear.\n");
}
