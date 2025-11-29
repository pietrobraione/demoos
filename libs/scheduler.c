#include "scheduler.h"
#include "cpu_switch.h"
#include "../drivers/irq/controller.h"
#include "../drivers/uart/uart.h" // TODO remove, only for debug

// First I create the init process which is the first one running
static struct PCB init_process = INIT_PROCESS;
struct PCB* current_process = &init_process;
struct PCB* processes[N_PROCESSES] = {&init_process, };
int n_processes = 1;

void preempt_enable() {
    current_process->preempt_disabled--;
}

void preempt_disable() {
    current_process->preempt_disabled++;
}

void _schedule() {
    preempt_disable();
    long max_counter, next_process_index;
    while (1) {
        max_counter = -1;
        next_process_index = 0;
        for (int i = 0; i < N_PROCESSES; i++) {
            if (processes[i]) {
                if (processes[i]->state == PROCESS_RUNNING && processes[i]->counter > max_counter) {
                    max_counter = processes[i]->counter;
                    next_process_index = i;
                }
            }
        }

        if (max_counter) {
            break;
        }

        // uart_puts("[DEBUG] I can't find a process with more priority; increasing all\n");
        // If I didn't find any process, I increment the counter of each one
        for (int i = 0; i < N_PROCESSES; i++) {
            if (processes[i]) {
                processes[i]->counter = (processes[i]->counter >> 1) + processes[i]->priority;
            }
        }
    }
    switch_to_process(processes[next_process_index]);
    preempt_enable();
}

// Asks the scheduler to terminate the current project to run another one
void schedule() {
    // I give the current process the lower priority
    current_process->counter = 0;
    _schedule();
}

void switch_to_process(struct PCB* next_process) {
    if (current_process == next_process) {
        return;
    }
    struct PCB* previous_process = current_process;
    current_process = next_process;
    cpu_switch_to_process(previous_process, current_process);
}

void schedule_tail(void) {
    preempt_enable();
}

void handle_timer_tick() {
    uart_puts("[DEBUG] Handling timer tick\n");
    current_process->counter -= 1;
    if (current_process->counter > 0 || current_process->preempt_disabled == 1) {
        return;
    }
    current_process->counter = 0;
    enable_irq();
    _schedule();
    disable_irq();
}

// Terminates the current process
void exit_process() {
    preempt_disable();

    for (int i = 0; i < N_PROCESSES; i++) {
        if (processes[i] == current_process) {
            processes[i]->state = PROCESS_ZOMBIE;
            break;
        }
    }

    if (current_process->stack) {
        free_page(current_process->stack);
    }

    preempt_enable();
    schedule();
}
