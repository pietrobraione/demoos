/**
 * Implementazione fork: crea un nuovo processo kernel o utente.
 * Gestisce PCB, stack e registri.
 */

#include "fork.h"
#include "scheduler.h"
#include "allocator.h"
#include "../drivers/irq/entry.h"
#include "../drivers/uart/uart.h"

// ==============================
// Creazione processo
// ==============================

int fork(unsigned long clone_flags, unsigned long function, unsigned long argument, unsigned long stack) {
	preempt_disable();

	struct PCB* new_process = (struct PCB*) get_free_page();
	if (!new_process) return -1;

	struct pt_regs* child_registers = task_pt_regs(new_process);
	memzero((unsigned long)child_registers, sizeof(struct pt_regs));
	memzero((unsigned long)&new_process->cpu_context, sizeof(struct cpu_context));

	if (clone_flags & PF_KTHREAD) {
		// Kernel thread: solo funzione e argomento
		new_process->cpu_context.x19 = function;
		new_process->cpu_context.x20 = argument;
	} else {
		// User thread: copia registri e nuovo stack
		struct pt_regs* current_registers = task_pt_regs(current_process);
		*child_registers = *current_registers;
		child_registers->registers[0] = 0;
		child_registers->sp = stack + PAGE_SIZE;
		new_process->stack = stack;
	}

	int process_id = n_processes++;
	new_process->flags = clone_flags;
	new_process->priority = current_process->priority;
	new_process->state = PROCESS_RUNNING;
	new_process->counter = current_process->priority;
	new_process->preempt_disabled = 1;
	new_process->pid = process_id;

	new_process->cpu_context.pc = (unsigned long) ret_from_fork;
	new_process->cpu_context.sp = (unsigned long) child_registers;

	processes[process_id] = new_process;

	preempt_enable();
	return process_id;
}

// ==============================
// Passaggio a user mode
// ==============================

int move_to_user_mode(unsigned long pc) {
	struct pt_regs* regs = task_pt_regs(current_process);
	memzero((unsigned long)regs, sizeof(*regs));

	regs->pc = pc;
	regs->pstate = PSR_MODE_EL0t;

	unsigned long stack = get_free_page();
	if (!stack) return -1;

	regs->sp = stack + PAGE_SIZE;
	current_process->stack = stack;

	return 0;
}

// ==============================
// Accesso ai registri di un processo
// ==============================

struct pt_regs* task_pt_regs(struct PCB* process) {
	unsigned long p = (unsigned long)process + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs*)p;
}

