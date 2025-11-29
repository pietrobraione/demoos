#include "fork.h"
#include "scheduler.h"
#include "allocator.h"
#include "../drivers/irq/entry.h"
#include "../drivers/uart/uart.h"

// Creates a new process that executes the given function
int fork(unsigned long function, unsigned long argument) {
    // I disable the preempt to avoid this function to be interrupted
    preempt_disable();

    // I ask the allocator a free page for the new PCB
    struct PCB* new_process;
    new_process = (struct PCB*) get_free_page();

    if (!new_process) {
        return 1;
    }

    int process_id = n_processes++;

    new_process->priority = current_process->priority;
    new_process->state = PROCESS_RUNNING;
    new_process->counter = current_process->priority;
    new_process->preempt_disabled = 1;
    new_process->pid = process_id;

    // x19 and x20 will be used in the assembly to call the function
    new_process->cpu_context.x19 = function;
    new_process->cpu_context.x20 = argument;
    new_process->cpu_context.pc = (unsigned long) ret_from_fork;
    new_process->cpu_context.sp = (unsigned long) new_process + PROCESS_SIZE;

    processes[process_id] = new_process;

    preempt_enable();

    return 0;
}
