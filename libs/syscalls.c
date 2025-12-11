/**
 * Implementazione delle system call lato kernel.
 * Ogni funzione fornisce un servizio di base (scrittura su UART, allocazione memoria,
 * clonazione processo, terminazione).
 */

#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "fork.h"
#include "allocator.h"

// ==============================
// System call: scrittura su UART
// ==============================

void syscall_write(char* buffer) {
		uart_puts(buffer);
}

// ==============================
// System call: clonazione processo
// ==============================

int syscall_clone(unsigned long stack) {
		// Crea un nuovo processo utente con stack specificato
		return fork(0, 0, 0, stack);
}

// ==============================
// System call: allocazione memoria
// ==============================

unsigned long syscall_malloc(void) {
		unsigned long address = get_free_page();
		return address ? address : (unsigned long)-1;
}

// ==============================
// System call: terminazione processo
// ==============================

void syscall_exit(void) {
		exit_process();
}

// ==============================
// Tabella delle system call
// ==============================

void* const sys_call_table[] = {
		syscall_write,	// numero 0
		syscall_malloc,	// numero 1
		syscall_clone,	// numero 2
		syscall_exit	// numero 3
};

