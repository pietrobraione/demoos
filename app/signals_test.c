#include "../common/user_syscalls.h"
#include "../common/ipc_types.h"

void main() {
    int pid = call_syscall_fork();
    if (pid == 0) {
    call_syscall_write("[SON] Started...\n");
    while (1) {
        call_syscall_write("[SON] I am the son and I'm using the CPU just for fun\n");
        call_syscall_yield();
    }
    call_syscall_write("[SON] This line should never be printed\n");
    } else {
    call_syscall_yield();
    call_syscall_send_signal(pid, SIGNAL_KILL);
    call_syscall_write("[FATHER] I terminate my son.\n");
    }
    call_syscall_exit();
}
