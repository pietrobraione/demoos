#include "../../libs/mmio.h"
#include "../../libs/utils.h"
#include "timer.h"
#include "../uart/uart.h"

const unsigned int interval = 200000;
unsigned int curVal = 0;

void timer_init(void)
{
    curVal = mmio_read(TIMER_CLO);
    curVal += interval;
    mmio_write(TIMER_C1, curVal);
}

void handle_timer_irq(void)
{
    mmio_write(TIMER_CS, TIMER_CS_M1);
    curVal += interval;
    mmio_write(TIMER_C1, curVal);
    // uart_puts("Timer interrupt received\r\n");
}
