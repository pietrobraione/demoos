#ifndef __IPC_H
#define __IPC_H

#include "./scheduler.h"
#include "../common/ipc_types.h"

extern int push_message(struct MessagesCircularBuffer* buffer, struct Message* message);
extern int pop_message(struct MessagesCircularBuffer* buffer, struct Message* message);

int send_message(struct PCB* source_process, int destination_process_pid, char* body);
void receive_message(struct PCB* destination_process, char* body);

#endif