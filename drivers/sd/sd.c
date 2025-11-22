#include "sd.h"
#include "../../libs/utils.h"
#include "../uart/uart.h"

#include <stdio.h>

void enable_card_detect();
void enable_clock_and_command();
void enable_data_pins();
void reset_emmc();

int sd_init(void) {
    enable_card_detect();
    enable_clock_and_command();
    enable_data_pins();

    uart_puts("[DEBUG] SD pins set up OK\n");

    long sd_standard_version = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    reset_emmc();
    uart_puts("[DEBUG] EMMC reset OK");

    return SD_OK;
}

void enable_card_detect() {
    // First, I set the GPIO 47 (card detect) pin to input (000). The GPIO47 is inside
    // group 4 (40-49) in position 7 and each position is 3 bit: GPFSEL4 + 7 * 3
    long r = *GPFSEL4;
    r &= ~(7 << (7 * 3));
    *GPFSEL4 = r;

    // Then I pullup the value of the pin to high to avoid unstable state
    // The GPPUD is the register which stores which type of pull to perform (up or down)
    // I have to write this register to perform a pullup which value is two
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

void enable_clock_and_command() {
    // The GPIO48 is the PIN which controls the clock of the SD controller, 
    // while the GPIO49 controls the command line to send commands to the SD
    // These PINs are in the 4th group with an offset of 8 and 9
    
    // As before, I have to set the function registers. In this case the function
    // bits are 111
    long r = *GPFSEL4;
    r |= (7 << (8 * 3)) | (7 << 9 * 3);
    *GPFSEL4 = r;

    // Then I pullup these PINS like in the function above
    *GPPUD = 2;
    delay(150);

    *GPPUDCLK1 = (1 << 16) | (1 << 17);
    delay(150);

    *GPPUD = 0;
    *GPPUDCLK1 = 0;
}

void enable_data_pins() {
    // DAT0 is the GPIO50, so it is part of the group 5. I have to apply the function
    // bits, which in this case will be 111
    long r = *GPFSEL5;
    r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
    *GPFSEL5 = r;

    *GPPUD = 2;
    delay(150);

    *GPPUDCLK1 |= (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    delay(150);

    *GPPUD = 0;
    *GPPUDCLK1 = 0;
}

void reset_emmc() {
    // I have to reset all the bits of the controller
    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 |= C1_SRST_HC;

    // The raspberry hardware should put the EMMC_CONTROL1 to 0 after a while;
    // so I wait this to happen
    // IMPORTANT: qemu is not implementing the EMMC controller, so the CONTROL1 
    // bit will never be written
    // To confirm this thesis, execute the following code:
    /*
    *EMMC_CONTROL1 = 0x20;
    uart_hex(*EMMC_CONTROL1); // Prints 0x20
    uart_puts("\n");

    *EMMC_CONTROL1 = 0x01000000;
    uart_hex(*EMMC_CONTROL1); // Prints 0
    uart_puts("\n");
    */
    // Maybe the bit reset is really fast but this is really sus

    while (*EMMC_CONTROL1 & C1_SRST_HC) {
        uart_puts("[DEBUG] I'm waiting the controller to reset the CONTROL1\n");
    }
}