/**
 * UART driver per Raspberry Pi 3.
 * Inizializza GPIO e PL011, imposta baud rate, gestisce TX/RX e interrupt.
 */

#include <stddef.h>
#include <stdint.h>
#include "uart.h"
#include "../mbox/mbox.h"
#include "../../libs/mmio.h"
#include "../../libs/utils.h"

// ==============================
// Inizializzazione
// ==============================

void uart_init(void)
{
	// Disabilita UART0
	mmio_write(UART0_CR, 0x00000000);

	// Configura GPIO 14 e 15 per UART (pull disabilitati temporaneamente)
	mmio_write(*GPPUD, 0x00000000);
	delay(150);
	mmio_write(*GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
	mmio_write(*GPPUDCLK0, 0x00000000);

	// Pulisce gli interrupt pendenti
	mmio_write(UART0_ICR, 0x7FF);

	// Imposta clock UART a 3 MHz tramite mailbox (stabile per baud rate)
	mbox[0] = 9 * 4;
	mbox[1] = MBOX_REQUEST;
	mbox[2] = MBOX_TAG_SETCLKRATE;
	mbox[3] = 12;
	mbox[4] = 8;
	mbox[5] = 2;
	mbox[6] = 3000000;
	mbox[7] = 0;
	mbox[8] = MBOX_TAG_LAST;
	mbox_call(MBOX_CH_PROP);

	// Baud rate 115200: IBRD/FBRD
	mmio_write(UART0_IBRD, 1);
	mmio_write(UART0_FBRD, 40);

	// FIFO abilitato, 8 bit, 1 stop, no parity
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Maschera gli interrupt comuni (RX/TX/timeout ecc.)
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
							 (1 << 6) | (1 << 7) | (1 << 8) |
							 (1 << 9) | (1 << 10));

	// Abilita UART0 (RX/TX)
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

// ==============================
// Trasmissione/Ricezione
// ==============================

void uart_putc(unsigned char c)
{
	// Attende che il TX FIFO non sia pieno
	while (mmio_read(UART0_FR) & (1 << 5)) { }
	mmio_write(UART0_DR, c);
}

unsigned char uart_getc(void)
{
	// Attende che il RX FIFO non sia vuoto
	while (mmio_read(UART0_FR) & (1 << 4)) { }
	return mmio_read(UART0_DR);
}

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; ++i)
		uart_putc((unsigned char)str[i]);
}

void uart_hex(unsigned long num)
{
	// Stampa un numero esadecimale (64 bit)
	uart_puts("0x");
	for (int i = 60; i >= 0; i -= 4) {
		unsigned int nibble = (num >> i) & 0xF;
		uart_putc("0123456789ABCDEF"[nibble]);
	}
}

// ==============================
// Gestione interrupt
// ==============================

static inline int uart_rx_fifo_not_empty(void) {
	return !(mmio_read(UART0_FR) & (1 << 4));
}

static inline int uart_tx_fifo_not_full(void) {
	return !(mmio_read(UART0_FR) & (1 << 5));
}

void handle_uart_irq(void)
{
	unsigned int mis = mmio_read(UART0_MIS);

	// RX interrupt
	if (mis & (1 << 4)) {
		while (uart_rx_fifo_not_empty()) {
			unsigned int dr = mmio_read(UART0_DR);
			unsigned int err = (dr >> 8) & 0xF;

			if (err) {
				uart_puts("UART RX error ");
				uart_hex(err);
				uart_puts("\r\n");
			} else {
				char c = (char)(dr & 0xFF);
				uart_putc(c);
				uart_puts("\n");
			}
		}
		mmio_write(UART0_ICR, (1 << 4));
	}

	// TX interrupt
	if (mis & (1 << 5)) {
		// Se non hai un buffer TX, disabilita TXIM
		unsigned int imsc = mmio_read(UART0_IMSC);
		mmio_write(UART0_IMSC, imsc & ~(1 << 5));
		mmio_write(UART0_ICR, (1 << 5));
	}

	// Timeout di ricezione
	if (mis & (1 << 6)) {
		while (uart_rx_fifo_not_empty()) {
			unsigned int dr = mmio_read(UART0_DR);
			char c = (char)(dr & 0xFF);
			uart_putc(c);
			uart_puts("\n");
		}
		mmio_write(UART0_ICR, (1 << 6));
	}
}

