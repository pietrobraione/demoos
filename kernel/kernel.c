#include <stddef.h>
#include <stdint.h>
#include "../libs/utils.h"
#include "../drivers/uart/uart.h"
#include "../drivers/timer/timer.h"
#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"

void process(char* array);

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
    uart_puts("[DEBUG] IRQ vector INIT successful\n");
    timer_init();
    uart_puts("[DEBUG] Timer INIT successful\n");
    enable_interrupt_controller();
    uart_puts("[DEBUG] Interrupt controller INIT successful\n");
    enable_irq();
    uart_puts("[DEBUG] IRQ ENABLE successful\n");

    // EMMC non funziona su qemu
    int sd_ok = sd_init();
    if (sd_ok == SD_OK) {
        uart_puts("SD card initialized OK\n");
    } else {
        uart_puts("SD card initialization FAILED\n");
    }

    // Ora gli interrupt gestiscono RX/TX della UART e il timer.
    // Non serve più loop su uart_getc().
    while (1) {
    }
}

void process(char* array) {
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_putc(array[i]);
            delay(100000);
        }
    }
}
