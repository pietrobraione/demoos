/**
 * Implementazione del mailbox property channel.
 * Esegue chiamate sincrone con maschera canale e attende risposta.
 */

#include <stdint.h>
#include "mbox.h"
#include "../../libs/mmio.h"

// ==============================
// Costanti mailbox
// ==============================

#define MBOX_RESPONSE	0x80000000
#define MBOX_FULL		0x80000000
#define MBOX_EMPTY		0x40000000

volatile uint32_t __attribute__((aligned(16))) mbox[36];

#define MBOX_MAIL_CHANNEL_MASK 0xF

// ==============================
// Chiamata mailbox
// ==============================

int mbox_call(uint8_t channel)
{
	uint64_t r = (((uint64_t)(&mbox) & ~MBOX_MAIL_CHANNEL_MASK) | (channel & MBOX_MAIL_CHANNEL_MASK));

	// Attende che il canale non sia pieno
	while (mmio_read(MBOX_STATUS) & MBOX_FULL) { }

	// Invia la richiesta
	mmio_write(MBOX_WRITE, r);

	// Attende risposta per lo stesso indirizzo
	while ((mmio_read(MBOX_STATUS) & MBOX_EMPTY) || mmio_read(MBOX_READ) != r) { }

	return (mbox[1] == MBOX_RESPONSE);
}

