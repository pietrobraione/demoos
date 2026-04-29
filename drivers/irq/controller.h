#ifndef __CONTROLLER_H
#define __CONTROLLER_H

/**
 * Controller degli interrupt: registri e funzioni di abilitazione/gestione.
 * Offset relativi a MMIO_BASE (0x3F000000 su Raspberry Pi 3).
 */

// ==============================
// Registri del controller IRQ
// ==============================

#define IRQ_BASIC_PENDING	0x0000B200
#define IRQ_PENDING_1		0x0000B204
#define IRQ_PENDING_2		0x0000B208
#define FIQ_CONTROL			0x0000B20C
#define ENABLE_IRQS_1		0x0000B210
#define ENABLE_IRQS_2		0x0000B214
#define ENABLE_BASIC_IRQS	0x0000B218
#define DISABLE_IRQS_1		0x0000B21C
#define DISABLE_IRQS_2		0x0000B220
#define DISABLE_BASIC_IRQS	0x0000B224

// ==============================
// Bit utili
// ==============================

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

// IRQ UART0 (PL011) #57 -> ENABLE_IRQS_2 bit 25
#define UART0_IRQ_BIT		(1 << 25)

void enable_interrupt_controller(void);
void irq_vector_init(void);
void enable_irq(void);
void disable_irq(void);

#endif // __CONTROLLER_H

