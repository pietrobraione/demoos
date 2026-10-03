#ifndef __PROCESS_API_H
#define __PROCESS_API_H

/**
 * process_api.h defines the process-creation API for user programs: library
 * functions built ON TOP of the syscalls (fork + set_sched_param + exec), so
 * that one single call creates a process from a binary with its own scheduling
 * parameters. This is the layering of POSIX: the syscalls stay minimal and
 * orthogonal, the convenience lives in the library.
 * The model is posix_spawn() with the scheduling attributes of posix_spawnattr:
 * ref: man 3 posix_spawn, man 3 posix_spawnattr_setschedparam
 */

// Creates a new process executing the binary at the given path, with the given
// scheduling parameters: priority (static priority, used by priority aging),
// tickets (lottery share) and queue_priority (multilevel queue level, 0 =
// highest). Pass a NEGATIVE value to leave that parameter at the inherited
// default. Returns the pid of the new process, or -1 on error.
int spawn_process(char* path, int priority, int tickets, int queue_priority);

#endif
