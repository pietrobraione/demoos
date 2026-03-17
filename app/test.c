#include "../common/user_syscalls.h"

void main() {
    call_syscall_write("[TEST]\nI'm loaded from the filesystem.\n");
    call_syscall_exit();
    call_syscall_write("This test should never appear.\n");
}
