#include "uart.h"
#include <stddef.h>
#include "../../libs/mmio.h"
#include "../mbox/mbox.h"
#include "../../libs/utils.h"

void uart_init()
{
	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);

	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);

	// For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
	// Set it to 3Mhz so that we can consistently set the baud rate
	mbox[0] = 9*4; //length: 9*4 bytes
	mbox[1] = MBOX_REQUEST;
	mbox[2] = MBOX_TAG_SETCLKRATE;
	mbox[3] = 12;
	mbox[4] = 8;
	mbox[5] = 2; //UART clock
	mbox[6] = 30000000; //3 MHz
	mbox[7] = 0; //clear turbo
	mbox[8] = MBOX_TAG_LAST;
	mbox_call(MBOX_CH_PROP);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fractional part register = (Fractional part * 64) + 0.5
	// Baud = 115200.
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);

	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}

unsigned char uart_getc()
{
	// Wait for UART to have received something.
	while ( mmio_read(UART0_FR) & (1 << 4) ) { }
	return mmio_read(UART0_DR);
}

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; ++i)
		uart_putc((unsigned char)str[i]);
}

void putc ( void* p, char c)
{
	uart_putc(c);
}

void uart_hex(unsigned long num)
{
    uart_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        unsigned int nibble = (num >> i) & 0xF;
        uart_putc("0123456789ABCDEF"[nibble]);
    }
}

// Helpers per FIFO
static inline int uart_rx_fifo_not_empty(void) {
    return !(mmio_read(UART0_FR) & (1 << 4));
}

static inline int uart_tx_fifo_not_full(void) {
    return !(mmio_read(UART0_FR) & (1 << 5));
}

// Gestore interrupt UART
void handle_uart_irq(void)
{
	// uart_puts("UART interrupt received");
    unsigned int mis = mmio_read(UART0_MIS);

    // RX interrupt
    if (mis & (1 << 4)) {
        // uart_puts(": type RX\r\n");
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
	uart_puts(": type TX\r\n");
        // Se non hai buffer TX, disabilita TXIM
        unsigned int imsc = mmio_read(UART0_IMSC);
        mmio_write(UART0_IMSC, imsc & ~(1 << 5));
        mmio_write(UART0_ICR, (1 << 5));
    }

    // Receive timeout
    if (mis & (1 << 6)) {
	uart_puts(": timeout\r\n");
        while (uart_rx_fifo_not_empty()) {
            unsigned int dr = mmio_read(UART0_DR);
            char c = (char)(dr & 0xFF);
            uart_putc(c);
			uart_puts("\n");
        }
        mmio_write(UART0_ICR, (1 << 6));
    }
}
