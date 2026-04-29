#include "user.h"
#include "../common/user_syscalls.h"

#include <stddef.h>

void init_process_main() {
  int pid = call_syscall_fork();
  if (pid == 0) {
    int error = call_syscall_exec("/bin/shell.bin", 0, NULL);
    if (error) {
      call_syscall_write("[INIT] Cannot start shell binary\n");
      call_syscall_exit();
    }
  } else {
    call_syscall_write("[INIT] Init process started \n");
    while (1) {
      call_syscall_yield();
    }
  }
}
