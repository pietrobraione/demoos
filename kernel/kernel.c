#include <stddef.h>
#include <stdint.h>
#include "../libs/utils.h"
#include "../drivers/uart/uart.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../drivers/timer/timer.h"
#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    // Inizializza UART
    uart_init();
    uart_puts("Hello, kernel world!\r\n");

	// Stampa EL
    uart_puts("Exception level: ");
    uart_putc('0' + get_el());
    uart_puts("\n");

    // Inizializza vettore IRQ e timer
    irq_vector_init();
    timer_init();
    enable_interrupt_controller();
    enable_irq();

    int sd_ok = sd_init();
    if (sd_ok == SD_OK) {
        uart_puts("SD card initialized successfully\n");
    } else {
        uart_puts("SD card initialization failed\n");
    }

    // Ora gli interrupt gestiscono RX/TX della UART e il timer.
    // Non serve più loop su uart_getc().
    while (1) {
    }
}
