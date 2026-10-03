#include "../arch/peripherals/base.h"
#include "../arch/mmu.h"
#include "fork.h"
#include "../drivers/irq/entry.h"
#include "../drivers/uart/uart.h"
#include "mm.h"
#include "scheduler.h"
#include "../common/memory.h"

// Creates a new process
// If clone_flags is PF_KTHREAD, the process will execute the given function; otherwhise it will be
// a copy of the current process
int copy_process(unsigned long clone_flags, unsigned long function, unsigned long argument) {
  // I disable the preemption to avoid this function to be interrupted
  preempt_disable();

  struct PCB* new_process;
  new_process = (struct PCB*)allocate_kernel_page();
  
  // Initial check: do we have memory and are we within the process limit?
  if (!new_process || n_processes >= N_PROCESSES) {
    return -1;
  }

  struct pt_regs* child_registers = task_pt_regs(new_process);
  memzero((unsigned long)child_registers, sizeof(struct pt_regs));
  memzero((unsigned long)&new_process->cpu_context, sizeof(struct cpu_context));

  // Files are not shared with the son
  for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
    new_process->files[i] = NULL;
  }

  if (clone_flags & PF_KTHREAD) {
    // If we are running a kernel thread, we only need to specify the function
    new_process->cpu_context.x19 = function;
    new_process->cpu_context.x20 = argument;
  } else {
    struct pt_regs* current_registers = task_pt_regs(current_process);
    *child_registers = *current_registers;

    // The X0 register is the one which contains the return value; if the process is the child, it has to be 0
    child_registers->registers[0] = 0;

    copy_virtual_memory(new_process);
  }

  // Create and assign the PID and all parameters to the PCB
  int process_id = n_processes;
  new_process->flags = clone_flags;
  new_process->priority = current_process->priority;
  // The new process is READY (runnable): it will become RUNNING only when the
  // scheduler dispatches it in switch_to_process
  new_process->state = PROCESS_READY;
  new_process->counter = 10;

  // The estimate of the child's next CPU burst is inherited from the parent: it
  // is the best guess available until the child gets measured on its own bursts
  new_process->est_burst = current_process->est_burst;
  // No tick consumed yet in the current burst
  new_process->burst_ticks = 0;

  // The multilevel queue priority is chosen at creation: the child inherits the
  // parent's level (the init process is at the highest priority, 0, so that is
  // the default). It can be changed on the PCB before the first enqueue
  new_process->queue_priority = current_process->queue_priority;

  // Lottery tickets are inherited from the parent as well (OSTEP chap. 9): the
  // default is the 10 tickets of the init process, and the field can be changed
  // on the PCB to give a process a larger or smaller CPU share
  new_process->tickets = current_process->tickets;
  
  // The child is born with the preemption disabled: schedule_tail re-enables
  // it at the end of its first context switch
  new_process->preempt_disabled = 1;
  new_process->pid = process_id;

  // x19 and x20 will be used in the assembly to call the function
  new_process->cpu_context.pc = (unsigned long)ret_from_fork;
  new_process->cpu_context.sp = (unsigned long)child_registers;

  // ====================================================
  // CRITICAL MODIFICATION: SCHEDULER ABSTRACTION
  // ====================================================
  // Now that the PCB has its PID and data, we use the official 
  // scheduler function to register it in the system!
  if (add_process_to_scheduler(new_process) < 0) {
      return -1; // Fails if the process array is full
  }

  preempt_enable();

  return process_id;
}

// Moves the current process to the user mode, executing the code at the start position, starting
// from the given relative program counter
int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc) {
  struct pt_regs *regs = task_pt_regs(current_process);

  memzero((unsigned long)regs, sizeof(struct pt_regs));

  regs->pstate = PSR_MODE_EL0t;
  regs->pc = pc;
  regs->sp = 16 * PAGE_SIZE;

  copy_code(current_process, (void*)start, size);

  set_pgd(current_process->mm.pgd);
  return 0;
}

// Returns the pointer to the registers struct in the given PCB
struct pt_regs* task_pt_regs(struct PCB *process) {
  unsigned long p = (unsigned long)process + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}

// Copies the given buffer in the process code addresses space; the code will be placed in the first
// pages, until it is fully copied
void copy_code(struct PCB* process, char* buffer, unsigned long size) {
  unsigned long copied_bytes = 0;
  for (int i = 0; i < 16; i++) {
    unsigned long virtual_address  = i * PAGE_SIZE;
    unsigned long kernel_virtual_address = allocate_user_page(process, virtual_address);

    if (copied_bytes < size) {
      int bytes_to_copy = (size - copied_bytes > PAGE_SIZE) ? PAGE_SIZE : size - copied_bytes;
      memcpy((void*)kernel_virtual_address, (void*)(buffer + copied_bytes), bytes_to_copy);

      copied_bytes += bytes_to_copy;
    }
  }
}
