#include "process_api.h"
#include "user_syscalls.h"
#include <stddef.h>

// Process-creation API (see process_api.h): fork the current process, let the
// parent configure the child's scheduling parameters right after the fork (the
// POSIX scheme of nice()/setpriority(), see the set_sched_param syscall), and
// let the child replace its address space with the given binary.
// ref: man 3 posix_spawn, man 3 posix_spawnattr_setschedparam
int spawn_process(char* path, int priority, int tickets, int queue_priority) {
  // Create the new process as a copy of the caller
  int pid = call_syscall_fork();

  // Fork failed: report the error to the caller
  if (pid < 0) return -1;

  if (pid == 0) {
    // Child process: replace the address space with the binary at the path
    int error = call_syscall_exec(path, 0, NULL);
    // If the exec failed the child must not keep running the parent's code
    if (error) call_syscall_exit();
    // Never reached: on success the exec restarts the process from its entry
    return 0;
  }

  // Parent process: configure the child before returning its pid; a negative
  // parameter means "keep the value inherited from the parent"
  if (priority >= 0) call_syscall_set_sched_param(pid, SCHED_PARAM_PRIORITY, priority);
  if (tickets >= 0) call_syscall_set_sched_param(pid, SCHED_PARAM_TICKETS, tickets);
  if (queue_priority >= 0) call_syscall_set_sched_param(pid, SCHED_PARAM_QUEUE_PRIORITY, queue_priority);

  // Return the pid of the new process to the caller
  return pid;
}
