/**
 * Gestione del System Timer: programma il confronto, pulisce l’interrupt
 * e notifica il tick allo scheduler.
 */

#include "../../libs/mmio.h"
#include "../../libs/utils.h"
#include "../uart/uart.h"
#include "../../libs/scheduler.h"
#include "timer.h"

// ==============================
// Stato del timer
// ==============================

static const unsigned int interval = 200000; // periodo del timer
static unsigned int curVal = 0;

// ==============================
// Inizializzazione timer
// ==============================

void timer_init(void)
{
	curVal = mmio_read(TIMER_CLO);
	curVal += interval;
	mmio_write(TIMER_C1, curVal);
}

// ==============================
// Gestore interrupt timer
// ==============================

void handle_timer_irq(void)
{
	// Pulisce l’interrupt e riprogramma il successivo
	mmio_write(TIMER_CS, TIMER_CS_M1);
	curVal += interval;
	mmio_write(TIMER_C1, curVal);

	// Notifica lo scheduler (tick)
	handle_timer_tick();
}

