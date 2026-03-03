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
  long counter;
  long priority;
  int preempt_disabled;
  long pid;

  unsigned long flags;

  FatResource *files[16];

  struct mm_struct mm;

  struct MessagesCircularBuffer messages_buffer;
};

#define PROCESS_RUNNING 1
#define PROCESS_ZOMBIE 2
#define PROCESS_WAITING_UART_INPUT 3
#define PROCESS_WAITING_TO_RECEIVE_MESSAGE 4
#define PROCESS_WAITING_TO_SEND_MESSAGE 5

#define INIT_PROCESS {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 1, 1, 0, 0, 0, {}, {0, 0, {}, 0, {}}, {}}

extern struct PCB *current_process;
extern struct PCB *processes[N_PROCESSES];
extern int n_processes;

extern void preempt_enable();
extern void preempt_disable();
extern void schedule();
extern void switch_to_process(struct PCB *);
extern void handle_timer_tick();
extern void exit_process();

// 741fade

#endif
#endif
