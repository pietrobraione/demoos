/**
 * Gestione del controller degli interrupt.
 * Abilita le sorgenti (Timer1, UART0) e demultiplexa gli IRQ.
 */

#include "../../libs/mmio.h"
#include "../../libs/utils.h"
#include "../uart/uart.h"
#include "../timer/timer.h"
#include "controller.h"

// ==============================
// Messaggi di errore per entry non valide
// ==============================

static const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",
	"FIQ_INVALID_EL1t",
	"ERROR_INVALID_EL1t",
	"SYNC_INVALID_EL1h",
	"IRQ_INVALID_EL1h",
	"FIQ_INVALID_EL1h",
	"ERROR_INVALID_EL1h",
	"SYNC_INVALID_EL0_64",
	"IRQ_INVALID_EL0_64",
	"FIQ_INVALID_EL0_64",
	"ERROR_INVALID_EL0_64",
	"SYNC_INVALID_EL0_32",
	"IRQ_INVALID_EL0_32",
	"FIQ_INVALID_EL0_32",
	"ERROR_INVALID_EL0_32"
};

// ==============================
// Abilitazione sorgenti IRQ
// ==============================

void enable_interrupt_controller(void)
{
	// Abilita System Timer 1 (ENABLE_IRQS_1)
	unsigned int v1 = mmio_read(ENABLE_IRQS_1);
	mmio_write(ENABLE_IRQS_1, v1 | SYSTEM_TIMER_IRQ_1);

	// Abilita UART0 (ENABLE_IRQS_2, bit 25)
	unsigned int v2 = mmio_read(ENABLE_IRQS_2);
	mmio_write(ENABLE_IRQS_2, v2 | UART0_IRQ_BIT);
}

// ==============================
// Log di entry non valida
// ==============================

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	uart_puts(entry_error_messages[type]);
	uart_puts(", ESR: ");
	uart_hex(esr);
	uart_puts(", address: ");
	uart_hex(address);
	uart_puts("\r\n");
}

// ==============================
// Gestore IRQ (demultiplex)
// ==============================

void handle_irq(void)
{
	unsigned int p1 = mmio_read(IRQ_PENDING_1);
	unsigned int p2 = mmio_read(IRQ_PENDING_2);

	if (p1 & SYSTEM_TIMER_IRQ_1) {
		handle_timer_irq();
		return;
	}

	if (p2 & UART0_IRQ_BIT) {
		handle_uart_irq();
		return;
	}

	uart_puts("Unknown pending irq: ");
	uart_hex(((unsigned long)p1 << 32) | p2);
	uart_puts("\r\n");
}

