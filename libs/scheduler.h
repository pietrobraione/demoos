#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#define CPU_CONTEXT_OFFSET_IN_PCB 0

#ifndef __ASSEMBLER__

#define N_PROCESSES 64

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
    unsigned long fp;
    unsigned long sp;
    unsigned long pc;
};

struct PCB {
    struct cpu_context cpu_context;
    long state;
    long counter;
    long priority;
    int preempt_disabled;
};

#define PROCESS_RUNNING 1

#define INIT_TASK { \
    { 0,0,0,0,0,0,0,0,0,0,0,0,0 }, \
    0, 0, 1, 0 \
}

extern struct PCB* current_process;
extern struct PCB* processes[N_PROCESSES];
extern int n_processes;

extern void preempt_enable();
extern void preempt_disable();
extern void _schedule();
extern void switch_to_process(struct PCB*);
extern void handle_timer_tick();

#endif
#endif
