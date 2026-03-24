#include "scheduler.h"
#include "../drivers/irq/controller.h"
#include "mm.h"
#include "fork.h"

// First I create the init process which is the first one running
static struct PCB init_process = INIT_PROCESS;
struct PCB *current_process = &init_process;
struct PCB *processes[N_PROCESSES] = {
    &init_process,
};
int n_processes = 0;

void handle_process_signals(struct PCB* process);

// Enables the preempt for the current process
void preempt_enable() {
  current_process->preempt_disabled--;
}

// Disabled the preempt for the current process
void preempt_disable() {
  current_process->preempt_disabled++;
}

// Finds the new process to assign the CPU and switches the context to it
void _schedule() {
  preempt_disable();
  long max_counter, next_process_index;
  while (1) {
    max_counter = -1;
    next_process_index = 0;
    for (int i = 0; i < N_PROCESSES; i++) {
      if (processes[i]) {
        handle_process_signals(processes[i]);
        if (processes[i]->state == PROCESS_RUNNING && processes[i]->counter > max_counter) {
          max_counter = processes[i]->counter;
          next_process_index = i;
        }
      }
    }

    if (max_counter > -1) {
      break;
    }

    // If I didn't find any process, I increment the counter of each one
    for (int i = 0; i < N_PROCESSES; i++) {
      if (processes[i]) {
        processes[i]->counter = (processes[i]->counter >> 1) + processes[i]->priority;
      }
    }
  }

  struct PCB* next_process = processes[next_process_index];
  handle_process_signals(next_process);
  
  // I check again the process state because signals can change it
  if (next_process->state == PROCESS_RUNNING) {
    switch_to_process(next_process);
  }
  preempt_enable();
}

// Asks the scheduler to stop the current project to run another one
void schedule() {
  // I give the current process the lower priority
  current_process->counter = 0;
  _schedule();
}

// Modifies the process PCB depending on the pending signals
void handle_process_signals(struct PCB* process) {
  if (!process->pending_signals) {
    return;
  }

  if (process->pending_signals & (1 << SIGNAL_KILL)) {
    process->state = PROCESS_ZOMBIE;
    process->pending_signals &= ~(1 << SIGNAL_KILL);
  } else if (process->pending_signals & (1 << SIGNAL_STOP)) {
    process->state = PROCESS_STOPPED;
    process->pending_signals &= ~(1 << SIGNAL_STOP);
  } else if (process->pending_signals & (1 << SIGNAL_RESUME)) {
    process->state = PROCESS_RUNNING;
    process->pending_signals &= ~(1 << SIGNAL_RESUME);
  }
}

// Switches the CPU context to the new process
void switch_to_process(struct PCB *next_process) {
  if (current_process == next_process) {
    return;
  }
  struct PCB *previous_process = current_process;
  current_process = next_process;

  set_pgd(next_process->mm.pgd);
  cpu_switch_to_process(previous_process, current_process);
}

void schedule_tail(void) {
  preempt_enable();
}

// Calls the scheduler after each timer tick
void handle_timer_tick() {
  // Necessario solo per SCHEDULING COOPERATIVO,
  // modificare kernel/kernel.c ogni volta che si cambia modalita'
  // return;

  current_process->counter -= 1;
  if (current_process->counter > 0 || current_process->preempt_disabled == 1) {
    return;
  }
  current_process->counter = 0;
  enable_irq();
  _schedule();
  disable_irq();
}

// Terminates the current process and calls the scheduler
void exit_process() {
  preempt_disable();

  current_process->state = PROCESS_ZOMBIE;
  for (int i = 0; i < N_PROCESSES; i++) {
    if (!processes[i]) {
      continue;
    }

    if (processes[i]->state == PROCESS_WAITING_ANOTHER_PROCESS && processes[i]->pid_to_wait == current_process->pid) {
      processes[i]->state = PROCESS_RUNNING;
      processes[i]->pid_to_wait = -1;
    }
  }

  preempt_enable();
  schedule();
}
