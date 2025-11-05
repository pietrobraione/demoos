#include "utils.h"
#include "timer.h"

const unsigned int interval = 200000;
unsigned int curVal = 0;

void timer_init ( void )
{
	curVal = get32(TIMER_CLO);
	curVal += interval;
	put32(TIMER_C1, curVal);

}

void handle_timer_irq(void)
{
    put32(TIMER_CS, TIMER_CS_M1);
    curVal += interval;
    put32(TIMER_C1, curVal);
    uart_puts("Timer interrupt received\r\n");
}
