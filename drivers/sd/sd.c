#include "sd.h"
#include "../../libs/utils.h"

void enable_card_detect();

int sd_init(void) {
    enable_card_detect();

    return SD_OK;
}

void enable_card_detect() {
    // The GPIO15 pin is used for card detect; I configure it as an input
    // to detect card presence

    // First, I set the GPIO 47 (card detect) pin to input. The GPIO47 is inside
    // group 4 (40-49) in position 7 and each position is 3 bit: GPFSEL4 + 7 * 3
    long r = *GPFSEL4;
    r &= ~(7 << 7 * 3);
    *GPFSEL4 = r;

    // Then I pullup the value of the pin to high to avoid unstable state
    // The GPPUD is the register which stores which type of pull to perform (up or down)
    // I have to write this register to perform a pullup
    *GPPUD = 2;
    delay(150);
    // Then I need to write which PIN should have the pullup; GPPUDCLK1 controls GPIO fr
    // from 32 to 53, so the offset for the GPIO 47 is 15
    *GPPUDCLK1 = (1 << 15);
    delay(150);
    // Then the pullup is applied and I reset the registers
    *GPPUD = 0;
    *GPPUDCLK1 = 0;

    // The register GPHEN1 saves which PINS (from 21 to 53) will generate an interrupt
    // if their value is maintained to 1; I have to write the GPIO 47
    r = *GPHEN1;
    r |= 1 << 15;
    *GPHEN1 = r;
}
