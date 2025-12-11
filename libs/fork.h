#ifndef __FORK_H
#define __FORK_H

#include "scheduler.h"

/**
 * Funzioni per creare nuovi processi (kernel thread o user thread).
 * Definizione struct pt_regs per salvataggio registri.
 */

// ==============================
// Modalità processore (PSR)
// ==============================

#define PSR_MODE_EL0t	 0x00000000
#define PSR_MODE_EL1t	 0x00000004
#define PSR_MODE_EL1h	 0x00000005
#define PSR_MODE_EL2t	 0x00000008
#define PSR_MODE_EL2h	 0x00000009
#define PSR_MODE_EL3t	 0x0000000c
#define PSR_MODE_EL3h	 0x0000000d

int fork(unsigned long, unsigned long, unsigned long, unsigned long);
int move_to_user_mode(unsigned long);
struct pt_regs* task_pt_regs(struct PCB*);

// ==============================
// Struttura registri processo
// ==============================

struct pt_regs {
	unsigned long registers[31];
	unsigned long sp;
	unsigned long pc;
	unsigned long pstate;
};

#endif // __FORK_H

