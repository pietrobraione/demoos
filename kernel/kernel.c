#include <stddef.h>
#include <stdint.h>
#include "../drivers/utils.h"
#include "../drivers/uart.h"
#include "../drivers/framebuffer.h"
#include "../drivers/timer.h"
#include "../drivers/irq.h"

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
	uart_init();

	uart_puts("Hello, kernel world!\r\n");

	uart_puts("Exception level: ");
	uart_putc('0' + get_el());
	uart_puts("\n");

	irq_vector_init();
	timer_init();
	enable_interrupt_controller();
	enable_irq();

	// Blocco codice per generare forzatamente eccezione tipo SYNC
	// uart_puts("Triggering SYNC exception...\r\n");
    // asm volatile("svc #0");

	while (1)
		uart_putc(uart_getc());

}

