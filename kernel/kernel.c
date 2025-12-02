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

void kernel_process();
void user_process();
void user_process1(char*);

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    uart_init();
    uart_puts("Hello, kernel world!\r\n");

    irq_vector_init();
    timer_init();
    enable_interrupt_controller();
    enable_irq();

    int res = fork(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);

	// Non serve piu', lo switch dei processi e' ora deciso dal timer
    /*while (1) {
        schedule();
    }*/

	// Necessario solo per SCHEDULING COOPERATIVO,
	// Forza l'esecuzione del primo processo
	// modificare linea 74, libs/scheduler.c ogni volta che si cambia modalita'
	//schedule();

}

void kernel_process() {
    uart_puts("Kernel process started\n");

    int error = move_to_user_mode((unsigned long)&user_process);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }

}

void process(int a) {
	while(1) {
		uart_puts("Sono il processo ");
		uart_hex(a);
		uart_puts("\n");
		for (volatile unsigned long i = 0; i < 20000000; i++); // Simula lavoro}
		
		// Necessario solo per SCHEDULING COOPERATIVO,
		// modificare linea 74, libs/scheduler.c ogni volta che si cambia modalita'
		//schedule();
	}
}

void user_process() {
    call_syscall_write("User process started\n");

    unsigned long stack = call_syscall_malloc();
    if (stack < 0) {
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
    if (stack < 0) {
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

void user_process1(char* array) {
    char buffer[2] = {0};
    while (1) {
        for (int i = 0; i < 5; i++) {
            buffer[0] = array[i];
            call_syscall_write(buffer);
            delay(100000);
        }
    }
}