#ifndef __CPU_SWITCH_H
#define __CPU_SWITCH_H

/**
 * Funzione assembly per il context switch tra due PCB.
 */

struct PCB; // forward declaration
extern void cpu_switch_to_process(struct PCB*, struct PCB*);

#endif // __CPU_SWITCH_H

