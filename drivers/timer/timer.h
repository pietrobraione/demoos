#ifndef _TIMER_H
#define _TIMER_H

// Offsets relativi a MMIO_BASE (0x3F000000 su Raspberry Pi 3)
#define TIMER_BASE   0x00003000

#define TIMER_CS     (TIMER_BASE + 0x00)
#define TIMER_CLO    (TIMER_BASE + 0x04)
#define TIMER_CHI    (TIMER_BASE + 0x08)
#define TIMER_C0     (TIMER_BASE + 0x0C)
#define TIMER_C1     (TIMER_BASE + 0x10)
#define TIMER_C2     (TIMER_BASE + 0x14)
#define TIMER_C3     (TIMER_BASE + 0x18)

#define TIMER_CS_M1  (1 << 1)

void timer_init(void);
void handle_timer_irq(void);

#endif
