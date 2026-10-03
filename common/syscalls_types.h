#ifndef __SYSCALLS_TYPES
#define __SYSCALLS_TYPES

/**
 * syscalls_types.h defines the constants with the number of each available syscall
 */

#define SYSCALL_WRITE_NUMBER 0
#define SYSCALL_MALLOC_NUMBER 1
#define SYSCALL_EXIT_NUMBER 2
#define SYSCALL_CREATE_DIR_NUMBER 3
#define SYSCALL_OPEN_DIR_NUMBER 4
#define SYSCALL_OPEN_FILE_NUMBER 5
#define SYSCALL_CLOSE_FILE_NUMBER 6
#define SYSCALL_WRITE_FILE_NUMBER 7
#define SYSCALL_READ_FILE_NUMBER 8
#define SYSCALL_YIELD_NUMBER 9
#define SYSCALL_INPUT_NUMBER 10
#define SYSCALL_GET_NEXT_ENTRY_NUMBER 11
#define SYSCALL_FORK_NUMBER 12
#define SYSCALL_SEND_MESSAGE_NUMBER 13
#define SYSCALL_RECEIVE_MESSAGE_NUMBER 14
#define SYSCALL_SEND_SIGNAL_NUMBER 15
#define SYSCALL_EXEC_NUMBER 16
#define SYSCALL_WAIT_NUMBER 17
#define SYSCALL_WRITE_HEX_NUMBER 18
#define SYSCALL_GET_TIME_NUMBER 19
#define SYSCALL_SET_SCHED_PARAM_NUMBER 20

// The maximum size of each argument passed to the exec systemcall
#define SYSCALL_EXEC_ARGUMENT_DIMENSION 64

// Selectors for the set_sched_param syscall: which static scheduling parameter
// of the target process has to be changed. One generic syscall covers the
// per-process knobs of all the algorithms, in the spirit of the POSIX
// setpriority()/nice() interface generalized like Linux's sched_setattr
// ref: man 2 setpriority (POSIX), man 2 sched_setattr (Linux)
#define SCHED_PARAM_PRIORITY 0        // static priority (priority aging)
#define SCHED_PARAM_TICKETS 1         // lottery tickets (proportional share)
#define SCHED_PARAM_QUEUE_PRIORITY 2  // multilevel queue level (MLQ/MLFQ)

#endif
