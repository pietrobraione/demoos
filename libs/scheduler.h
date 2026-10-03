#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#define CPU_CONTEXT_OFFSET_IN_PCB 0

#ifndef __ASSEMBLER__

#define N_PROCESSES 64

#define THREAD_SIZE 4096

#define MAX_FILES_PER_PROCESS 16
#define MAX_PROCESS_PAGES 16
#define MAX_MESSAGES_PER_PROCESS 4
#define MAX_MESSAGES_BODY_SIZE 256

#define PF_KTHREAD 0x00000002

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

#include "./fat32/fat.h"
#include "../common/ipc_types.h"

typedef enum { RESOURCE_TYPE_FILE, RESOURCE_TYPE_FOLDER } ResourceType;

typedef struct {
  ResourceType resource_type;

  union {
    File *f;
    Dir *d;
  };
} FatResource;

struct user_page {
    unsigned long physical_address;
    unsigned long virtual_address;
};

struct mm_struct {
    unsigned long pgd;
    int n_user_pages;
    struct user_page user_pages[MAX_PROCESS_PAGES];
    int n_kernel_pages;
    unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct Message {
    struct PCB* source_process;
    struct PCB* destination_process;
    char body[MAX_MESSAGES_BODY_SIZE];
};

struct MessagesCircularBuffer {
  volatile int head;
  volatile int tail;
  struct Message buffer[MAX_MESSAGES_PER_PROCESS];
};

struct PCB {
  struct cpu_context cpu_context;
  long state;
  // Residual time slice in timer ticks: recharged by the scheduling algorithm
  // when the process is dispatched and decremented at every timer tick; when it
  // reaches 0 the process can be preempted. For the priority aging algorithm it
  // doubles as the DYNAMIC priority of the process (historic Linux epoch scheme,
  // see _schedule_priority_aging in scheduler.c)
  long counter;
  // Static priority: used by the priority aging algorithm to recharge counter
  // at the beginning of every epoch
  long priority;
  // Estimated length of the next CPU burst, in timer ticks: it is the sorting key
  // of the SJF ready queue. Every time the process blocks, the estimate is
  // updated with an exponential average of the measured bursts:
  // est_burst = alpha * measured + (1 - alpha) * est_burst, with alpha = 0.5
  long est_burst;
  // Timer ticks consumed so far in the current CPU burst (the measurement starts
  // when the process receives the CPU and ends when it blocks)
  long burst_ticks;
  // Priority level for the multilevel queue algorithms: each queue corresponds
  // to exactly one priority (0 = highest, ML_MAX_LEVELS-1 = lowest) and every
  // process carries the priority of the queue it belongs to. Chosen at process
  // creation (the child inherits the parent's level): MLQ keeps it fixed
  // forever, MLFQ uses it as the starting level before demotions and boosts.
  // It generalizes the old foreground/background pair to N levels and replaces
  // the PID-parity criterion, which gave the user no control over the priority
  int queue_priority;
  // Lottery scheduling tickets (OSTEP, chap. 9 "Proportional Share"): at every
  // scheduling decision a winning ticket is drawn, so the expected CPU share of
  // a process is tickets / total tickets of the runnable processes. Chosen at
  // creation: the child inherits the parent's tickets (default 10)
  long tickets;
  // Preemption-disable nesting counter: while greater than 0 the timer tick will not
  // preempt this process (critical-section guard, see preempt_disable/preempt_enable).
  // Not to be confused with the algorithm's preemption policy (is_preemptive)
  int preempt_disabled;
  long pid;

  unsigned long flags;

  FatResource *files[MAX_FILES_PER_PROCESS];

  struct mm_struct mm;

  struct MessagesCircularBuffer messages_buffer;

  int pending_signals;

  int pid_to_wait;
  struct PCB* next_ready;
};

// The process currently owning the CPU: at most one process is in this state at
// any time, and the transition to it happens only inside switch_to_process
#define PROCESS_RUNNING 1
#define PROCESS_ZOMBIE 2
#define PROCESS_WAITING_UART_INPUT 3
#define PROCESS_WAITING_TO_RECEIVE_MESSAGE 4
#define PROCESS_WAITING_TO_SEND_MESSAGE 5
#define PROCESS_WAITING_ANOTHER_PROCESS 6
#define PROCESS_STOPPED 7
// The process is runnable but does NOT have the CPU: it sits in a ready queue
// waiting to be dispatched. It goes back to this state every time it is
// re-enqueued
#define PROCESS_READY 8

// Highest priority level for the queue_priority field of the PCB: one queue per
// priority level, from 0 (highest) down to n_levels - 1 (lowest, see the
// MultilevelPolicy below)
#define QUEUE_PRIORITY_HIGHEST 0

#define INIT_PROCESS {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 1, 1, 0, 0, QUEUE_PRIORITY_HIGHEST, 10, 0, 0, 0, {}, {0, 0, {}, 0, {}}, {}, 0, -1, NULL}

// =========================================================================
// MULTILEVEL QUEUE POLICY
// =========================================================================
// Anti-starvation boost period of the MLFQ policy, in timer ticks: with the
// 10 ms system tick (TIMER_PERIOD in drivers/timer/timer.h) 300 ticks are 3
// seconds, the order of magnitude OSTEP suggests for the period S of Rule 5
// ("after some time period S, move all the jobs to the topmost queue")
#define MLFQ_BOOST_PERIOD 300

// MLQ and MLFQ share a single implementation: an array of priority queues, one
// queue per priority level, scanned from level 0 (highest priority) downwards.
// Every process carries in queue_priority the level of the queue it belongs to.
// What distinguishes the two algorithms is only this set of parameters: number
// of levels, time quantum per level and the migration rules. MLQ is the
// degenerate case with a null migration policy: every process stays forever in
// the queue chosen by its queue_priority.
#define ML_MAX_LEVELS 5
typedef struct {
  // Number of priority levels actually used (at most ML_MAX_LEVELS)
  int n_levels;
  // Time quantum in ticks assigned when a process is picked from each level
  long quantum[ML_MAX_LEVELS];
  // Migration rule: if 1, a process that exhausts its quantum is demoted to the
  // next lower level (MLFQ); if 0 it stays at its level forever (MLQ)
  int demote_on_expiry;
  // Migration rule: if > 0, every boost_period ticks all processes are moved
  // back to level 0 to prevent starvation (MLFQ); 0 disables the boost (MLQ)
  int boost_period;
} MultilevelPolicy;

// =========================================================================
// SCHEDULING ALGORITHM DESCRIPTOR
// =========================================================================
// Every scheduling algorithm is described by a table of function pointers, so that
// all its algorithm-specific behaviour lives behind a single interface.
// Switching algorithm only requires changing which descriptor the pointer
// active_algorithm (in scheduler.c) refers to: insertion policy, selection policy,
// per-tick bookkeeping and preemptiveness all follow automatically.
typedef struct {
  // Inserts a process that became ready into the algorithm's ready structure(s)
  void (*enqueue)(struct PCB* process);
  // Selects the next process to run and performs the context switch
  void (*pick_next)(void);
  // Optional per-tick bookkeeping (multilevel promotions/demotions); NULL if unused
  void (*on_tick)(void);
  // If 0 the timer tick never forces a context switch: the running process keeps
  // the CPU until it blocks, yields or exits (FCFS, SJF)
  int is_preemptive;
  // Parameters of the multilevel queue engine (only for MLQ/MLFQ, else NULL)
  const MultilevelPolicy* ml_policy;
} SchedAlgorithm;

// Descriptors for all the available scheduling algorithms (defined in scheduler.c)
extern const SchedAlgorithm sched_round_robin;
extern const SchedAlgorithm sched_fcfs;
extern const SchedAlgorithm sched_sjf;
extern const SchedAlgorithm sched_lottery;
extern const SchedAlgorithm sched_priority_aging;
extern const SchedAlgorithm sched_mlq;
extern const SchedAlgorithm sched_mlfq;

extern struct PCB *current_process;
extern struct PCB *processes[N_PROCESSES];
extern int n_processes;

// Kernel API to change one static scheduling parameter of a process (see the
// SCHED_PARAM_* selectors in common/syscalls_types.h). It is the single entry
// point used by the set_sched_param syscall and available to kernel code; the
// scheduler owns it because only the scheduler knows the invariants to protect
// (e.g. the reconciliation between queue_priority and the current queue level).
// ref: man 2 setpriority (POSIX), man 2 sched_setattr (Linux)
extern int sched_set_param(struct PCB* process, int param, long value);

// Critical-section guard of the scheduler: code between preempt_disable() and
// preempt_enable() cannot be preempted by the timer tick. This is a protection
// for the scheduler's data structures, NOT the preemption policy of the
// scheduling algorithm (which is the is_preemptive field of the descriptor)
extern void preempt_enable();
extern void preempt_disable();
extern void schedule();
extern void switch_to_process(struct PCB *);
extern void handle_timer_tick();
extern void exit_process();
extern int add_process_to_scheduler(struct PCB* process);
extern void enqueue_process(struct PCB* process);
#endif
#endif
