#include "../user/user.h"
#include "../common/user_syscalls.h"
#include "../common/process_api.h"
#include <stddef.h>

// How many programs do we want to launch in parallel?
#define NUM_PROCESSES 4

void main() {
    int pids[NUM_PROCESSES];

    call_syscall_write("[RUN_ALL] Starting multiple load test...\n");

    // Process creation loop: one spawn_process call creates each benchmark
    // with its own scheduling parameters (see common/process_api.h). Each
    // child gets a different static priority (used by priority aging), a
    // different number of lottery tickets (its expected CPU share) and a
    // different multilevel queue level (0 = highest priority queue)
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int pid = spawn_process("/bin/benchmark.bin", i + 1, (i + 1) * 10, i);

        if (pid < 0) {
            call_syscall_write("[RUN_ALL] Error: unable to start the binary!\n");
        } else {
            // Save the PID of the newly created child in the array
            pids[i] = pid;
        }
    }

    // The parent process (run_all) now methodically waits for all 
    // the children to finish their work before closing.
    call_syscall_write("[RUN_ALL] All processes started. Waiting for completion...\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        call_syscall_wait(pids[i]);
    }

    call_syscall_write("[RUN_ALL] Test completed! All processes have terminated.\n");
    call_syscall_exit();
}
