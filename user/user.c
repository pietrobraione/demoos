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
  
    // 1. Fix: We pass the PID of the Shell!
    call_syscall_wait(pid);
  
    // 2. Improvement: Safety net. 
    // If the shell dies and the Init process wakes up, we don't let it fall into the void.
    // We ensure it terminates cleanly.
    call_syscall_exit(); 
  }
}
