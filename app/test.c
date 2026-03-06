#include "./user_syscalls.h"

void main() {
    while (1) {
        call_syscall_write("I am a process loaded from the file system!\n");
    }
}
