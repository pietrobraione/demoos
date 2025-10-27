#include <stddef.h>
#include <stdint.h>
#include "../drivers/uart.h"
#include "../drivers/framebuffer.h"

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
  uart_init();
  uart_puts("Hello, kernel world!\r\n");
  
  while (1)
    uart_putc(uart_getc());
}
