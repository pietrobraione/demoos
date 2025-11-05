#include "utils.h"
#include "timer.h"
#include "entry.h"
#include "irq.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

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

void enable_interrupt_controller()
{
	put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
    uart_puts(entry_error_messages[type]);
    uart_puts(", ESR: ");
    uart_hex(esr);
    uart_puts(", address: ");
    uart_hex(address);
    uart_puts("\r\n");
}

void handle_irq(void)
{
    unsigned int irq = get32(IRQ_PENDING_1);

    if (irq & SYSTEM_TIMER_IRQ_1) {
        handle_timer_irq();
    } else {
        uart_puts("Unknown pending irq: ");
        uart_putc('0' + ((irq >> 4) & 0xF));
        uart_putc('0' + (irq & 0xF));
        uart_puts("\r\n");
    }
}

