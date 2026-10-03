#include "timer.h"
#include "../../arch/mmio.h"
#include "../../libs/scheduler.h"

// Il periodo del tick e' la costante TIMER_PERIOD definita in timer.h
unsigned int curVal = 0;

void timer_init(void) {
  curVal = mmio_read(TIMER_CLO);
  curVal += TIMER_PERIOD;
  mmio_write(TIMER_C1, curVal);
}

void handle_timer_irq(void) {
  mmio_write(TIMER_CS, TIMER_CS_M1);
  curVal += TIMER_PERIOD;
  mmio_write(TIMER_C1, curVal);
  handle_timer_tick();
}
