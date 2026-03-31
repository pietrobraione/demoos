#include "../arch/peripherals/base.h"
#include "../arch/mmu.h"
#include "fork.h"
#include "../drivers/irq/entry.h"
#include "../drivers/uart/uart.h"
#include "mm.h"
#include "scheduler.h"
#include "../common/memory.h"

int map_mmio_registers(struct PCB* process);

// Creates a new process
// If clone_flags is PF_KTHREAD, the process will execute the given function; otherwhise it will be
// a copy of the current process
int copy_process(unsigned long clone_flags, unsigned long function, unsigned long argument) {
  // I disable the preempt to avoid this function to be interrupted
  preempt_disable();

  struct PCB* new_process;
  new_process = (struct PCB*)allocate_kernel_page();
  if (!new_process) {
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
    int error = map_mmio_registers(new_process);
    if (error) {
      return -1;
    }
 }

  int process_id = n_processes;
  new_process->flags = clone_flags;
  new_process->priority = current_process->priority;
  new_process->state = PROCESS_RUNNING;
  new_process->counter = current_process->priority;
  new_process->preempt_disabled = 1;
  new_process->pid = process_id;

  // x19 and x20 will be used in the assembly to call the function
  new_process->cpu_context.pc = (unsigned long)ret_from_fork;
  new_process->cpu_context.sp = (unsigned long)child_registers;

  processes[process_id] = new_process;
  n_processes++;

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

  // I also need to map the addresses for MMIO to let the process communicate with the SD card
  int error = map_mmio_registers(current_process);
  if (error) {
    return -1;
  }

  set_pgd(current_process->mm.pgd);
  return 0;
}

// Returns the pointer to the registers struct in the given PCB
struct pt_regs* task_pt_regs(struct PCB *process) {
  unsigned long p = (unsigned long)process + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}

// Maps the MMIO registers in the process page tables
int map_mmio_registers(struct PCB* process) {
  int error = map_sector(process, DEVICE_BASE, PHYS_MEMORY_SIZE - SECTION_SIZE, DEVICE_BASE, MMU_DEVICE_FLAGS);
  return error;
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
