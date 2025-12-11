/**
 * Entry del kernel: inizializza sottosistemi (UART, IRQ, Timer),
 * crea processo kernel e avvia utenti tramite syscall.
 */

#include <stddef.h>
#include <stdint.h>
#include "../libs/scheduler.h"
#include "../libs/fork.h"
#include "../libs/utils.h"
#include "../libs/syscalls.h"
#include "../drivers/uart/uart.h"
#include "../drivers/timer/timer.h"
#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"

// ==============================
// Dichiarazioni forward
// ==============================

static void kernel_process(void);
static void user_process(void);
static void user_process1(char*);

// ==============================
// Entry principale del kernel
// ==============================

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
	uart_init();
	uart_puts("Hello, kernel world!\r\n");

	irq_vector_init();
	timer_init();
	enable_interrupt_controller();
	enable_irq();

	int res = fork(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
	(void)res;

	// In modalità preemptive lo switch è deciso dal timer;
	// in cooperativa si può forzare il primo schedule.
	// schedule();
}

// ==============================
// Processo kernel: porta in user mode
// ==============================

static void kernel_process(void) {
	uart_puts("Kernel process started\n");

	int error = move_to_user_mode((unsigned long)&user_process);
	if (error < 0) {
		uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
	}
}

// ==============================
// Processo utente (princpale)
// ==============================

static void user_process(void) {
	call_syscall_write("User process started\n");

	unsigned long stack = call_syscall_malloc();
	if ((long)stack < 0) {
		uart_puts("[ERROR] Cannot allocate stack for process 1\n\r");
		return;
	}
	call_syscall_write("[DEBUG] Allocated stack for process 1\n");

	int error = call_syscall_clone((unsigned long)&user_process1, (unsigned long)"12345", stack);
	if (error < 0) {
		uart_puts("[ERROR] Cannot clone process 1\n\r");
		return;
	}
	call_syscall_write("[DEBUG] Cloned process 1\n");

	stack = call_syscall_malloc();
	if ((long)stack < 0) {
		uart_puts("[ERROR] Cannot allocate stack for process 2\n\r");
		return;
	}
	call_syscall_write("[DEBUG] Allocated stack for process 2\n");

	error = call_syscall_clone((unsigned long)&user_process1, (unsigned long)"abcd", stack);
	if (error < 0) {
		uart_puts("[ERROR] Cannot clone process 2");
		return;
	}
	call_syscall_write("[DEBUG] Cloned process 2\n");

	call_syscall_exit();
}

// ==============================
// Processo utente (stampa caratteri)
// ==============================

static void user_process1(char* array) {
	char buffer[2] = {0};
	while (1) {
		for (int i = 0; i < 5; i++) {
			buffer[0] = array[i];
			call_syscall_write(buffer);
			delay(100000);
		}
	}
}

