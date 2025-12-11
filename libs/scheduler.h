#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/**
 * Scheduler round-robin con priorità.
 * Definisce il contesto CPU, la PCB dei processi e le funzioni di scheduling.
 */

// ==============================
// Offset per context switch ASM
// ==============================

#define CPU_CONTEXT_OFFSET_IN_PCB 0

#ifndef __ASSEMBLER__

// ==============================
// Parametri del sistema
// ==============================

#define N_PROCESSES 64			// numero massimo di processi
#define THREAD_SIZE 4096		// dimensione dello spazio di thread

// Flag processo
#define PF_KTHREAD 0x00000002	// processo kernel (thread)

// ==============================
// Contesto CPU salvato tra switch
// ==============================

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;			 // frame pointer (x29)
	unsigned long sp;			 // stack pointer
	unsigned long pc;			 // program counter
};

// ==============================
// PCB: Process Control Block
// ==============================

struct PCB {
	struct cpu_context cpu_context; // contesto CPU
	long state;					 	// stato (RUNNING/ZOMBIE)
	long counter;					// contatore time-slice
	long priority;					// priorità di scheduling
	int	preempt_disabled;			// livello di preemption disabilitata
	long pid;						// identificatore processo

	unsigned long stack;			// base dello stack utente (se presente)
	unsigned long flags;			// flag di creazione (es. PF_KTHREAD)
};

// Stati processo
#define PROCESS_RUNNING 1
#define PROCESS_ZOMBIE	2

// Processo init (primo processo in esecuzione)
#define INIT_PROCESS \
	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
		/* state */ 0, \
		/* counter */ 1, \
		/* priority */ 1, \
		/* preempt_disabled */ 0, \
		/* pid */ 0 \
	}

// ==============================
// Variabili globali dello scheduler
// ==============================

extern struct PCB* current_process;
extern struct PCB* processes[N_PROCESSES];
extern int n_processes;

void preempt_enable(void);
void preempt_disable(void);
void schedule(void);
void switch_to_process(struct PCB* next);
void handle_timer_tick(void);
void exit_process(void);

#endif // __ASSEMBLER__
#endif // __SCHEDULER_H

