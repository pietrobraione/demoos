#ifndef _FORK_H
#define _FORK_H

#include "scheduler.h"

#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d

int copy_process(unsigned long, unsigned long, unsigned long);
int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc);
void copy_code(struct PCB* process, char* buffer, unsigned long size);
extern void cpu_switch_to_process(struct PCB *, struct PCB *);

struct pt_regs* task_pt_regs(struct PCB*);

struct pt_regs {
    unsigned long registers[31];
    unsigned long sp;
    unsigned long pc;
    unsigned long pstate;
};

#endif
