#include "ipc.h"
#include "scheduler.h"
#include "../drivers/uart/uart.h"
#include "./mm.h"
#include "../common/string.h"
#include <stddef.h>

// Sends a message from a process to another one
int send_message(struct PCB* source_process, struct PCB* destination_process, char* body) {
    struct Message message;
    
    message.source_process = current_process;
    message.destination_process = destination_process;
    strcpy(message.body, body);

    int push_ok = -1;
    do {
        push_ok = push_message(&destination_process->messages_buffer, &message);
        if (push_ok == -1) {
            source_process->state = PROCESS_WAITING_TO_SEND_MESSAGE;
            schedule();
        }
    } while (push_ok != 0);

    return 0;
}

// Blocks the process until it receives a message, and puts its content in body
void receive_message(struct PCB* destination_process, char* body) {
    struct Message received_message;

    do {
        int pop_ok = pop_message(&destination_process->messages_buffer, &received_message);
        if (pop_ok == 0) {
            break;
        } else {
            current_process->state = PROCESS_WAITING_TO_RECEIVE_MESSAGE;
            schedule();
        }
    } while (1);
    strcpy(body, received_message.body);

    for (int i = 0; i < N_PROCESSES; i++) {
        if (processes[i]->state == PROCESS_WAITING_TO_SEND_MESSAGE) {
            processes[i]->state = PROCESS_RUNNING;
        }
    }
}

// Pushes a message in the given circular buffer
int push_message(struct MessagesCircularBuffer* buffer, struct Message* message) {
    int next_head = buffer->head + 1;
    if (next_head > MAX_MESSAGES_PER_PROCESS) {
        next_head = 0;
    }

    if (next_head == buffer->tail) {
        // It means that the buffer is full
        return -1;
    }

    buffer->buffer[buffer->head] = *message;
    buffer->head = next_head;

    // If the destination process is waiting for a message, I awake him
    struct PCB* destination_process = message->destination_process;
    if (destination_process->state == PROCESS_WAITING_TO_RECEIVE_MESSAGE) {
        destination_process->state = PROCESS_RUNNING;
    }

    return 0;
}

// Pops the next message from the queue and puts in it message
int pop_message(struct MessagesCircularBuffer* buffer, struct Message* message) {
    if (buffer->head == buffer->tail) {
        // The buffer is empty
        return -1;
    }

    int next_tail = buffer->tail + 1;
    if (next_tail > MAX_MESSAGES_PER_PROCESS) {
        next_tail = 0;
    }

    message->source_process = buffer->buffer[buffer->tail].source_process;
    message->destination_process = buffer->buffer[buffer->tail].destination_process;
    strcpy(message->body, buffer->buffer[buffer->tail].body);

    buffer->tail = next_tail;

    return 0;
}

void print_circular_buffer(struct MessagesCircularBuffer* buffer) {
    uart_puts("[BUFFER] Head: "); uart_hex(buffer->head);
    uart_puts(" | Tail: "); uart_hex(buffer->tail);
    uart_puts("\n");
    for (int i = 0; i < MAX_MESSAGES_PER_PROCESS; i++) {
        uart_hex(i);
        uart_puts("\t");
        uart_puts(buffer->buffer[i].body);
        uart_puts("\n");
    }
}

// Sets the given flag in the given process signals
int send_signal(struct PCB* destination_process, int signal_flag) {
    destination_process->pending_signals |= (1 << signal_flag);

    return 0;
}
